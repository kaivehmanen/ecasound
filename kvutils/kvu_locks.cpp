// ------------------------------------------------------------------------
// kvu_locks.cpp: Various lock related helper functions.
// Copyright (C) 2000-2002 Kai Vehmanen
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

#include <signal.h>         /* ANSI-C: sig_atomic_t */
#include <pthread.h>        /* POSIX: threading */

#include "kvu_dbc.h"
#include "kvu_locks.h"

/* NOTE: This implementation doesn't really guarantee correct
 *       operation at the moment as I wanted to avoid 
 *       maintaining primitives for dozens of different 
 *       platforms.
 *
 *       The primary reason for using this class is to 
 *       mark all code segments where atomic access 
 *       is required.
 *
 * Tested platforms:
 *   - IA32 single- and multi-processor cases
 *
 * Platforms that should work according to specs:
 *   - Alpha
 *   - ARM
 *   - IA64
 *   - PowerPC
 * 
 * Platforms that do _NOT_ work:
 *   - multi-processor SPARC
 *   - IBM's S390
 */

ATOMIC_INTEGER::ATOMIC_INTEGER(int value)
{
  value_rep = value;
}

ATOMIC_INTEGER::~ATOMIC_INTEGER(void) 
{
}

int ATOMIC_INTEGER::get(void) const
{
  return(value_rep);
}

void ATOMIC_INTEGER::set(int value)
{
  value_rep = value;
}

KVU_GUARD_LOCK::KVU_GUARD_LOCK(pthread_mutex_t* lock_arg)
{
  lock_repp = lock_arg;
  DBC_CHECK(pthread_mutex_lock(lock_repp) == 0);
}

KVU_GUARD_LOCK::~KVU_GUARD_LOCK(void)
{
  DBC_CHECK(pthread_mutex_unlock(lock_repp) == 0);
}
