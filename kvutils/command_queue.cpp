// ------------------------------------------------------------------------
// command_queue.cpp: Thread-safe way to transmit string objects.
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
#include <pthread.h>

#include "command_queue.h"

pthread_mutex_t _cq_lock = PTHREAD_MUTEX_INITIALIZER;

void COMMAND_QUEUE::push_back(const string& cmd) {
  //  cerr << "pushbefore";
  pthread_mutex_lock(&_cq_lock);
  //  cerr << "pushon";
  cmds.push_back(cmd);
  pthread_mutex_unlock(&_cq_lock);
  //  cerr << "pushoff\n";
}

void COMMAND_QUEUE::pop_front(void) {
  pthread_mutex_lock(&_cq_lock);
  cmds.pop_front();
  pthread_mutex_unlock(&_cq_lock);
}    

string COMMAND_QUEUE::front(void) const {
  //    if (pthread_mutex_trylock(&_cq_lock) == EBUSY)
  int n = pthread_mutex_trylock(&_cq_lock);
  //  cerr << "Front returned " << n << ".\n"; 
  if (n != 0)
    return("");
  //  cerr << "fronton";
  if (cmds.size() == 0) {
    pthread_mutex_unlock(&_cq_lock);  
    return("");
  }
  string s = cmds.front();
  pthread_mutex_unlock(&_cq_lock);   
  //  cerr << "frontoff\n";
  return(s);
}

bool COMMAND_QUEUE::cmds_available(void) {
  //    if (pthread_mutex_trylock(&_cq_lock) == EBUSY)
    if (pthread_mutex_trylock(&_cq_lock) != 0)
        return(false);
    bool temp = false;
    if (cmds.size() > 0) temp = true;
    pthread_mutex_unlock(&_cq_lock);
    return(temp);
}    

void COMMAND_QUEUE::flush(void) {
 int n = pthread_mutex_trylock(&_cq_lock);
 if (n != 0) return;
 while (cmds.size() > 0) cmds.pop_front();
 pthread_mutex_unlock(&_cq_lock);
}
