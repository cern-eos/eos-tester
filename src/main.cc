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

int main(int argc, char **argv) {
  // Reset terminal colors on exit
  std::atexit([](){std::cout << rang::style::reset;});

  int32_t randomSeed = 0;
  std::string targetPath = "";

  CLI::App app{"This tool collects a number of functional and stress tests for the EOS storage system."};
  app.add_option("--path", targetPath, "The path under which all tests will be run.")
    ->required();

  app.add_option("--seed", randomSeed, "Seed for random number generation");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  return 0;
}
