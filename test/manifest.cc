// ----------------------------------------------------------------------
// File: manifest.cc
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
#include "Manifest.hh"
#include "HashCalculator.hh"
using namespace eostest;

TEST(Manifest, BasicSanity) {
  Manifest manifest("/eos/pps/base/somedir/manifest");

  // empty manifest
  ASSERT_EQ(manifest.toString(),
    "MANIFEST: /eos/pps/base/somedir/manifest\n"
    "----------\n"
    "----------\n"
    "----------\n"
    "d5cd8d066bf2abb559bc7423407550912a794c0fd445113b90259ad7647d22b3\n"
  );

  manifest.addFile("f1");
  manifest.addFile("f2");
  manifest.addFile("f3");

  ASSERT_EQ(manifest.toString(),
    "MANIFEST: /eos/pps/base/somedir/manifest\n"
    "----------\n"
    "----------\n"
    "FILE: f1\n"
    "FILE: f2\n"
    "FILE: f3\n"
    "----------\n"
    "37d8794ac29b2f7bcf80ea8477d7b1e210de6334f3315e20e13a3c873ae9895c\n"
  );

  ASSERT_EQ(manifest.checksum(), HashCalculator::sha256(manifest.toStringWithoutChecksum()));

  manifest.addSubdir("dir1");
  manifest.addSubdir("dir2");
  manifest.addSubdir("dir3");

  ASSERT_EQ(manifest.toString(),
    "MANIFEST: /eos/pps/base/somedir/manifest\n"
    "----------\n"
    "SUBDIR: dir1\n"
    "SUBDIR: dir2\n"
    "SUBDIR: dir3\n"
    "----------\n"
    "FILE: f1\n"
    "FILE: f2\n"
    "FILE: f3\n"
    "----------\n"
    "11d888bdbe49f3ac1709d7c7b9fe2bd1579915ed8a431982dad42fd3b9fdb468\n"
  );

  ASSERT_EQ(manifest.checksum(), HashCalculator::sha256(manifest.toStringWithoutChecksum()));

  std::string tmp;
  ASSERT_TRUE(manifest.popFile(tmp));
  ASSERT_EQ(tmp, "f1");

  ASSERT_TRUE(manifest.popFile(tmp));
  ASSERT_EQ(tmp, "f2");

  ASSERT_TRUE(manifest.popFile(tmp));
  ASSERT_EQ(tmp, "f3");

  ASSERT_FALSE(manifest.popFile(tmp));

  ASSERT_TRUE(manifest.popSubdir(tmp));
  ASSERT_EQ(tmp, "dir1");

  ASSERT_TRUE(manifest.popSubdir(tmp));
  ASSERT_EQ(tmp, "dir2");

  ASSERT_TRUE(manifest.popSubdir(tmp));
  ASSERT_EQ(tmp, "dir3");

  ASSERT_FALSE(manifest.popSubdir(tmp));
}

TEST(Manifest, Parsing) {
  std::string contents =
    "MANIFEST: /eos/pps/base/somedir/manifest\n"
    "----------\n"
    "----------\n"
    "FILE: f1\n"
    "FILE: f2\n"
    "FILE: f3\n"
    "----------\n"
    "37d8794ac29b2f7bcf80ea8477d7b1e210de6334f3315e20e13a3c873ae9895c\n";

  Manifest manifest;
  ASSERT_TRUE(manifest.parse(contents));
  ASSERT_EQ(manifest.getFilename(), "/eos/pps/base/somedir/manifest");

  ASSERT_TRUE(manifest.getDirectories().empty());

  std::string item;
  ASSERT_TRUE(manifest.popFile(item));
  ASSERT_EQ(item, "f1");

  ASSERT_TRUE(manifest.popFile(item));
  ASSERT_EQ(item, "f2");

  ASSERT_TRUE(manifest.popFile(item));
  ASSERT_EQ(item, "f3");

  ASSERT_FALSE(manifest.popFile(item));

  contents =
    "MANIFEST: /eos/pps/base/somedir/manifest\n"
    "----------\n"
    "----------\n"
    "FILE: f1\n"
    "FILE: f2\n"
    "FILE: f3\n"
    "----------\n"
    "3718794ac29b2f7bcf80ea8477d7b1e210de6334f3315e20e13a3c873ae9895c\n";

  ASSERT_FALSE(manifest.parse(contents));
}
