// ----------------------------------------------------------------------
// File: ProgressTracker.cc
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

#include "ProgressTracker.hh"
#include "Macros.hh"
using namespace eostest;

ProgressTracker::ProgressTracker(int32_t ops) : total(ops) { }
ProgressTracker::~ProgressTracker() {}

void ProgressTracker::addInFlight() {
  eost_assert(!totalKnown() || (inFlight + 1 + successful + failed) <= total);
  inFlight++;
}

void ProgressTracker::addSuccessful() {
  eost_assert(inFlight >= 1);

  successful++;
  inFlight--;
}

void ProgressTracker::addFailed() {
  eost_assert(inFlight >= 1);

  failed++;
  inFlight--;
}

int32_t ProgressTracker::getInFlight() {
  return inFlight;
}

int32_t ProgressTracker::getSuccessful() {
  return successful;
}

int32_t ProgressTracker::getFailed() {
  return failed;
}

int32_t ProgressTracker::getPending() {
  if(!totalKnown()) return 0;
  return total - (inFlight + successful + failed);
}

bool ProgressTracker::totalKnown() const {
  return total > 0;
}
