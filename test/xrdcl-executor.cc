// ----------------------------------------------------------------------
// File: xrdcl-executor.cc
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
#include "XrdClExecutor.hh"
using namespace eostest;

TEST(DISABLED_XrdClExecutor, BasicSanity) {
  OperationStatus status = XrdClExecutor::mkdir(1, "root://eospps.cern.ch///eos/user/gbitzes/eostester/").get();
  ASSERT_TRUE(status.ok());

  XrdClExecutor::put(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/f1", "adfasf");

}
