// ------------------------------------------------------------------------
// locks.cpp: Various pthread and locking related helper functions.
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <sys/errno.h>
#include "locks.h"

/**
 * A variant of the standard pthread_mutex_lock. This routine is
 * originally from Quasimodo CVS-tree, from the libpbd library, Written 
 * by Paul Barton-Davis. This version of the routine adds a second
 * argument, 'spinlimit' which specifies the number of loops spent
 * spinning, before blocking with the standard pthread_mutex_lock() call.
 */
int pthread_mutex_spinlock (pthread_mutex_t *mp, long int spinlimit) {
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
