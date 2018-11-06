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
#include "TestcaseStatus.hh"
#include "Styling.hh"
using namespace eostest;

TestcaseStatus::TestcaseStatus() {}

TestcaseStatus::TestcaseStatus(const std::string &err) {
  addError(err);
}

void TestcaseStatus::addError(const std::string &err) {
  errors.push_back(err);
}

bool TestcaseStatus::ok() const {
  if(!errors.empty()) return false;

  for(size_t i = 0; i < children.size(); i++) {
    if(!children[i].ok()) return false;
  }

  return true;
}

std::string TestcaseStatus::toString() const {
  std::ostringstream ss;

  ss << description << std::endl;
  for(size_t i = 0; i < errors.size(); i++) {
    ss << "- " << errors[i] << std::endl;
  }

  return ss.str();
}

bool TestcaseStatus::absorbErrors(const TestcaseStatus &acc) {
  // This function simply adds all errors found in the given TestcaseStatus,
  // but retains its own description.
  // addChild() is a better choice, if you want to retain the description of
  // sub-errors.

  for(size_t i = 0; i < acc.errors.size(); i++) {
    errors.push_back(acc.errors[i]);
  }

  return !acc.errors.empty();

}

void TestcaseStatus::seal(const std::string &descr, std::chrono::nanoseconds dur) {
  description = descr;
  duration = dur;
}

std::chrono::nanoseconds TestcaseStatus::getDuration() {
  return duration;
}

void TestcaseStatus::addChild(TestcaseStatus &&child) {
  children.emplace_back(std::move(child));
}

std::string& TestcaseStatus::getDescription() {
  return description;
}

static void printMany(std::ostringstream &ss, const std::string &str, size_t times) {
  for(size_t i = 0; i < times; i++) {
    ss << str;
  }
}

std::string TestcaseStatus::prettyPrint(size_t level) const {
  std::ostringstream ss;

  printMany(ss, " ", level*4);

  if(ok()) {
    ss << Styling::success("PASS");
  }
  else {
    ss << Styling::failure("FAIL");
  }

  ss << " " << description << std::endl;

  for(size_t i = 0; i < errors.size(); i++) {
    printMany(ss, " ", (level+1)*4);
    ss << errors[i] << std::endl;
  }

  for(size_t i = 0; i < children.size(); i++) {
    ss << children[i].prettyPrint(level+1);
  }

  return ss.str();
}
