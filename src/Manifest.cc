// ----------------------------------------------------------------------
// File: Manifest.cc
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

#include <sstream>
#include <iostream>

#include <XrdCl/XrdClXRootDResponses.hh>

#include "Manifest.hh"
#include "HashCalculator.hh"
#include "Utils.hh"
#include "Macros.hh"
using namespace eostest;

namespace {
  const std::string kManifest = "MANIFEST: ";
  const std::string kSeparator = "----------\n";
  const std::string kSubdir = "SUBDIR: ";
  const std::string kFile = "FILE: ";
}

Manifest::Manifest() {}

Manifest::Manifest(const std::string &file) : filename(file) {}

bool Manifest::fromString(const std::string &filename, std::string &error) {
  return false;
}

std::string Manifest::checksum() const {
  return HashCalculator::sha256(this->toStringWithoutChecksum());
}

std::string Manifest::toString() const {
  std::ostringstream ss;
  ss << toStringWithoutChecksum();
  ss << HashCalculator::base16Encode(checksum()) << std::endl;
  return ss.str();
}

std::string Manifest::toStringWithoutChecksum() const {
  std::ostringstream ss;

  ss << kManifest << filename << std::endl;
  ss << kSeparator;

  for(const std::string& subdir : directories) {
    ss << kSubdir << subdir << std::endl;
  }

  ss << kSeparator;

  for(const std::string& file : files) {
    ss << kFile << file << std::endl;
  }

  ss << kSeparator;
  return ss.str();
}

bool Manifest::exists(const std::string &name) {
  if(files.count(name) != 0) return true;
  if(directories.count(name) != 0) return true;
  return false;
}

size_t Manifest::fileCount() const {
  return files.size();
}

size_t Manifest::subdirCount() const {
  return directories.size();
}

bool Manifest::tryAddFile(const std::string &file) {
  if(exists(file)) return false;
  files.insert(file);
  return true;
}

bool Manifest::tryAddSubdir(const std::string &subdir) {
  if(exists(subdir)) return false;
  directories.insert(subdir);
  return true;
}

bool Manifest::popFile(std::string &file) {
  if(files.empty()) return false;
  auto it = files.begin();
  file = *it;
  files.erase(it);
  return true;
}

bool Manifest::popSubdir(std::string &subdir) {
  if(directories.empty()) return false;
  auto it = directories.begin();
  subdir = *it;
  directories.erase(it);
  return true;
}

std::string Manifest::getFilename() const {
  return filename;
}

void Manifest::clear() {
  filename.clear();
  files.clear();
  directories.clear();
}

bool Manifest::parse(const std::string &contents) {
  clear();

  size_t index = 0;
  if(!extractLineWithPrefix(contents, index, kManifest, filename)) return false;
  index += kManifest.size() + 1 + filename.size();

  if(!isEqualAndProgressIndex(contents, index, kSeparator)) return false;
  if(!parseList(contents, index, true)) return false;
  if(!parseList(contents, index, false)) return false;

  std::string givenChecksum;
  if(!extractLineWithPrefix(contents.c_str(), index, "", givenChecksum)) return false;
  if(givenChecksum.size() != 64) return false;

  if(HashCalculator::base16Encode(checksum()) != givenChecksum) return false;
  if(index + givenChecksum.size() + 1 != contents.size()) return false;
  return true;
}

bool Manifest::parseList(const std::string &contents, size_t& index, bool dirs) {
  std::string prefix = kFile;
  if(dirs) prefix = kSubdir;

  while(true) {
    if(contents.size() <= index) return false;
    if(isEqualAndProgressIndex(contents, index, kSeparator)) return true;

    std::string tmp;
    if(!extractLineWithPrefix(contents, index, prefix, tmp)) return false;
    index += prefix.size() + 1 + tmp.size();

    if(dirs) {
      directories.insert(tmp);
    }
    else {
      files.insert(tmp);
    }
  }
}

std::set<std::string>& Manifest::getDirectories() {
  return directories;
}

std::set<std::string>& Manifest::getFiles() {
  return files;
}

bool Manifest::crossCheckDirlist(XrdCl::DirectoryList& dirlist) {
  size_t directoryCount = 0;
  size_t fileCount = 0;

  for(size_t i = 0; i < dirlist.GetSize(); i++) {
    // Directory?
    if(dirlist.At(i)->GetStatInfo()->TestFlags(XrdCl::StatInfo::IsDir)) {
      directoryCount++;
      if(directories.count(dirlist.At(i)->GetName()) == 0) return false;
    }
    else {
      // File
      fileCount++;
      if(files.count(dirlist.At(i)->GetName()) == 0) return false;
    }
  }

  if(directories.size() != directoryCount) return false;
  if(files.size() != fileCount) return false;
  return true;
}
