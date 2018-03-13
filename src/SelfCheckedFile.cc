// ----------------------------------------------------------------------
// File: SelfCheckedFile.cc
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
#include <sstream>
#include "SelfCheckedFile.hh"
#include "HashCalculator.hh"
#include "Utils.hh"
#include "Macros.hh"
using namespace eostest;

namespace {
  const std::string kFilenamePrefix = "FILENAME: ";
  const std::string kRandomBytesPrefix = "RANDOM-BYTES: ";
  const std::string kSeparator = "----------";
}

SelfCheckedFile::SelfCheckedFile() { }

SelfCheckedFile::SelfCheckedFile(const std::string &fname, const std::string &rnd)
: filename(fname), randomBytes(rnd) { }

std::string SelfCheckedFile::checksum() const {
  return HashCalculator::sha256(this->toStringWithoutChecksum());
}

std::string SelfCheckedFile::toStringWithoutChecksum() const {
  std::stringstream ss;
  ss << kFilenamePrefix << filename << std::endl;
  ss << kRandomBytesPrefix << randomBytes.size() << std::endl;
  ss << kSeparator << std::endl;
  ss << randomBytes << std::endl;
  ss << kSeparator << std::endl;
  return ss.str();
}

std::string SelfCheckedFile::toString() const {
  std::stringstream ss;
  ss << toStringWithoutChecksum();
  ss << HashCalculator::base16Encode(checksum()) << std::endl;
  return ss.str();
}

bool SelfCheckedFile::parse(const std::string &contents) {
  if(!startswith(contents, 0, kFilenamePrefix)) return false;

  std::ostringstream ss;

  size_t index = kFilenamePrefix.size();
  while(index < contents.size() && contents[index] != '\n') {
    ss << contents[index];
    index++;
  }

  filename = ss.str();
  if(filename.empty()) return false;
  index++;

  if(!startswith(contents, index, kRandomBytesPrefix)) return false;
  index += kRandomBytesPrefix.size();

  size_t parseIndexStart = index;
  size_t parseIndexEnd = index;

  while(parseIndexEnd < contents.size() && contents[parseIndexEnd] != '\n') {
    parseIndexEnd++;
  }

  if(parseIndexEnd == parseIndexStart) return false;

  std::string value = std::string(contents.c_str() + parseIndexStart, parseIndexEnd - parseIndexStart);
  int64_t randomBytesLength = -1;

  if(!my_strtoll(value, randomBytesLength)) return false;
  index = parseIndexEnd;

  if(!startswith(contents, index, SSTR("\n" << kSeparator << "\n"))) return false;
  index += kSeparator.size() + 2;

  if(contents.size() <= index + randomBytesLength) return false;
  randomBytes = std::string(contents.c_str(), index, randomBytesLength);
  index += randomBytesLength;

  if(!startswith(contents, index, SSTR("\n" << kSeparator << "\n"))) return false;
  index += kSeparator.size() + 2;

  parseIndexStart = index;
  parseIndexEnd = index;
  while(parseIndexEnd < contents.size() && contents[parseIndexEnd] != '\n') {
    parseIndexEnd++;
  }

  if(parseIndexEnd+1 != contents.size()) return false;
  if(contents[parseIndexEnd] != '\n') return false;

  std::string givenChecksum = std::string(contents.c_str() + parseIndexStart, parseIndexEnd - parseIndexStart);
  if(givenChecksum.size() != 64) return false;

  if(HashCalculator::base16Encode(checksum()) != givenChecksum) return false;

  return true;
}

std::string SelfCheckedFile::getFilename() const {
  return filename;
}

std::string SelfCheckedFile::getRandomBytes() const {
  return randomBytes;
}
