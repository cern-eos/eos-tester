// ----------------------------------------------------------------------
// File: ProgressTracker.hh
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

#ifndef EOSTESTER_PROGRESS_TRACKER_H
#define EOSTESTER_PROGRESS_TRACKER_H

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

namespace eostest {

class TestcaseStatus;

class ProgressTracker {
public:
  ProgressTracker(int32_t totalOperations);
  ~ProgressTracker();

  void addInFlight();
  void addSuccessful();
  void addFailed();

  int32_t getInFlight();
  int32_t getSuccessful();
  int32_t getFailed();
  int32_t getPending();

  bool totalKnown() const;

  template<typename T>
  T filterFuture(T&& fut) {
    addInFlight();
    return std::move(fut).filter(std::bind(&ProgressTracker::futureCallback<typename T::value_type>, this, std::placeholders::_1));
  }

  template<typename T>
  bool futureCallback(const T& status) {
    if(status.ok()) {
      addSuccessful();
    }
    else {
      addFailed();
    }

    return true;
  }

  void setDescription(const std::string &str);
  std::string getDescription() const;

private:
  int32_t total;
  std::atomic<int32_t> inFlight {0};
  std::atomic<int32_t> successful {0};
  std::atomic<int32_t> failed {0};

  std::string description;
};

}

#endif
