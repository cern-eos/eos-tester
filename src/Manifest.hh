// ----------------------------------------------------------------------
// File: Manifest.hh
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

#ifndef EOSTESTER_MANIFEST_H
#define EOSTESTER_MANIFEST_H

#include <set>
#include <string>

namespace eostest {

class Manifest {
public:
  Manifest(const std::string &filename);

  std::string toString() const;
  std::string toStringWithoutChecksum() const;
  std::string checksum() const;
  bool fromString(const std::string &filename, std::string &error);

  void addFile(const std::string &file);
  void addSubdir(const std::string &subdir);

  bool popFile(std::string &file);
  bool popSubdir(std::string &subdir);

  std::string getFilename() const;
private:
  std::string filename;
  std::set<std::string> directories;
  std::set<std::string> files;
};

}

#endif
