// ------------------------------------------------------------------------
// kvu_rtcaps.h: Routines for checking realtime-related capabilities.
// Copyright (C) 2001,2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <config.h>
#endif

#include <iostream>
#include <unistd.h> /* getpid(), _POSIX_MEMLOCK */

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h> /* mlockall(), munlockall() */
#endif

#include "kvu_rtcaps.h"

static bool kvu_check_for_sched_sub(int policy);

bool kvu_check_for_sched_sub(int policy)
{
  bool result = false;
#ifdef HAVE_SCHED_GETPARAM
  int curpid = getpid();

  /* store old scheduling params */
  struct sched_param prev_sparam;
  int ret = sched_getparam(curpid, &prev_sparam);
  if (ret == 0) {
    int prev_prio = prev_sparam.sched_priority;
    int prev_policy = sched_getscheduler(0);
    if (prev_policy >= 0) {
      /* get maximum priority for the tested policy */
      int max_prio = sched_get_priority_max(policy);
      if (max_prio >= 0) {
	struct sched_param sparam;
	sparam.sched_priority = max_prio;
	/* try to change scheduling according the new params */
        int ret = sched_setscheduler(curpid, policy, &sparam);
	if (ret == 0) {
	  /* test succeeded, restore old settings */
	  result = true;
	  sparam.sched_priority = prev_prio;
	  sched_setscheduler(curpid, prev_policy, &sparam);
	}
      }
    }
  } 
#else /* HAVE_SCHED_GETPARAM */
  std::cerr << "(libkvutils) kvu_rtcaps: warning! sched_getparam() not supported" << std::endl;
#endif
  return(result);
}

/**
 * Checks whether current process has privileges
 * to set scheduler to SCHED_FIFO.
 */
bool kvu_check_for_sched_fifo(void) {
#ifdef HAVE_SCHED_H
  return(kvu_check_for_sched_sub(SCHED_FIFO));
#else
  std::cerr << "(libkvutils) kvu_rtcaps: warning! sched.h not available" << std::endl;
#endif
}

/**
 * Checks whether current process has privileges
 * to set scheduler to SCHED_RR.
 */
bool kvu_check_for_sched_rr(void) {
#ifdef HAVE_SCHED_H
  return(kvu_check_for_sched_sub(SCHED_RR));
#else
  std::cerr << "(libkvutils) kvu_rtcaps: warning! sched.h not available" << std::endl;
#endif
}


/**
 * Checks whether mlockall() call is available 
 * and whether current process has privileges
 * to execute it.
 *
 * Note! Function issues an munlockall() call,
 *       which will free all previously locked
 *       memory areas for this process.
 */
bool kvu_check_for_mlockall(void) {
  bool result = false;
#if defined(_POSIX_MEMLOCK) && defined(HAVE_MLOCKALL) && defined(HAVE_MUNLOCKALL) /* unistd.h */
  int ret = mlockall(MCL_CURRENT);
  if (ret == 0) {
    result = true;
    munlockall();
  }
#else
  std::cerr << "(libkvutils) kvu_rtcaps: warning! POSIX_MEMLOCK not supported" << std::endl;
#endif
  return(result);
}
