// ----------------------------------------------------------------------
// File: TreeBuilder.cc
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

#include <queue>
#include <iostream>
#include <rang.hpp>
#include <XrdCl/XrdClFile.hh>
#include "Macros.hh"
#include "TreeBuilder.hh"
#include "../XrdClExecutor.hh"
#include "../HierarchyBuilder.hh"
#include "utils/ProgressTracker.hh"
#include "utils/Sealing.hh"
using namespace eostest;

TreeBuilder::TreeBuilder(const Options &opts, ProgressTracker *track) {
  options = opts;
  tracker = track;
}

folly::Future<TestcaseStatus> TreeBuilder::initialize() {
  std::cout << std::endl;
  std::string description = SSTR(rang::style::bold << rang::fg::magenta << "Construct tree" << rang::style::reset << " :: Depth " << options.depth << " with " << options.files << " files");

  if(tracker) tracker->setDescription(description);

  folly::Future<TestcaseStatus> fut = promise.getFuture();
  thread.reset(&TreeBuilder::main, this);
  return Sealing::seal(std::move(fut), description);
}

void TreeBuilder::main(ThreadAssistant &assistant) {
  TestcaseStatus accumulator;

  const size_t pipelineLength = 5000;
  std::queue<folly::Future<TestcaseStatus>> queue;

  XrdCl::URL url(options.baseUrl);

  HierarchyConstructionOptions opts;
  opts.base = url.GetPath();
  opts.seed = options.seed;
  opts.depth = options.depth;
  opts.files = options.files;

  HierarchyBuilder hierarchyBuilder(opts);

  while(true) {
    if(assistant.terminationRequested()) {
      accumulator.addError("Early termination requested");
      break;
    }

    // Pop any ready futures at the head of the queue
    while(!queue.empty() && (queue.front().isReady() || queue.size() >= pipelineLength)) {
      accumulator.absorbErrors(std::move(queue.front()).get());
      queue.pop();
    }

    // Push more futures
    while(queue.size() <= pipelineLength) {
      HierarchyEntry entry;
      if(!hierarchyBuilder.next(entry)) {
        while(!queue.empty()) {
          accumulator.absorbErrors(std::move(queue.front()).get());
          queue.pop();
        }
        goto out;
      }

      url.SetPath(entry.fullPath);

      if(entry.dir) {
        queue.push(XrdClExecutor::mkdir(1, url.GetURL()));
      }
      else {
        folly::Future<TestcaseStatus> fut = XrdClExecutor::put(1, url.GetURL(), entry.contents);
        if(tracker) fut = tracker->filterFuture(std::move(fut));
        queue.push(std::move(fut));
      }
    }
  }

out:
  promise.setValue(std::move(accumulator));
}
