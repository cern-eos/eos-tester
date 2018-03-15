// ----------------------------------------------------------------------
// File: XrdClExecutor.hh
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

#ifndef EOSTESTER_XRDCL_EXECUTOR_H
#define EOSTESTER_XRDCL_EXECUTOR_H

#include <future>
#include <vector>
#include <string>
#include "utils/ErrorAccumulator.hh"
#include <folly/futures/Future.h>
#include <XrdCl/XrdClFileSystem.hh>

namespace eostest {

class OpenStatus;
class ReadOutcome;

class OperationStatus : public ErrorAccumulator {
public:
  OperationStatus(); // success
  OperationStatus(const std::string &err); // error
  OperationStatus(const OpenStatus &openStatus); // copy any errors from openStatus
};

class ReadStatus {
public:
  ReadStatus() {}
  ReadStatus(const ReadOutcome &outcome);

  OperationStatus status;
  std::string contents;

  bool ok() const;
};

class DirListStatus : public ErrorAccumulator {
public:
  using ErrorAccumulator::ErrorAccumulator;
  std::unique_ptr<XrdCl::DirectoryList> contents;
};

class XrdClExecutor {
public:
  static folly::Future<OperationStatus> mkdir(size_t connectionId, const std::string &url);
  static folly::Future<OperationStatus> put(size_t connectionId, const std::string &url, const std::string &contents);
  static folly::Future<OperationStatus> rm(size_t connectionId, const std::string &url);
  static folly::Future<ReadStatus> get(size_t connectionId, const std::string &path);
  static folly::Future<DirListStatus> dirList(size_t connectionId, const std::string &path);
};

}

#endif
