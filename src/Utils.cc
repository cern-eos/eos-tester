// ----------------------------------------------------------------------
// File: Utils.cc
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

#include <climits>
#include <iostream>
#include "Utils.hh"
using namespace eostest;

bool eostest::startswith(const std::string &str, size_t start, const std::string &prefix) {
  if(prefix.size() > str.size() + start) return false;

  for(size_t i = 0; i < prefix.size(); i++) {
    if(str[i+start] != prefix[i]) return false;
  }
  return true;
}

bool eostest::my_strtoll(const std::string &str, int64_t &ret) {
  char *endptr = NULL;
  ret = strtoll(str.c_str(), &endptr, 10);
  if(endptr != str.c_str() + str.size() || ret == LLONG_MIN || ret == LONG_LONG_MAX) {
    return false;
  }
  return true;
}

bool eostest::extractLineWithPrefix(const std::string &str, size_t start, const std::string &prefix, std::string &val) {
  size_t index = start;
  if(!startswith(str, index, prefix)) return false;

  size_t startIndex = start + prefix.size();
  size_t endIndex = startIndex;

  while(endIndex < str.size() && str[endIndex] != '\n') {
    endIndex++;
  }

  if(str[endIndex] != '\n') return false;
  val = std::string(str.c_str() + startIndex, endIndex - startIndex);

  if(val.empty()) return false;

  return true;
}

bool eostest::isEqualAndProgressIndex(const std::string &str, size_t &index, const std::string &compare) {
  if(!startswith(str, index, compare)) return false;
  index += compare.size();
  return true;
}
