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

void VALUE_QUEUE::push_back(int key, double value) {
  pthread_mutex_lock(&lock);
  cmds.push_back(pair<int,double>(key, value));
  pthread_mutex_unlock(&lock);
}

void VALUE_QUEUE::pop_front(void) {
  // --------
  // require:
  assert(is_empty() == false);
  // --------

  pthread_mutex_lock(&lock);
  cmds.pop_front();
  pthread_mutex_unlock(&lock);
}    

const pair<int,double>& VALUE_QUEUE::front(void) {
  // --------
  // require:
  assert(is_empty() == false);
  // --------

  pthread_mutex_lock(&lock);
  const pair<int,double>& s = cmds.front();
  pthread_mutex_unlock(&lock);
  return(s);
}

bool VALUE_QUEUE::is_empty(void) const {
  return cmds.empty(); 
}
