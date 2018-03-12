// ----------------------------------------------------------------------
// File: HierarchyBuilder.hh
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

#ifndef EOSTESTER_HIERARCHY_BUILDER_H
#define EOSTESTER_HIERARCHY_BUILDER_H

#include <string>

namespace eostest {

struct HierarchyConstructionOptions {
  std::string base;
  int32_t seed;
  size_t depth;
  int64_t files; // number of files, including manifests
};

struct HierarchyFile {
  std::string fullPath;
  std::string contents;
  bool dir;
};

class HierarchyBuilder {
public:
  HierarchyBuilder(const HierarchyConstructionOptions &opts);
  bool next(HierarchyFile &result);

private:
  HierarchyConstructionOptions options;

};

}

#endif
