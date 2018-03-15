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
#include "testcases/TreeBuilder.hh"
using namespace eostest;

TEST(XrdClExecutor, BasicSanity) {
  OperationStatus status = XrdClExecutor::mkdir(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/").get();
  ASSERT_TRUE(status.ok());

  status = XrdClExecutor::rm(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/f1").get();
  ASSERT_FALSE(status.ok());

  status = XrdClExecutor::put(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/f1", "adfasf").get();
  ASSERT_TRUE(status.ok());

  ReadStatus rstatus = XrdClExecutor::get(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/f1").get();
  ASSERT_TRUE(rstatus.ok());
  ASSERT_EQ(rstatus.contents, "adfasf");

  DirListStatus lstatus = XrdClExecutor::dirList(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity").get();
  ASSERT_TRUE(lstatus.ok());
  ASSERT_EQ(lstatus.contents->GetSize(), 1u);
  ASSERT_EQ(lstatus.contents->At(0)->GetName(), "f1");
  ASSERT_FALSE(lstatus.contents->At(0)->GetStatInfo()->TestFlags(XrdCl::StatInfo::IsDir));

  status = XrdClExecutor::rm(1, "root://eospps.cern.ch///eos/user/gbitzes/eostester/sanity/f1").get();
  ASSERT_TRUE(status.ok());
}

TEST(TreeBuilder, BasicSanity) {
  TreeBuilder::Options opts;
  opts.baseUrl = "root://eospps.cern.ch//eos/user/gbitzes/eostester/tree";
  opts.seed = 42;
  opts.depth = 5;
  opts.files = 100;

  TreeBuilder builder(opts);

  ErrorAccumulator acc = builder.initialize().get();
  if(!acc.ok()) {
    std::cout << acc.toString() << std::endl;
  }

  ASSERT_TRUE(acc.ok());
}
