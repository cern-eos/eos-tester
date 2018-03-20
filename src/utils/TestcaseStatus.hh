// ----------------------------------------------------------------------
// File: ErrorAccumulator.hh
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

#ifndef EOSTESTER_ERROR_ACCUMULATOR_H
#define EOSTESTER_ERROR_ACCUMULATOR_H

#include <vector>
#include <string>

namespace eostest {

class TestcaseStatus {
public:
  TestcaseStatus();
  TestcaseStatus(const std::string &err);
  void addError(const std::string &err);
  bool ok() const;
  void absorbErrors(const TestcaseStatus &acc);
  std::string toString() const;

private:
  std::vector<std::string> errors;
};

}

#endif
