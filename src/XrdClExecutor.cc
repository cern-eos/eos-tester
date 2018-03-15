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

OperationStatus::OperationStatus() { }

OperationStatus::OperationStatus(const std::string &err) {
  errors.push_back(err);
}

bool OperationStatus::ok() const {
  return errors.empty();
}

std::string errorVectorToString(const std::vector<std::string> &errors) {
  std::ostringstream ss;
  for(size_t i = 0; i < errors.size(); i++) {
    ss << "Error #" << i+1 << ": " << errors[i] << std::endl;
  }

  return ss.str();
}

std::string OperationStatus::toString() const {
  return errorVectorToString(errors);
}

class HandlerHelper {
public:
  HandlerHelper() {}
  virtual ~HandlerHelper() {}

  template<typename PromiseType, typename... Args>
  void setValueAndDeleteThis(PromiseType &prom, Args&&... args) {
    prom.setValue(std::forward<Args>(args)...);
    delete this;
  }

  template<typename PromiseType, typename... Args>
  void finalize(PromiseType &prom, XrdCl::XRootDStatus *status, XrdCl::AnyObject *response, Args&&... args) {
    prom.setValue(std::forward<Args>(args)...);

    if(status) delete status;
    if(response) delete response;
    delete this;
  }

  void trivialResponseHandler(folly::Promise<OperationStatus> &promise, XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) {
    if(!status->IsOK()) {
      return finalize(promise, status, response, OperationStatus(status->ToString()));
    }

    return finalize(promise, status, response, OperationStatus());
  }

};

class MkdirHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  MkdirHandler(const XrdCl::URL &ur) : url(ur), fs(ur.GetURL()) { }
  virtual ~MkdirHandler() {}

  folly::Future<OperationStatus> initialize() {
    folly::Future<OperationStatus> fut = promise.getFuture();

    XrdCl::XRootDStatus status = fs.MkDir(url.GetPath(), XrdCl::MkDirFlags::None, XrdCl::Access::OR, this);
    if(!status.IsOK()) {
      setValueAndDeleteThis(promise, OperationStatus(status.ToString()));
      return fut;
    }

    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    trivialResponseHandler(promise, status, response);
  }

private:
  XrdCl::URL url;
  XrdCl::FileSystem fs;
  folly::Promise<OperationStatus> promise;
};

namespace eostest {

class OpenStatus {
public:
  OpenStatus(const std::string &err) {
    errors.emplace_back(err);
  }

  OpenStatus(std::unique_ptr<XrdCl::File> f)
  : file(std::move(f)) { }

  OpenStatus() {}

  bool ok() const {
    return errors.empty() && file;
  }

  std::string toString() const {
    return errorVectorToString(errors);
  }

  void addError(const std::string &err) {
    file.reset();
    errors.emplace_back(err);
  }

  std::vector<std::string> errors;
  std::unique_ptr<XrdCl::File> file;
};

}

OperationStatus::OperationStatus(const OpenStatus &openStatus) {
  errors = openStatus.errors;
}


class OpenHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  OpenHandler(const XrdCl::URL &ur, const XrdCl::OpenFlags::Flags &flag,
    const XrdCl::Access::Mode &mod)
  : url(ur), flags(flag), mode(mod) {}

  virtual ~OpenHandler() {}

  folly::Future<OpenStatus> initialize() {
    folly::Future<OpenStatus> fut = promise.getFuture();

    file.reset(new XrdCl::File());
    XrdCl::XRootDStatus status = file->Open(url.GetURL(), flags, mode, this);
    if(!status.IsOK()) {
      setValueAndDeleteThis(promise, OpenStatus(status.ToString()));
    }

    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    if(!status->IsOK()) {
      return finalize(promise, status, response, OpenStatus(status->ToString()));
    }

    return finalize(promise, status, response, OpenStatus(std::move(file)));
  }

private:
  XrdCl::URL url;
  XrdCl::OpenFlags::Flags flags;
  XrdCl::Access::Mode mode;

  std::unique_ptr<XrdCl::File> file;
  folly::Promise<OpenStatus> promise;
};


class WriteHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  WriteHandler(const std::string &cont) : contents(cont) {}

  folly::Future<OpenStatus> initialize(OpenStatus openStatus) {
    folly::Future<OpenStatus> fut = promise.getFuture();

    if(!openStatus.ok()) {
      setValueAndDeleteThis(promise, std::move(openStatus));
      return fut;
    }

    XrdCl::XRootDStatus status = openStatus.file->Write(0, contents.size(), contents.c_str(), this);
    if(!status.IsOK()) {
      openStatus.addError(status.ToString());
      setValueAndDeleteThis(promise, std::move(openStatus));
      return fut;
    }

    file = std::move(openStatus);
    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    if(!status->IsOK()) {
      file.addError(status->ToString());
    }

    return finalize(promise, status, response, std::move(file));
  }

private:
  std::string contents;
  OpenStatus file;
  folly::Promise<OpenStatus> promise;
};

class CloseHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  CloseHandler() {}

  folly::Future<OperationStatus> initialize(OpenStatus openStatus) {
    folly::Future<OperationStatus> fut = promise.getFuture();

    if(!openStatus.ok()) {
      setValueAndDeleteThis(promise, OperationStatus(openStatus));
      return fut;
    }

    XrdCl::XRootDStatus status = openStatus.file->Close(this);
    if(!status.IsOK()) {
      setValueAndDeleteThis(promise, OperationStatus(status.ToString()));
    }

    file = std::move(openStatus);
    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    OperationStatus retval;

    if(!status->IsOK()) {
      retval.errors.push_back(status->ToString());
    }

    return finalize(promise, status, response, retval);
  }

private:
  OpenStatus file;
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

folly::Future<OperationStatus> XrdClExecutor::put(size_t connectionId, const std::string &path, const std::string &contents) {
  OpenHandler *openHandler = new OpenHandler(
    makeURL(connectionId, path),
    XrdCl::OpenFlags::Update | XrdCl::OpenFlags::New,
    XrdCl::Access::None
  );

  WriteHandler *writeHandler = new WriteHandler(contents);
  CloseHandler *closeHandler = new CloseHandler();

  return openHandler->initialize()
    .then(&WriteHandler::initialize, writeHandler)
    .then(&CloseHandler::initialize, closeHandler);
}

class RmHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  RmHandler(const XrdCl::URL &ur) : url(ur), fs(ur.GetURL()) { }
  virtual ~RmHandler() {}

  folly::Future<OperationStatus> initialize() {
    folly::Future<OperationStatus> fut = promise.getFuture();

    XrdCl::XRootDStatus status = fs.Rm(url.GetPath(), this);
    if(!status.IsOK()) {
      setValueAndDeleteThis(promise, OperationStatus(status.ToString()));
      return fut;
    }

    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    trivialResponseHandler(promise, status, response);
  }

private:
  XrdCl::URL url;
  XrdCl::FileSystem fs;
  folly::Promise<OperationStatus> promise;
};

folly::Future<OperationStatus> XrdClExecutor::rm(size_t connectionId, const std::string &path) {
  RmHandler *rmHandler = new RmHandler(makeURL(connectionId, path));
  return rmHandler->initialize();
}
