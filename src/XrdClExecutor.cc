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

class OpenStatus : public ErrorAccumulator {
public:
  using ErrorAccumulator::ErrorAccumulator;

  OpenStatus(std::unique_ptr<XrdCl::File> f)
  : file(std::move(f)) { }

  bool ok() const {
    return ErrorAccumulator::ok() && file;
  }
  
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

template<typename IncomingStatus, typename OutgoingStatus>
class CloseHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  CloseHandler() {}

  folly::Future<OutgoingStatus> initialize(IncomingStatus incoming) {
    folly::Future<OutgoingStatus> fut = promise.getFuture();

    if(!incoming.ok()) {
      setValueAndDeleteThis(promise, OutgoingStatus(incoming));
      return fut;
    }

    XrdCl::XRootDStatus status = incoming.file->Close(this);
    if(!status.IsOK()) {
      incoming.addError(status.ToString());
      setValueAndDeleteThis(promise, OutgoingStatus(incoming));
    }

    incomingStatus = std::move(incoming);
    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    if(!status->IsOK()) {
      incomingStatus.addError(status->ToString());
    }

    return finalize(promise, status, response, OutgoingStatus(incomingStatus));
  }

private:
  IncomingStatus incomingStatus;
  folly::Promise<OutgoingStatus> promise;
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
  CloseHandler<OpenStatus, OperationStatus> *closeHandler = new CloseHandler<OpenStatus, OperationStatus>();

  return openHandler->initialize()
    .then(&WriteHandler::initialize, writeHandler)
    .then(&CloseHandler<OpenStatus, OperationStatus>::initialize, closeHandler);
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

namespace eostest {

class ReadOutcome {
public:
  ReadOutcome() {}
  ReadOutcome(const OpenStatus &openStatus) {
    readStatus.absorbErrors(openStatus);
  }

  void addError(const std::string &err) {
    readStatus.addError(err);
  }

  bool ok() const {
    return readStatus.ok();
  }

  ReadStatus readStatus;
  std::unique_ptr<XrdCl::File> file;
};

}

ReadStatus::ReadStatus(const ReadOutcome &outcome) {
  *this = outcome.readStatus;
}

class ReadHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  ReadHandler(size_t size = 4096) {
    retval.readStatus.contents.resize(size);
  }

  folly::Future<ReadOutcome> initialize(OpenStatus openStatus) {
    folly::Future<ReadOutcome> fut = promise.getFuture();

    if(!openStatus.ok()) {
      setValueAndDeleteThis(promise, ReadOutcome(openStatus));
      return fut;
    }

    retval.file = std::move(openStatus.file);
    XrdCl::XRootDStatus status = retval.file->Read(
      0,
      retval.readStatus.contents.size(),
      (void*) retval.readStatus.contents.c_str(),
      this
    );

    if(!status.IsOK()) {
      retval.readStatus.addError(status.ToString());
      setValueAndDeleteThis(promise, std::move(retval));
      return fut;
    }

    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    if(!status->IsOK()) {
      retval.readStatus.addError(status->ToString());
    }
    else {
      XrdCl::ChunkInfo *chunk;
      response->Get(chunk);
      response->Set( (int*) 0);
      size_t bytesRead = chunk->length;
      delete chunk;

      retval.readStatus.contents.resize(bytesRead);
    }

    return finalize(promise, status, response, std::move(retval));
  }

private:
  ReadOutcome retval;
  folly::Promise<ReadOutcome> promise;
};

folly::Future<ReadStatus> XrdClExecutor::get(size_t connectionId, const std::string &path) {
  OpenHandler *openHandler = new OpenHandler(
    makeURL(connectionId, path),
    XrdCl::OpenFlags::Read,
    XrdCl::Access::None
  );

  ReadHandler *readHandler = new ReadHandler();

  CloseHandler<ReadOutcome, ReadStatus> *closeHandler = new CloseHandler<ReadOutcome, ReadStatus>();

  return openHandler->initialize()
    .then(&ReadHandler::initialize, readHandler)
    .then(&CloseHandler<ReadOutcome, ReadStatus>::initialize, closeHandler);
}

folly::Future<OperationStatus> XrdClExecutor::rm(size_t connectionId, const std::string &path) {
  RmHandler *rmHandler = new RmHandler(makeURL(connectionId, path));
  return rmHandler->initialize();
}

class DirListHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  DirListHandler(const XrdCl::URL &ur) : url(ur), fs(url.GetURL()) {}

  folly::Future<DirListStatus> initialize() {
    folly::Future<DirListStatus> fut = promise.getFuture();

    XrdCl::XRootDStatus status = fs.DirList(
      url.GetPath(),
      XrdCl::DirListFlags::Stat,
      this
    );

    if(!status.IsOK()) {
      setValueAndDeleteThis(promise, DirListStatus(status.ToString()));
    }

    return fut;
  }

  virtual void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) override {
    if(!status->IsOK()) {
      return finalize(promise, status, response, DirListStatus(status->ToString()));
    }

    // Extract DirectoryList out of response
    XrdCl::DirectoryList *dirlist;
    response->Get(dirlist);
    response->Set( (int*) 0);

    DirListStatus retval;
    retval.contents.reset(dirlist);

    return finalize(promise, status, response, std::move(retval));
  }

private:
  XrdCl::URL url;
  XrdCl::FileSystem fs;
  folly::Promise<DirListStatus> promise;
};

folly::Future<DirListStatus> XrdClExecutor::dirList(size_t connectionId, const std::string &path) {
  DirListHandler *handler = new DirListHandler(
    makeURL(connectionId, path)
  );

  return handler->initialize();
}

class RmdirHandler : public HandlerHelper, XrdCl::ResponseHandler {
public:
  RmdirHandler(const std::string &path)
  : url(path), fs(url.GetURL()) {

  }

  folly::Future<OperationStatus> initialize() {
    folly::Future<OperationStatus> fut = promise.getFuture();

    XrdCl::XRootDStatus status = fs.RmDir(
      url.GetPath(),
      this
    );

    if(!status.IsOK()) {
      setValueAndDeleteThis(promise, OperationStatus(status.ToString()));
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

folly::Future<OperationStatus> XrdClExecutor::rmdir(size_t connectionId, const std::string &url) {
  RmdirHandler *handler = new RmdirHandler(url);
  return handler->initialize();
}
