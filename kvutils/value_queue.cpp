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
#include <pthread.h>

#include "value_queue.h"

VALUE_QUEUE::VALUE_QUEUE(void) { 
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cond, NULL);
  locked_rep = false;
}

void VALUE_QUEUE::push_back(int key, double value) {
  pthread_mutex_lock(&lock);
  while (locked_rep == true) {
    pthread_cond_wait(&cond, &lock);
  }
  locked_rep = true;

  cmds.push_back(pair<int,double>(key, value));

  locked_rep = false;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);
}

void VALUE_QUEUE::pop_front(void) {
  // --------
  // require:
  assert(is_empty() == false);
  // --------

  pthread_mutex_lock(&lock);
  while (locked_rep == true) {
    pthread_cond_wait(&cond, &lock);
  }
  locked_rep = true;

  cmds.pop_front();

  locked_rep = false;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);
}    

const pair<int,double>& VALUE_QUEUE::front(void) {
  // --------
  // require:
  assert(is_empty() == false);
  // --------

  pthread_mutex_lock(&lock);
  while (locked_rep == true) {
    pthread_cond_wait(&cond, &lock);
  }
  locked_rep = true;

  const pair<int,double>& s = cmds.front();

  locked_rep = false;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);
  return(s);
}

bool VALUE_QUEUE::is_empty(void) const {
  pthread_mutex_lock(&lock);
  while (locked_rep == true) {
    pthread_cond_wait(&cond, &lock);
  }
  locked_rep = true;
  
  bool result = cmds.empty(); 

  locked_rep = false;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);

  return(result);
}
