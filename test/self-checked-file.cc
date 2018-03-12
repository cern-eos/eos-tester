// ----------------------------------------------------------------------
// File: self-checked-file.cc
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

#include <gtest/gtest.h>
#include "SelfCheckedFile.hh"
using namespace eostest;

TEST(SelfCheckedFile, BasicSanity) {
  SelfCheckedFile scf("/eos/pps/base/f1", "some random bytes");

  ASSERT_EQ(scf.toString(),
    "FILENAME: /eos/pps/base/f1\n"
    "RANDOM-BYTES: 17\n"
    "----------\n"
    "some random bytes\n"
    "----------\n"
    "3d5b6e328474107d6a1c6d61d92a160e0d1eb4d3c3b99bc8574ebde0fba02eba\n"
  );

}
