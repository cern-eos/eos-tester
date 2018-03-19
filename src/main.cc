// ----------------------------------------------------------------------
// File: main.cc
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

#include <iostream>
#include <sstream>
#include <rang.hpp>
#include <CLI11.hpp>

#include "testcases/TreeBuilder.hh"
#include "testcases/TreeValidator.hh"

using namespace eostest;

int main(int argc, char **argv) {
  // Reset terminal colors on exit
  std::atexit([](){std::cout << rang::style::reset;});

  TreeBuilder::Options builderOpts;
  std::string targetPath = "";

  CLI::App app{"This tool collects a number of functional and stress tests for the EOS storage system."};

  auto treeSubcommand = app.add_subcommand("tree", "Build and verify namespace trees");
  app.require_subcommand();

  auto buildOpt = treeSubcommand->add_option("--build", targetPath, "Build a namespace tree in the specified URL.");
  auto seedOpt = treeSubcommand->add_option("--seed", builderOpts.seed, "Random seed to use when building a namespace tree.")
    ->needs(buildOpt);

  auto depthOpt = treeSubcommand->add_option("--depth", builderOpts.depth, "The depth of the namespace tree to be created.")
    ->needs(buildOpt);

  auto nfilesOpt = treeSubcommand->add_option("--nfiles", builderOpts.files, "The size in number of files for the namesapce tree to build")
   ->needs(buildOpt);

  auto validateOpt = treeSubcommand->add_option("--validate", targetPath, "Verify a namespace tree present in the specified URL.")
    ->excludes(buildOpt)
    ->excludes(seedOpt)
    ->excludes(depthOpt)
    ->excludes(nfilesOpt);

  buildOpt->group("Operation");
  validateOpt->group("Operation");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  if(*buildOpt) {
    builderOpts.baseUrl = targetPath;
    TreeBuilder builder(builderOpts);

    ErrorAccumulator accu = builder.initialize().get();
    if(!accu.ok()) {
      std::cout << accu.toString() << std::endl;
      return 1;
    }
  }
  else if(*validateOpt) {
    TreeValidator validator(targetPath);
    ErrorAccumulator accu = validator.initialize().get();

    if(!accu.ok()) {
      std::cout << accu.toString() << std::endl;
      return 1;
    }
  }

  return 0;
}
