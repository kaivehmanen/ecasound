// ------------------------------------------------------------------------
// eca-fileio-mmap-fthread.cpp: Thread sub-routines for mmap based file-io.
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

#include <vector>
#include <map>
#include <pthread.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <errno.h>

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <kvutils.h>

#include "eca-debug.h"
#include "eca-error.h"

#include "eca-fileio-mmap-fthread.h"

bool ecasound_fiommap_running = false;
pthread_mutex_t ecasound_fiommap_lock;
pthread_cond_t ecasound_fiommap_cond;
vector<ECASOUND_FIOMMAP_BUFFER*> ecasound_fiommap_buffer;
map<int,ECASOUND_FIOMMAP_BUFFER*> ecasound_fiommap_buffermap;
pthread_t ecasound_fiommap_thread;
const long int ecasound_fiommap_buffersize = 524288;

void ecasound_fiommap_register_fd(int fd, long int length) {
  //  cerr << "(eca-fileio-mmap-fthread) register_fd()\n";

  if (ecasound_fiommap_running == false) {
    ecasound_fiommap_running = true;
    pthread_mutex_init(&ecasound_fiommap_lock, NULL);
    pthread_cond_init(&ecasound_fiommap_cond, NULL);
    ecasound_fiommap_exec_thread();
  }

  ECASOUND_FIOMMAP_BUFFER* temp = new ECASOUND_FIOMMAP_BUFFER;
  temp->buffers[0] = new unsigned char [ecasound_fiommap_buffersize];
  temp->buffers[1] = new unsigned char [ecasound_fiommap_buffersize];
  temp->ready_for_fill[0] = true;
  temp->ready_for_fill[1] = true;
  temp->fd = fd;
  temp->file_length = length;

  pthread_mutex_lock(&ecasound_fiommap_lock);
  ecasound_fiommap_buffer.push_back(temp);
  ecasound_fiommap_buffermap[fd] = temp;
  pthread_mutex_unlock(&ecasound_fiommap_lock);

  ecasound_fiommap_reset(fd, 0);
}

void ecasound_fiommap_close_fd(int fd) {
  //  cerr << "(eca-fileio-mmap-fthread) close_fd()\n";

  pthread_cancel(ecasound_fiommap_thread);
  pthread_join(ecasound_fiommap_thread,NULL);

  vector<ECASOUND_FIOMMAP_BUFFER*>::iterator p = ecasound_fiommap_buffer.begin();
  while(p != ecasound_fiommap_buffer.end()) {
    if ((*p)->fd == fd) {
     ecasound_fiommap_buffermap.erase(fd);
     ecasound_fiommap_buffer.erase(p);
     break;
    }
    ++p;
  }

  if (ecasound_fiommap_buffer.size() == 0) {
    ecasound_fiommap_running = false;
    pthread_cond_destroy(&ecasound_fiommap_cond);
    pthread_mutex_destroy(&ecasound_fiommap_lock);
  }
  else {
    ecasound_fiommap_exec_thread();
  }
}

