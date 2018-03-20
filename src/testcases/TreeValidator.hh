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

#include <folly/futures/Future.h>
#include "utils/TestcaseStatus.hh"
#include "utils/AssistedThread.hh"
#include "Manifest.hh"

namespace eostest {

struct ManifestHolder : public TestcaseStatus {
  Manifest manifest;
};

struct TreeLevel {
  TreeLevel(ManifestHolder &&holder) : manifest(std::move(holder)) {}

  ManifestHolder manifest;
  std::deque<folly::Future<ManifestHolder>> unexpandedChildren;
};


class TreeValidator {
public:
  TreeValidator(const std::string &url);
  folly::Future<TestcaseStatus> initialize();
  void main(ThreadAssistant &assistant);


private:
  std::string url;
  folly::Promise<TestcaseStatus> promise;
  AssistedThread thread;

  TreeLevel insertLevel(ManifestHolder manifest);
};

}
