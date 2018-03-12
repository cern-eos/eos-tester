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

#include <sstream>
#include "SelfCheckedFile.hh"
#include "HashCalculator.hh"
using namespace eostest;

SelfCheckedFile::SelfCheckedFile() { }

SelfCheckedFile::SelfCheckedFile(const std::string &fname, const std::string &rnd)
: filename(fname), randomBytes(rnd) { }

std::string SelfCheckedFile::checksum() const {
  return HashCalculator::sha256(this->toStringWithoutChecksum());
}

std::string SelfCheckedFile::toStringWithoutChecksum() const {
  std::stringstream ss;
  ss << "FILENAME: " << filename << std::endl;
  ss << "RANDOM-BYTES: " << randomBytes.size() << std::endl;
  ss << "----------" << std::endl;
  ss << randomBytes << std::endl;
  ss << "----------" << std::endl;
  return ss.str();
}

std::string SelfCheckedFile::toString() const {
  std::stringstream ss;
  ss << toStringWithoutChecksum();
  ss << HashCalculator::base16Encode(checksum()) << std::endl;
  return ss.str();
}
