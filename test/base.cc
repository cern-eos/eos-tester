// ----------------------------------------------------------------------
// File: base.cc
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
#include "HashCalculator.hh"
#include "Utils.hh"
#include "utils/ProgressTracker.hh"
#include "utils/Sealing.hh"
#include "Macros.hh"
#include <rang.hpp>
using namespace eostest;

TEST(HashCalculator, BasicSanity) {
  std::string hash = HashCalculator::base16Encode(HashCalculator::sha256("lalalal"));
  ASSERT_EQ(hash, "1ab094d49f13d198d8e5a80d44e697bd82756ad63403ab75ffb7b5d6c8fcdac6");
}

TEST(Utils, chopPath) {
  ASSERT_EQ("/eos/some/path", chopPath("/eos/some/path/abc"));
}

TEST(Utils, extractLineWithPrefix) {
  std::string contents ="abc\nFILENAME: adgfas\nasdfa";
  std::string extracted;
  ASSERT_TRUE(extractLineWithPrefix(contents, 4, "FILENAME: ", extracted));
  ASSERT_EQ(extracted, "adgfas");

  contents ="abc\nFILENAME:: adgfas\nasdfa";
  ASSERT_FALSE(extractLineWithPrefix(contents, 4, "FILENAME: ", extracted));

  contents ="abc\nFILENAME: adgfas\n";
  ASSERT_TRUE(extractLineWithPrefix(contents, 4, "FILENAME: ", extracted));
  ASSERT_EQ(extracted, "adgfas");

  contents ="abc\nFILENAME: adgfas";
  ASSERT_FALSE(extractLineWithPrefix(contents, 4, "FILENAME: ", extracted));
}

TEST(Utils, ProgressTracker) {
  ProgressTracker tracker(100);

  ASSERT_EQ(tracker.getPending(), 100);
  ASSERT_EQ(tracker.getInFlight(), 0);

  ASSERT_THROW(tracker.addSuccessful(), FatalException);
  ASSERT_THROW(tracker.addFailed(), FatalException);

  tracker.addInFlight();

  ASSERT_EQ(tracker.getPending(), 99);
  ASSERT_EQ(tracker.getInFlight(), 1);
  ASSERT_EQ(tracker.getSuccessful(), 0);
  ASSERT_EQ(tracker.getFailed(), 0);

  tracker.addSuccessful();

  ASSERT_EQ(tracker.getPending(), 99);
  ASSERT_EQ(tracker.getInFlight(), 0);
  ASSERT_EQ(tracker.getSuccessful(), 1);
  ASSERT_EQ(tracker.getFailed(), 0);
}

TEST(Utils, ProgressTrackerOverflow) {
  ProgressTracker tracker(2);

  tracker.addInFlight();
  tracker.addInFlight();

  ASSERT_THROW(tracker.addInFlight(), FatalException);

  ASSERT_EQ(tracker.getPending(), 0);
  ASSERT_EQ(tracker.getInFlight(), 2);
  ASSERT_EQ(tracker.getSuccessful(), 0);
  ASSERT_EQ(tracker.getFailed(), 0);

  tracker.addSuccessful();
  tracker.addFailed();

  ASSERT_EQ(tracker.getPending(), 0);
  ASSERT_EQ(tracker.getInFlight(), 0);
  ASSERT_EQ(tracker.getSuccessful(), 1);
  ASSERT_EQ(tracker.getFailed(), 1);
}

TEST(Utils, ProgressTrackerWithUnknownNumberOfOperations) {
  ProgressTracker tracker(-1);

  ASSERT_EQ(tracker.getPending(), 0);
  ASSERT_EQ(tracker.getInFlight(), 0);

  ASSERT_THROW(tracker.addSuccessful(), FatalException);

  tracker.addInFlight();

  ASSERT_EQ(tracker.getPending(), 0);
  ASSERT_EQ(tracker.getInFlight(), 1);
  ASSERT_EQ(tracker.getSuccessful(), 0);
  ASSERT_EQ(tracker.getFailed(), 0);

  tracker.addSuccessful();

  ASSERT_EQ(tracker.getPending(), 0);
  ASSERT_EQ(tracker.getInFlight(), 0);
  ASSERT_EQ(tracker.getSuccessful(), 1);
  ASSERT_EQ(tracker.getFailed(), 0);
}

TEST(Utils, Sealing) {
  folly::Promise<TestcaseStatus> promise;
  folly::Future<TestcaseStatus> fut = Sealing::seal(promise.getFuture(), "A random description");

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  ASSERT_FALSE(fut.isReady());
  promise.setValue(TestcaseStatus());
  ASSERT_TRUE(fut.isReady());

  TestcaseStatus st = fut.get();
  ASSERT_EQ(st.getDescription(), "A random description");
  ASSERT_TRUE(st.getDuration() > std::chrono::microseconds(500));
}

TEST(Utils, VisualTestSuccess) {
  rang::setControlMode(rang::control::Force);

  TestcaseStatus status;
  status.seal("An awesome test", std::chrono::milliseconds(3));

  std::cout << status.prettyPrint() << std::endl;
}

TEST(Utils, VisualTestFailure) {
  rang::setControlMode(rang::control::Force);

  TestcaseStatus status;
  status.seal("A less than awesome test", std::chrono::milliseconds(30));
  status.addError("The Earth blew up.");

  TestcaseStatus st1;
  st1.seal("This sub-test passed", std::chrono::milliseconds(9));

  TestcaseStatus st2;
  st2.seal("This sub-test did not", std::chrono::milliseconds(5));
  st2.addError("The Moon blew up.");

  status.addChild(std::move(st1));
  status.addChild(std::move(st2));

  std::cout << status.prettyPrint() << std::endl;
}
