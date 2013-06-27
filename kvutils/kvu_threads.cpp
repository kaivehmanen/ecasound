// ------------------------------------------------------------------------
// kvu_threads.cpp: Various pthread related helper functions.
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

#include <time.h>     /* clock_gettime() */
#include <sys/time.h> /* gettimeofday() */
#include <errno.h> /* ETIMEDOUT */

#include "kvu_threads.h"

using std::string;

/**
 * A variant of the standard pthread_mutex_lock. This routine is
 * originally from Quasimodo CVS-tree, from the libpbd library, Written 
 * by Paul Davis. This version of the routine adds a second
 * argument, 'spinlimit' which specifies the number of loops spent
 * spinning, before blocking with the standard pthread_mutex_lock() call.
 */
int kvu_pthread_mutex_spinlock (pthread_mutex_t *mp, long int spinlimit)
{
  int err = EBUSY;
  unsigned int i;
  
  i = spinlimit;
  
  while (i > 0 && ((err = pthread_mutex_trylock (mp)) == EBUSY))
    i--;
  
  if (err == EBUSY) {
    err = pthread_mutex_lock (mp);
  }
  
  return err;
}

/**
 * Waits for condition to occur.
 *
 * Note: this model of condition waiting is inherently racy and
 *       must be only used in specific cases (not a general solution).
 *       The main problem is that the condition is not checked with
 *       mutex held before going into pthread_cond_timedwait(), so a
 *       condition change may be missed.
 *
 * @return 0 on success, 
 *         -ETIMEDOUT if timeout occured, 
 *         other nonzero value on other errors
 */
int kvu_pthread_timed_wait(pthread_mutex_t* mutex, pthread_cond_t* cond, long int seconds)
{
   struct timeval now;
   gettimeofday(&now, 0);
   struct timespec sleepcount;
   sleepcount.tv_sec = now.tv_sec + seconds;
   sleepcount.tv_nsec = now.tv_usec * 1000;
   int ret = 0;

   /* note: timing race possible here, if condition has already been
    *       signaled at this point, thus the sleepcount is mandatory */
    
   pthread_mutex_lock(mutex);
   ret = pthread_cond_timedwait(cond, 
				mutex,
				&sleepcount);
   pthread_mutex_unlock(mutex);

   return(ret);
}

/**
 * Returns a string explaning the error code 
 * returned by kvu_pthread_timed_wait().
 */
string kvu_pthread_timed_wait_result(int result, const string& prefix)
{
  if (result != 0) {
    if (result == -ETIMEDOUT)
      return(prefix + " failed; timeout");
    else
      return(prefix + " failed");
  }
  return(prefix + " ok");
}

/**
 * Fills absolute timeout struct out for pthread_cond_timedwait() that
 * results in 'seconds' of relative timeout.
 *
 * @arg seconds timeout in seconds (must be positive)
 * @out out pointer to output timeout struct
 * @out mononic true if CLOCK_MONOTONIC should be used (condition
 *      attribute set to MONOTONIC using pthread_condattr_setclock(MONOTONIC);
 *      otherwise using CLOCK_REALTIME (system time)
 */
int kvu_pthread_cond_timeout(int seconds, struct timespec *out, bool monotonic = false)
{
  // FIXME: needs to be implemented using MONOTONIC clock_gettime() 
  //        and setting  on the clock
  int res = 0;
  if (monotonic) {
#if defined CLOCK_MONOTONIC
    res = clock_gettime(CLOCK_MONOTONIC, out);
    out->tv_sec += seconds;
#else
    res = -1;
#endif
  }
  else {
#if defined(CLOCK_REALTIME)
    res = clock_gettime(CLOCK_REALTIME, out);
    out->tv_sec += seconds;
#else
    struct timeval tv;
    res = gettimeofday(&tv, NULL);
    out->tv_sec = tv.tv_sec + seconds;
    out->tv_nsec = tv.tv_usec * 1000;
#endif
  }

  return res;
}
