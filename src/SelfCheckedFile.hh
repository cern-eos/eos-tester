// ----------------------------------------------------------------------
// File: SelfCheckedFile.hh
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

#ifndef EOSTESTER_SELF_CHECKED_FILE_H
#define EOSTESTER_SELF_CHECKED_FILE_H

#include <string>

namespace eostest {

class SelfCheckedFile {
public:
  SelfCheckedFile();
  SelfCheckedFile(const std::string &filename, const std::string &randomBytes);

  std::string toStringWithoutChecksum() const;
  std::string toString() const;
  std::string checksum() const;

  bool parse(const std::string &contents);
  std::string getFilename() const;
  std::string getRandomBytes() const;
  bool operator==(const SelfCheckedFile &rhs) const;
  void clear();

private:
  std::string filename;
  std::string randomBytes;
};

}

#endif
