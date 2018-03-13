// ----------------------------------------------------------------------
// File: HierarchyBuilder.cc
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
#include "HierarchyBuilder.hh"
#include "Macros.hh"
#include "Utils.hh"
#include "SelfCheckedFile.hh"
using namespace eostest;

HierarchyBuilder::HierarchyBuilder(const HierarchyConstructionOptions &opt)
: options(opt), generator(options.seed) {

  insertNode(options.base);
}

void HierarchyBuilder::insertNode(const std::string &path) {
  stack.emplace(path);

  // Roll dice to decide how many subdirs and files to insert
  std::uniform_int_distribution<> distr(0, 10);
  size_t files = distr(generator) + 1;
  size_t subdirs = distr(generator);

  files = std::min(options.files, files);
  options.files -= files;

  for(size_t i = 0; i < files - 1; i++) {
    stack.top().manifest.addFile(getRandomAlphanumericBytes(5, generator));
  }

  for(size_t i = 0; i < subdirs; i++) {
    stack.top().manifest.addSubdir(getRandomAlphanumericBytes(5, generator));
  }
}

std::string HierarchyBuilder::getRandomFileContents() {
  // Roll dice to decide file length
  std::uniform_int_distribution<> distr(1, 256);
  return getRandomPrintableBytes(distr(generator), generator);
}

bool HierarchyBuilder::next(HierarchyEntry &result) {
  while(!stack.empty()) {
    if(!stack.top().manifestDone) {
      stack.top().manifestDone = true;
      result.fullPath = stack.top().manifest.getFilename();
      result.contents = stack.top().manifest.toString();
      result.dir = false;
      return true;
    }

    std::string nextFile;
    if(stack.top().manifest.popFile(nextFile)) {
      result.fullPath = SSTR(stack.top().path <<  "/" << nextFile);
      result.contents = SelfCheckedFile(result.fullPath, getRandomFileContents() ).toString();
      result.dir = false;
      return true;
    }

    // No more files, should we add next directory?
    if(stack.size() <= options.depth && options.files >= 1) {
      std::string nextDir;
      if(stack.top().manifest.popSubdir(nextDir)) {
        result.fullPath = SSTR(stack.top().path << "/" << nextDir);
        result.contents = "";
        result.dir = true;

        insertNode(result.fullPath);
        return true;
      }
    }

    // Nope, pop
    stack.pop();
  }

  return false;
}
