// ------------------------------------------------------------------------
// kvu_locks.cpp: Various lock related helper functions.
// Copyright (C) 2000-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#ifdef HAVE_CONFIG_H
#include <config.h> /* for USE_ATOMIC */
#endif

#ifdef ECA_USE_ASM_ATOMIC
#include <asm/atomic.h>
#else
#ifdef __GNUC__
#warning "locks.h: ECA_USE_ASM_ATOMIC not defined!"
#endif
#endif

#include <iostream> /* FIXME: remove me! */

#include <pthread.h>
#include <sys/errno.h>

#include "kvu_dbc.h"
#include "kvu_locks.h"

ATOMIC_INTEGER::ATOMIC_INTEGER(int value)
{
#ifdef USE_ASM_ATOMIC
  atomic_t* ptr = new atomic_t;
  value_repp = reinterpret_cast<void*>(ptr);
  atomic_set(ptr, value);
#else
  pthread_mutex_t* mutex = new pthread_mutex_t;
  mutex_repp = reinterpret_cast<void*>(mutex);
  pthread_mutex_init(mutex, NULL);
  set(value);
#endif
}
 
ATOMIC_INTEGER::~ATOMIC_INTEGER(void)
{
#ifdef USE_ASM_ATOMIC
  atomic_t* ptr = reinterpret_cast<atomic_t*>(value_repp);
  delete ptr;
#else
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(mutex_repp);
  pthread_mutex_destroy(mutex);
#endif
}


int ATOMIC_INTEGER::get(void) const
{
#ifdef USE_ASM_ATOMIC
  return(atomic_read(reinterpret_cast<atomic_t*>(value_repp)));
#else
  int temp;
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(mutex_repp);
  pthread_mutex_lock(mutex);
  temp = value_rep;
  pthread_mutex_unlock(mutex);
  return(temp);
#endif
}

void ATOMIC_INTEGER::set(int value)
{
#ifdef USE_ASM_ATOMIC
  atomic_set(reinterpret_cast<atomic_t*>(value_repp), value);
#else
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(mutex_repp);
  pthread_mutex_lock(mutex);
  value_rep = value;
  pthread_mutex_unlock(mutex);
#endif
}

void ATOMIC_INTEGER::add(int value)
{
#ifdef USE_ASM_ATOMIC
  atomic_add(value, reinterpret_cast<atomic_t*>(value_repp));
#else
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(mutex_repp);
  pthread_mutex_lock(mutex);
  value_rep += value;
  pthread_mutex_unlock(mutex);
#endif
}

void ATOMIC_INTEGER::subtract(int value)
{
#ifdef USE_ASM_ATOMIC
  atomic_sub(value, reinterpret_cast<atomic_t*>(value_repp));
#else
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(mutex_repp);
  pthread_mutex_lock(mutex);
  value_rep -= value;
  pthread_mutex_unlock(mutex);
#endif
}

void ATOMIC_INTEGER::increment(void)
{
#ifdef USE_ASM_ATOMIC
  atomic_inc(reinterpret_cast<atomic_t*>(value_repp));
#else
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(mutex_repp);
  pthread_mutex_lock(mutex);
  ++value_rep;
  pthread_mutex_unlock(mutex);
#endif
}

void ATOMIC_INTEGER::decrement(void)
{
#ifdef USE_ASM_ATOMIC
  atomic_dec(reinterpret_cast<atomic_t*>(value_repp));
#else
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(mutex_repp);
  pthread_mutex_lock(mutex);
  --value_rep;
  pthread_mutex_unlock(mutex);
#endif
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
