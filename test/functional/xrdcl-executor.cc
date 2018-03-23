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
#include "testcases/TreeValidator.hh"
#include "utils/ProgressTracker.hh"
#include "utils/ProgressTicker.hh"
using namespace eostest;

TEST(XrdClExecutor, BasicSanity) {
  TestcaseStatus status = XrdClExecutor::mkdir(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/").get();
  ASSERT_TRUE(status.ok()) << status.toString();

  status = XrdClExecutor::rm(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/f1").get();
  ASSERT_FALSE(status.ok());

  status = XrdClExecutor::put(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/f1", "adfasf").get();
  ASSERT_TRUE(status.ok());

  ReadStatus rstatus = XrdClExecutor::get(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/f1").get();
  ASSERT_TRUE(rstatus.ok());
  ASSERT_EQ(rstatus.contents, "adfasf");
  ASSERT_EQ(rstatus.getDescription(), "xroot::get on 'root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity/f1'");

  DirListStatus lstatus = XrdClExecutor::dirList(1, "root://eospps.cern.ch//eos/user/gbitzes/eostester/sanity").get();
  ASSERT_TRUE(lstatus.ok());
  ASSERT_EQ(lstatus.contents->GetSize(), 1u);
  ASSERT_EQ(lstatus.contents->At(0)->GetName(), "f1");
  ASSERT_FALSE(lstatus.contents->At(0)->GetStatInfo()->TestFlags(XrdCl::StatInfo::IsDir));

  status = XrdClExecutor::rm(1, "root://eospps.cern.ch///eos/user/gbitzes/eostester/sanity/f1").get();
  ASSERT_TRUE(status.ok());
}

TEST(TreeValidator, BasicSanity) {
  ASSERT_EQ(system("gfal-rm -r root://eospps.cern.ch///eos/user/gbitzes/eostester/tree-simple/"), 0);

  TreeBuilder::Options opts;
  opts.baseUrl = "root://eospps.cern.ch//eos/user/gbitzes/eostester/tree-simple";
  opts.seed = 42;
  opts.depth = 5;
  opts.files = 100;

  ProgressTracker tracker2(opts.files);
  TreeBuilder builder(opts, &tracker2);
  ProgressTicker ticker2(tracker2);

  TestcaseStatus acc = builder.initialize().get();
  ticker2.stop();

  std::cout << acc.prettyPrint();
  ASSERT_TRUE(acc.ok());

  ProgressTracker tracker(-1);
  TreeValidator validator(opts.baseUrl, &tracker);
  ProgressTicker ticker(tracker);
  acc = validator.initialize().get();

  ticker.stop();
  std::cout << acc.prettyPrint();
  ASSERT_TRUE(acc.ok());
}

TEST(ProgressTicker, VisualTest) {
  ProgressTracker tracker(5);
  ProgressTicker ticker(tracker);

  std::this_thread::sleep_for(std::chrono::seconds(5));





}
