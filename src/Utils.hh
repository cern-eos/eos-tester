// ----------------------------------------------------------------------
// File: Utils.hh
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

#ifndef EOSTESTER_UTILS_H
#define EOSTESTER_UTILS_H

#include <vector>
#include <string>
#include <random>
#include <sstream>

namespace eostest {

bool startswith(const std::string &str, size_t start, const std::string &prefix);
bool my_strtoll(const std::string &str, int64_t &ret);

inline std::string chopPath(const std::string &path) {
  std::string retval = path;
  for(size_t i = retval.size() - 1; i != 0; i--) {
    if(retval[i] == '/') {
      retval.erase(i, std::string::npos);
      break;
    }
  }

  return retval;
}

inline std::string getRandomPrintableBytes(size_t length, std::mt19937 &generator) {
  std::ostringstream ss;
  std::uniform_int_distribution<> distr(32, 254);

  for(size_t i = 0; i < length; i++) {
    ss << char(distr(generator));
  }

  return ss.str();
}

inline std::string getRandomAlphanumericBytes(size_t length, std::mt19937 &generator) {
  std::ostringstream ss;
  std::uniform_int_distribution<> distr(97, 122);

  for(size_t i = 0; i < length; i++) {
    ss << char(distr(generator));
  }

  return ss.str();
}




}

#endif
