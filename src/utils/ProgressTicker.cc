// ----------------------------------------------------------------------
// File: ProgressTicker.cc
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

#include "ProgressTicker.hh"
#include "utils/ProgressTracker.hh"
#include <iostream>
using namespace eostest;

ProgressTicker::ProgressTicker(ProgressTracker &track) : tracker(track) {
  thread.reset(&ProgressTicker::main, this);
}

ProgressTicker::~ProgressTicker() {}

void ProgressTicker::main(ThreadAssistant &assistant) {
  while(!assistant.terminationRequested()) {
    std::cout << "\x1b[2K\r";
    std::cout << "Pending: " << tracker.getPending() << ", in-flight: " << tracker.getInFlight() << ", succeeded: " << tracker.getSuccessful() << ", failed: " << tracker.getFailed() << "\r" << std::flush;
    assistant.wait_for(std::chrono::seconds(1));
  }
}
