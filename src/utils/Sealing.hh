// ----------------------------------------------------------------------
// File: Sealing.hh
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

#ifndef EOSTESTER_SEALING_H
#define EOSTESTER_SEALING_H

#include <vector>
#include <string>
#include <folly/futures/Future.h>
#include "utils/TestcaseStatus.hh"

namespace eostest {

class TestcaseStatus;

struct PendingSeal {
  std::string description;
  std::chrono::steady_clock::time_point startTime;
};

class Sealing {
public:

  template<typename T>
  static folly::Future<T> seal(folly::Future<T> &&fut, const std::string &description) {
    PendingSeal pendingSeal;
    pendingSeal.description = description;
    pendingSeal.startTime = std::chrono::steady_clock::now();
    return std::move(fut).thenValue(std::bind(Sealing::callback<T>, pendingSeal, std::placeholders::_1));
  }

  template<typename T>
  static T callback(PendingSeal seal, T st) {
    st.seal(seal.description, std::chrono::steady_clock::now() - seal.startTime);
    return std::move(st);
  }
};

}

#endif
