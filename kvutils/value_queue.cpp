// ------------------------------------------------------------------------
// value_queue.cpp: A thread-safe way to transmit int-double pairs.
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

#include <string>
#include <deque>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

#include "value_queue.h"

VALUE_QUEUE::VALUE_QUEUE(void) { 
  pthread_mutex_init(&lock_rep, NULL);
  pthread_cond_init(&cond_rep, NULL);
  empty_rep = pair<int,double>(0, 0.0f);
}

void VALUE_QUEUE::push_back(int key, double value) {
  pthread_mutex_lock(&lock_rep);

  cmds_rep.push_back(pair<int,double>(key, value));

  pthread_cond_broadcast(&cond_rep);
  pthread_mutex_unlock(&lock_rep);
}

void VALUE_QUEUE::pop_front(void) {
  // --------
  // require:
  assert(is_empty() == false);
  // --------

  pthread_mutex_lock(&lock_rep);

  cmds_rep.pop_front();

  pthread_mutex_unlock(&lock_rep);
}    

const pair<int,double>& VALUE_QUEUE::front(void) {
  // --------
  // require:
  assert(is_empty() == false);
  // --------

  pthread_mutex_lock(&lock_rep);

  const pair<int,double>& s = cmds_rep.front();

  pthread_mutex_unlock(&lock_rep);
  return(s);
}

void VALUE_QUEUE::poll(int timeout_sec,
		       long int timeout_usec) {
  struct timeval now;
  struct timespec timeout;
  int retcode;

  pthread_mutex_lock(&lock_rep);
  gettimeofday(&now, 0);
  timeout.tv_sec = now.tv_sec + timeout_sec;
  timeout.tv_nsec = now.tv_usec * 1000 + timeout_usec * 1000;
  retcode = 0;
  while (cmds_rep.empty() == true && retcode != ETIMEDOUT) {
    retcode = pthread_cond_timedwait(&cond_rep, &lock_rep, &timeout);
  }
  pthread_mutex_unlock(&lock_rep);
  return;
}

bool VALUE_QUEUE::is_empty(void) const {
  pthread_mutex_lock(&lock_rep);
  
  bool result = cmds_rep.empty(); 

  pthread_mutex_unlock(&lock_rep);

  return(result);
}
