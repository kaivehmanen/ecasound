// ------------------------------------------------------------------------
// definition_by_contract.h: Class for simulating desing-by-contract
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#ifndef _DEFINITION_BY_CONTRACT_H
#define _DEFINITION_BY_CONTRACT_H

#include <iostream>

/**
 * Exception class that is thrown when some contract 
 * fails
 *
 * @author Kai Vehmanen
 */
class DBC_EXCEPTION { 
 private:
  
  const char* type_rep;
  const char* file_rep;
  int line_rep;

 public:

  void print(void) { cerr << "Failed condition \"" << type_rep << "\": file " << file_rep << ", line " << line_rep << ".\n"; }
  DBC_EXCEPTION(const char* type, const char* file, int line) 
    : type_rep(type), file_rep(file), line_rep(line) { }
};

/**
 * Tools for simulating programming/design-by-contract in C++ 
 * classes. Features include routine preconditions, postconditions 
 * and virtual class invariants.
 *
 * @author Kai Vehmanen
 */
class DEFINITION_BY_CONTRACT {
 public:

#ifdef ENABLE_DBC

  inline void require(bool expr, const char* file, int line) const { check_invariant(file, line); if (!expr) throw(new DBC_EXCEPTION("require", file, line)); }
  inline void ensure(bool expr, const char* file, int line) const { if (!expr) throw(new DBC_EXCEPTION("require", file, line)); check_invariant(file, line); }
  inline void check_invariant(const char* file, int line) const { if (!class_invariant()) throw(new DBC_EXCEPTION("class invariant", file, line)); }

#define REQUIRE(expr)							      \
   (expr) ? static_cast<void>(0) :	(require (false,__FILE__, __LINE__))
#define ENSURE(expr)							      \
   (expr) ? static_cast<void>(0) :	(ensure (false,__FILE__, __LINE__))
#else

#define REQUIRE(expr)		((void) 0)
#define ENSURE(expr)		((void) 0)

  inline void require(bool expr, const char* file, int line) const { }
  inline void ensure(bool expr, const char* file, int line) const { }
  inline void check_invariant(const char* file, int line) const { }

#endif

  virtual bool class_invariant(void) const { return(true); }

  DEFINITION_BY_CONTRACT(void) { }
};

#endif
