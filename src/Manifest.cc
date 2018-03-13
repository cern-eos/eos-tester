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

#include "Manifest.hh"
#include "HashCalculator.hh"
#include <sstream>
using namespace eostest;

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

  ss << "MANIFEST: " << filename << std::endl;
  ss << "----------" << std::endl;

  for(const std::string& subdir : directories) {
    ss << "SUBDIR: " << subdir << std::endl;
  }

  ss << "----------" << std::endl;

  for(const std::string& file : files) {
    ss << "FILE: " << file << std::endl;
  }

  ss << "----------" << std::endl;
  return ss.str();
}

void Manifest::addFile(const std::string &file) {
  files.insert(file);
}

void Manifest::addSubdir(const std::string &subdir) {
  directories.insert(subdir);
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