void ecasound_fiommap_reset(int fd, long fposition) {
  //  cerr << "(eca-fileio-mmap-fthread) reset()\n";

  if (ecasound_fiommap_buffermap[fd]->mmap_low != -1 &&
      fposition >= ecasound_fiommap_buffermap[fd]->mmap_low &&
      fposition < ecasound_fiommap_buffermap[fd]->mmap_high) return;

  pthread_mutex_lock(&ecasound_fiommap_lock);

  long int mmap_high = 0;
  while (fposition >= mmap_high) {
    mmap_high += ecasound_fiommap_buffersize;
  }

  ecasound_fiommap_buffermap[fd]->mmap_high = mmap_high;
  ecasound_fiommap_buffermap[fd]->mmap_low =  mmap_high - ecasound_fiommap_buffersize;

  munlock(ecasound_fiommap_buffermap[fd]->buffers[0], ecasound_fiommap_buffermap[fd]->mmap_length[0]);
  munmap(ecasound_fiommap_buffermap[fd]->buffers[0], ecasound_fiommap_buffermap[fd]->mmap_length[0]);

  if (mmap_high > ecasound_fiommap_buffermap[fd]->file_length)
    ecasound_fiommap_buffermap[fd]->mmap_length[0] =
      ecasound_fiommap_buffermap[fd]->file_length -
      ecasound_fiommap_buffermap[fd]->mmap_low;
  else
    ecasound_fiommap_buffermap[fd]->mmap_length[0] = ecasound_fiommap_buffersize;
  

  //  cerr << "(eca-fileio-mmap-fthread) mapping[s] (reset) from " << ecasound_fiommap_buffermap[fd]->mmap_low
  //       << " to " << ecasound_fiommap_buffermap[fd]->mmap_low + ecasound_fiommap_buffermap[fd]->mmap_length[0] << " (fd:"
  //       << ecasound_fiommap_buffermap[fd]->fd << ", buf: 0)\n";

  ecasound_fiommap_buffermap[fd]->buffers[0] = (unsigned char*)mmap(0,
								    ecasound_fiommap_buffermap[fd]->mmap_length[0],
								    PROT_READ,
								    MAP_SHARED,
								    ecasound_fiommap_buffermap[fd]->fd,
								    ecasound_fiommap_buffermap[fd]->mmap_low);

  //  cerr << "(eca-fileio-mmap-fthread) mapping[f] (reset) from " << ecasound_fiommap_buffermap[fd]->mmap_low
  //       << " to " << ecasound_fiommap_buffermap[fd]->mmap_high << " (fd:"
  //       << ecasound_fiommap_buffermap[fd]->fd << ", buf: 0).\n";

  ecasound_fiommap_buffermap[fd]->locked_buffer = 0;
  ecasound_fiommap_buffermap[fd]->ready_for_fill[0] = false;
  ecasound_fiommap_buffermap[fd]->ready_for_fill[1] = true;
  pthread_cond_signal(&ecasound_fiommap_cond);
  pthread_mutex_unlock(&ecasound_fiommap_lock);
}

void ecasound_fiommap_next_buffer(int fd) {
  //  cerr << "(eca-fileio-mmap-fthread) next_buffer()\n";

  pthread_mutex_lock(&ecasound_fiommap_lock);
  if (ecasound_fiommap_buffermap[fd]->locked_buffer == 0) {
    pthread_cond_signal(&ecasound_fiommap_cond);
    while(ecasound_fiommap_buffermap[fd]->ready_for_fill[1] == true) {
      pthread_cond_wait(&ecasound_fiommap_cond,
			&ecasound_fiommap_lock);
    }
    ecasound_fiommap_buffermap[fd]->locked_buffer = 1;
    ecasound_fiommap_buffermap[fd]->ready_for_fill[0] = true;
  }
  else if (ecasound_fiommap_buffermap[fd]->locked_buffer == 1) {
    pthread_cond_signal(&ecasound_fiommap_cond);
    while(ecasound_fiommap_buffermap[fd]->ready_for_fill[0] == true) {
      pthread_cond_wait(&ecasound_fiommap_cond,
			&ecasound_fiommap_lock);
    }
    ecasound_fiommap_buffermap[fd]->locked_buffer = 0;
    ecasound_fiommap_buffermap[fd]->ready_for_fill[1] = true;
  }
  pthread_cond_signal(&ecasound_fiommap_cond);
  pthread_mutex_unlock(&ecasound_fiommap_lock);
}

unsigned char* ecasound_fiommap_active_buffer(int fd) {
  return(ecasound_fiommap_buffermap[fd]->buffers[ecasound_fiommap_buffermap[fd]->locked_buffer]);
}

long int ecasound_fiommap_active_buffersize(int fd) {
  return(ecasound_fiommap_buffermap[fd]->mmap_length[ecasound_fiommap_buffermap[fd]->locked_buffer]);
}

long int ecasound_fiommap_maximum_buffersize(void) { 
  return(ecasound_fiommap_buffersize);
}

