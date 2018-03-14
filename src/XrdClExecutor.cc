// ----------------------------------------------------------------------
// File: XrdClExecutor.cc
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

#include <iostream>
#include <XrdCl/XrdClFileSystem.hh>
#include <XrdCl/XrdClFile.hh>

#include "XrdClExecutor.hh"
#include "Macros.hh"

using namespace eostest;

bool OperationStatus::ok() const {
  return errors.empty();
}

std::string OperationStatus::toString() const {
  std::ostringstream ss;
  for(size_t i = 0; i < errors.size(); i++) {
    ss << "Error #" << i+1 << ": " << errors[i] << std::endl;
  }

  return ss.str();
}

class MkdirHandler : public XrdCl::ResponseHandler {
public:
  MkdirHandler(const XrdCl::URL &ur) : url(ur), fs(ur.GetURL()) { }
  virtual ~MkdirHandler() {}

  folly::Future<OperationStatus> initialize() {
    folly::Future<OperationStatus> fut = promise.getFuture();

    XrdCl::XRootDStatus status = fs.MkDir(url.GetPath(), XrdCl::MkDirFlags::None, XrdCl::Access::OR, this);
    if(!status.IsOK()) {
      OperationStatus retval;
      retval.errors.push_back(status.ToString());

      folly::Future<OperationStatus> fut = promise.getFuture();
      promise.setValue(retval);
      return fut;
    }

    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    if(!status->IsOK()) {
      OperationStatus retval;
      retval.errors.push_back(status->ToString());

      delete status;
      if(response) delete response;

      promise.setValue(retval);
      delete this;
      return;
    }

    OperationStatus retval;
    promise.setValue(retval);
    delete status;
    if(response) delete response;
    delete this;
    return;
  }

private:
  XrdCl::URL url;
  XrdCl::FileSystem fs;
  folly::Promise<OperationStatus> promise;
};

std::string makeURL(size_t connectionId, const std::string &path) {
  XrdCl::URL url(path);
  url.SetUserName(SSTR("t" << connectionId));
  return url.GetURL();
}

folly::Future<OperationStatus> XrdClExecutor::mkdir(size_t connectionId, const std::string &path) {
  MkdirHandler *handler = new MkdirHandler(makeURL(connectionId, path));
  return handler->initialize();
}
