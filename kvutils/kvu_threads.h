// ------------------------------------------------------------------------
// kvu_threads.h: Various pthread related helper functions.
// Copyright (C) 2002,2004,2012 Kai Vehmanen
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

// -*- mode: C++; -*-
#ifndef INCLUDED_KVU_THREADS_H
#define INCLUDED_KVU_THREADS_H

#include <string>
#include <pthread.h>

int kvu_pthread_mutex_spinlock (pthread_mutex_t *mp, long int spinlimit);
int kvu_pthread_timed_wait(pthread_mutex_t* mutex, pthread_cond_t* cond, long int seconds);
std::string kvu_pthread_timed_wait_result(int result, const std::string& prefix);
int kvu_pthread_cond_timeout(int seconds, struct timespec *out, bool monotonic);

#endif