void ecasound_fiommap_exec_thread(void) {
  int retcode = pthread_create(&ecasound_fiommap_thread, NULL, ecasound_fiommap_process, NULL);
  if (retcode != 0)
    throw(new ECA_ERROR("ECA-FILEIO-MMAP-FTHREAD", "unable to create thread for mmap()'ed file-I/O"));

  if (sched_getscheduler(0) == SCHED_FIFO) {
    struct sched_param sparam;
    sparam.sched_priority = 10;
    if (pthread_setschedparam(ecasound_fiommap_thread, SCHED_FIFO, &sparam) != 0)
      ecadebug->msg("(eca-fileio-mmap-fthread) Unable to change scheduling policy!");
    else 
      ecadebug->msg("(eca-fileio-mmap-fthread) Using realtime-scheduling (SCHED_FIFO/10).");
  }
}

void *ecasound_fiommap_process(void *) { 
  //  cerr << "(eca-fileio-mmap-fthread) process()\n";
  vector<ECASOUND_FIOMMAP_BUFFER*>::const_iterator p;
  vector<unsigned char*>::size_type q;

  //  if (setpriority(PRIO_PROCESS, 0, -10) == -1)
  //  mlockall(MCL_CURRENT | MCL_FUTURE);

  while(true) {
    pthread_cleanup_push(ecasound_fiommap_clean, (void *) &ecasound_fiommap_lock);
    pthread_mutex_lock(&ecasound_fiommap_lock);
    pthread_cond_wait(&ecasound_fiommap_cond, &ecasound_fiommap_lock);
    p = ecasound_fiommap_buffer.begin();
    while(p != ecasound_fiommap_buffer.end()) {
      q = 0;
      while(q != (*p)->buffers.size()) {
	if ((*p)->mmap_low != -1) {
	  if ((*p)->locked_buffer != q) {
	    if ((*p)->ready_for_fill[q] == false) {
	      ++q;
	      continue;
	    }
	    
	    (*p)->mmap_low += ecasound_fiommap_buffersize;
	    (*p)->mmap_high += ecasound_fiommap_buffersize;

	    munlock((*p)->buffers[q], (*p)->mmap_length[q]);
	    munmap((*p)->buffers[q], (*p)->mmap_length[q]);

	    if ((*p)->mmap_low > (*p)->file_length) {
	      (*p)->mmap_length[q] = 0;
	      (*p)->ready_for_fill[q] = false;
	      break;
	    }

	    if ((*p)->mmap_high > (*p)->file_length)
	      (*p)->mmap_length[q] = (*p)->file_length - (*p)->mmap_low;
	    else 
	      (*p)->mmap_length[q] =  ecasound_fiommap_buffersize;

	    //	    cerr << "(eca-fileio-mmap-fthread) mapping[s] (processs) from " << (*p)->mmap_low;
	    //	    cerr << " to " << (*p)->mmap_low + (*p)->mmap_length[q] << " (fd:" << (*p)->fd;
	    //	    cerr << ", buf:" << q << ").\n";

	    (*p)->buffers[q] = (unsigned char*)mmap(0,
						    (*p)->mmap_length[q],
						    PROT_READ,
						    MAP_SHARED,
						    (*p)->fd,
						    (*p)->mmap_low);
	    mlock((*p)->buffers[q], (*p)->mmap_length[q]);
	    //	    cerr << "(eca-fileio-mmap-fthread) mapping[f] (processs) from " << (*p)->mmap_low;
	    //	    cerr	 << " to " << (*p)->mmap_high << " (fd:" << (*p)->fd;
	    //	    cerr     << ", buf:" << q << ").\n";

	    (*p)->ready_for_fill[q] = false;
	  }
	}
	++q;
      }
      ++p;
    }
    pthread_cond_signal(&ecasound_fiommap_cond);
    pthread_mutex_unlock(&ecasound_fiommap_lock);
    pthread_cleanup_pop(0);
    pthread_testcancel();
  }
}

void ecasound_fiommap_clean(void*) {
  pthread_mutex_unlock(&ecasound_fiommap_lock);
}
