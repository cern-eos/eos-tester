// ----------------------------------------------------------------------
// File: TreeValidator.hh
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

#include "utils/TestcaseStatus.hh"
#include "utils/AssistedThread.hh"
#include "Manifest.hh"

#include <XrdCl/XrdClURL.hh>

#include <folly/futures/Future.h>
#include <folly/executors/Async.h>

#include <memory>

namespace eostest {

struct ManifestHolder : public TestcaseStatus {
  Manifest manifest;
};

struct TreeLevel {
  TreeLevel(ManifestHolder &&holder) : manifest(std::move(holder)) {}

  ManifestHolder manifest;
  std::deque<folly::Future<ManifestHolder>> unexpandedChildren;
};

class ProgressTracker;

class TreeValidator {
public:
  TreeValidator(const std::string &url, ProgressTracker *track);
  folly::Future<TestcaseStatus> initialize();
  void main(ThreadAssistant &assistant);

private:
  std::string url;
  folly::Promise<TestcaseStatus> promise;
  AssistedThread thread;
  ProgressTracker* tracker = nullptr;

  TreeLevel insertLevel(ManifestHolder manifest);
  folly::Future<ManifestHolder> validateSingleDirectory(size_t connectionId, const std::string &path);
  folly::Future<ManifestHolder> validateContainedFiles(size_t connectionId, ManifestHolder holder, std::string path);
  folly::Future<TestcaseStatus> validateSingleFile(size_t connectionId, const std::string &path);

  void worker(std::string url, TestcaseStatus &acc, ThreadAssistant &assistant);

  size_t getConnectionId() {
    return (currentConnectionId++) % 32;
  }

  std::atomic<size_t> currentConnectionId = 0;
};

}
