// ----------------------------------------------------------------------
// File: Macros.hh
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

#ifndef EOSTESTER_MACROS_H
#define EOSTESTER_MACROS_H

#include <sstream>

namespace eostest {

class FatalException : public std::exception {
public:
  FatalException(const std::string &m) : msg(m) {}
  virtual ~FatalException() {}

  virtual const char* what() const noexcept {
    return msg.c_str();
  }

private:
  std::string msg;
};

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()
#define DBG(message) std::cout << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl

#define eost_assert(condition) if(!((condition))) throw FatalException(SSTR("assertion violation, condition is not true: " << #condition))

}

#endif
