// ----------------------------------------------------------------------
// File: ProgressTicker.hh
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

#ifndef EOSTESTER_PROGRESS_TICKER_H
#define EOSTESTER_PROGRESS_TICKER_H

#include "utils/AssistedThread.hh"

namespace eostest {

class ProgressTracker;

class ProgressTicker {
public:
  ProgressTicker(ProgressTracker &tracker);
  ~ProgressTicker();

  void main(ThreadAssistant &assistant);
  void stop();

private:
  AssistedThread thread;
  ProgressTracker &tracker;
};

}

#endif
