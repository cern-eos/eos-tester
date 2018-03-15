// ----------------------------------------------------------------------
// File: ErrorAccumulator.cc
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
#include "../Macros.hh"
#include "ErrorAccumulator.hh"
using namespace eostest;

ErrorAccumulator::ErrorAccumulator() {}

void ErrorAccumulator::addError(const std::string &err) {
  errors.push_back(err);
}

bool ErrorAccumulator::ok() const {
  return errors.empty();
}

std::string ErrorAccumulator::toString() const {
  std::ostringstream ss;
  for(size_t i = 0; i < errors.size(); i++) {
    ss << "- " << errors[i] << std::endl;
  }

  return ss.str();
}

void ErrorAccumulator::absorbErrors(const ErrorAccumulator &acc) {
  for(size_t i = 0; i < acc.errors.size(); i++) {
    errors.push_back(SSTR("    " << acc.errors[i]));
  }
}
