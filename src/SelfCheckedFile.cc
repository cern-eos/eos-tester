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
  const std::string kSeparator = "----------\n";
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
  ss << kSeparator;
  ss << randomBytes << std::endl;
  ss << kSeparator;
  return ss.str();
}

std::string SelfCheckedFile::toString() const {
  std::stringstream ss;
  ss << toStringWithoutChecksum();
  ss << HashCalculator::base16Encode(checksum()) << std::endl;
  return ss.str();
}

void SelfCheckedFile::clear() {
  filename.clear();
  randomBytes.clear();
}

bool SelfCheckedFile::parse(const std::string &contents) {
  clear();

  std::string value;
  if(!extractLineWithPrefix(contents.c_str(), 0, kFilenamePrefix, filename)) return false;
  size_t index = kFilenamePrefix.size() + 1 + filename.size();

  if(!extractLineWithPrefix(contents.c_str(), index, kRandomBytesPrefix, value)) return false;
  int64_t randomBytesLength = -1;
  if(!my_strtoll(value, randomBytesLength)) return false;
  index += kRandomBytesPrefix.size() + 1 + value.size();

  if(!isEqualAndProgressIndex(contents, index, kSeparator)) return false;

  if(contents.size() <= index + randomBytesLength) return false;
  randomBytes = std::string(contents.c_str(), index, randomBytesLength);
  index += randomBytesLength;

  if(!isEqualAndProgressIndex(contents, index, SSTR("\n" << kSeparator))) return false;

  std::string givenChecksum;
  if(!extractLineWithPrefix(contents.c_str(), index, "", givenChecksum)) return false;
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

bool SelfCheckedFile::operator==(const SelfCheckedFile &rhs) const {
  return filename == rhs.filename && randomBytes == rhs.randomBytes;
}

TestcaseStatus SelfCheckedFile::validate(std::string fileContents, std::string expectedFilename) {
  SelfCheckedFile scf;
  if(!scf.parse(fileContents)) {
    return TestcaseStatus(SSTR("Could not parse self-checked-file contents: " << expectedFilename));
  }

  if(scf.getFilename() != expectedFilename) {
    return TestcaseStatus(SSTR("Expected self-checked-file path " << expectedFilename << ", received " << scf.getFilename()));
  }

  return TestcaseStatus();
}
