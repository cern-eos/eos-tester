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

namespace XrdCl {
  class DirectoryList;
}

namespace eostest {

class Manifest {
public:
  Manifest();
  Manifest(const std::string &filename);

  std::string toString() const;
  std::string toStringWithoutChecksum() const;
  std::string checksum() const;
  bool fromString(const std::string &filename, std::string &error);

  bool exists(const std::string &name);
  bool tryAddFile(const std::string &file);
  bool tryAddSubdir(const std::string &subdir);

  size_t fileCount() const;
  size_t subdirCount() const;

  bool popFile(std::string &file);
  bool popSubdir(std::string &subdir);
  std::string getFilename() const;

  void clear();
  bool parse(const std::string &contents);

  std::set<std::string>& getDirectories();
  std::set<std::string>& getFiles();

  bool crossCheckDirlist(XrdCl::DirectoryList &dirlist);

private:
  bool parseList(const std::string &contents, size_t& index, bool dir);

  std::string filename;
  std::set<std::string> directories;
  std::set<std::string> files;
};

}

#endif
