// ----------------------------------------------------------------------
// File: TreeValidator.cc
// Author: Georgios Bitzes - CERN
// ----------------------------------------------------------------------

/************************************************************************
 * eos-tester - a tool for stress testing EOS instances                 *
 * Copyright (C) 2018 CERN/Switzerland                                  *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 ************************************************************************/

#include "TreeValidator.hh"
#include "utils/ProgressTracker.hh"
#include "utils/Sealing.hh"
#include "../Manifest.hh"
#include "../XrdClExecutor.hh"
#include "../SelfCheckedFile.hh"
#include "Macros.hh"
#include "Utils.hh"

#include <rang.hpp>

#include <functional>
#include <iostream>

using namespace eostest;

TreeValidator::TreeValidator(const std::string &base, ProgressTracker *track) : url(base) {
  while(!url.empty() && url.back() == '/') {
    url.pop_back();
  }

  tracker = track;
}

folly::Future<TestcaseStatus> TreeValidator::initialize() {
  std::string description = SSTR(rang::style::bold << rang::fg::magenta << "Validate tree" << rang::style::reset << " :: " << url);

  if(tracker) tracker->setDescription(description);

  folly::Future<TestcaseStatus> fut = promise.getFuture();
  thread.reset(&TreeValidator::main, this);
  return Sealing::seal(std::move(fut), description);
}

ManifestHolder parseManifest(ReadStatus status, std::string path) {
  ManifestHolder holder;

  if(!status.ok()) {
    holder.addError(SSTR("Error fetching " << path << ": " << status.toString()));
    return holder;
  }

  if(!holder.manifest.parse(status.contents)) {
    holder.addError(SSTR("Could not parse contents for " << path << ": " << status.contents));
    return holder;
  }

  if(holder.manifest.getFilename() != path) {
    holder.addError(SSTR("Mismatch between expected manifest filename " << path << ", and included one: " << holder.manifest.getFilename()));
    return holder;
  }

  return holder;
}

TestcaseStatus parseFile(ReadStatus status, std::string path) {
  if(!status.ok()) {
    return status;
  }

  TestcaseStatus accu;
  accu.absorbErrors(
    SelfCheckedFile::validate(status.contents, XrdCl::URL(path).GetPath())
  );

  accu.seal(SSTR("Validate self-checked-file " << path));
  return accu;
}

folly::Future<ManifestHolder> fetchManifest(size_t connectionId, std::string path) {
  folly::Future<ReadStatus> readStatus = XrdClExecutor::get(connectionId, path);
  return std::move(readStatus).thenValue(std::bind(parseManifest, std::placeholders::_1, XrdCl::URL(path).GetPath()));
}

ManifestHolder validateManifest(std::tuple<ManifestHolder, DirListStatus> tup) {
  ManifestHolder manifestHolder = std::move(std::get<0>(tup));
  DirListStatus dirList = std::move(std::get<1>(tup));

  if(!manifestHolder.ok() || dirList.ok()) return manifestHolder;

  // Manifest is OK. Do manifest contents and dirList match?
  if(!manifestHolder.manifest.crossCheckDirlist(*dirList.contents)) {
    manifestHolder.addError(SSTR("Mismatch between dirlist and manifest contents for " << manifestHolder.manifest.getFilename()));
    return manifestHolder;
  }

  return manifestHolder;
}

folly::Future<TestcaseStatus> TreeValidator::validateSingleFile(size_t connectionId, const std::string &path) {
  folly::Future<ReadStatus> readStatus = XrdClExecutor::get(connectionId, path);
  return std::move(readStatus).thenValue(std::bind(parseFile, std::placeholders::_1, path));
}

ManifestHolder combineErrors(ManifestHolder &holder, std::vector<TestcaseStatus> errors) {
  for(size_t i = 0; i < errors.size(); i++) {
    holder.absorbChildIfError(std::move(errors[i]));
  }

  return holder;
}

folly::Future<ManifestHolder> TreeValidator::validateContainedFiles(size_t connectionId, ManifestHolder holder, std::string path) {
  std::vector<folly::Future<TestcaseStatus>> accus;

  std::string file;
  while(holder.manifest.popFile(file)) {
    folly::Future<TestcaseStatus> st = validateSingleFile(connectionId, SSTR(path << "/" << file));
    if(tracker) st = tracker->filterFuture(std::move(st));
    accus.emplace_back(std::move(st));
  }

  return folly::collect(accus)
    .thenValue(std::bind(combineErrors, std::move(holder), std::placeholders::_1));
}

folly::Future<ManifestHolder> TreeValidator::validateSingleDirectory(size_t connectionId, const std::string &path) {
  folly::Future<DirListStatus> dirList = XrdClExecutor::dirList(connectionId, path);
  folly::Future<ManifestHolder> holder = fetchManifest(connectionId, SSTR(path << "/MANIFEST"));

  return folly::collect(holder, dirList)
    .thenValue(validateManifest)
    .thenValue(std::bind(&TreeValidator::validateContainedFiles, this, connectionId, std::placeholders::_1, path));
}

eostest::TreeLevel TreeValidator::insertLevel(ManifestHolder manifest) {
  TreeLevel level(std::move(manifest));

  XrdCl::URL base(url);

  std::string subdir;
  while(level.manifest.manifest.popSubdir(subdir)) {
    base.SetPath(SSTR(chopPath(level.manifest.manifest.getFilename()) << "/" << subdir));
    level.unexpandedChildren.push_back(validateSingleDirectory(getConnectionId(), base.GetURL()));
  }

  return level;
}

void TreeValidator::main(ThreadAssistant &assistant) {
  TestcaseStatus acc;

  AssistedThread worker1(&TreeValidator::worker, this, url, std::ref(acc));
  assistant.propagateTerminationSignal(worker1);
  worker1.blockUntilThreadJoins();

  promise.setValue(std::move(acc));
}

void TreeValidator::worker(std::string url, TestcaseStatus &acc, ThreadAssistant &assistant) {
  std::deque<TreeLevel> stack;
  stack.emplace_back(insertLevel(validateSingleDirectory(getConnectionId(), url).get()));
  acc.absorbErrors(stack.back().manifest);
  XrdCl::URL base(url);

  while(true) {
    // Early termination requested?
    if(assistant.terminationRequested()) {
      break;
    }

    if(stack.empty()) {
      // We've exhausted our designated search space, return to caller.
      break;
    }
    auto& unexpandedChildren = stack.back().unexpandedChildren;

    if(!unexpandedChildren.empty()) {
      // Case 3: Expand a directory
      stack.push_back(insertLevel(std::move(unexpandedChildren.front()).get()));
      acc.absorbErrors(stack.back().manifest);
      unexpandedChildren.pop_front();
    }
    else {
      // Case 4: We're completely done with this level, pop.
      stack.pop_back();
    }
  }
}
