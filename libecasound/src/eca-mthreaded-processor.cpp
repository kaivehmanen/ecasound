// ------------------------------------------------------------------------
// eca-mthreaded-processor.cpp: External functions for multithreaded 
//                              processing.
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
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sched.h>

#include <kvutils.h>

#include "audioio.h"
#include "samplebuffer.h"
#include "eca-chain.h"
#include "eca-session.h"
#include "eca-debug.h"
#include "eca-mthreaded-processor.h"

void *mthread_process_chains(void* params) {
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // other threads can stop this one
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  ECA_SESSION* ecaparams = (ECA_SESSION*)params;
  
  ecadebug->control_flow("Submix-thread ready");
  ecadebug->msg(1,"(eca-main) Submix-pid: " + kvu_numtostr(getpid()));

  // ---
  // Handle general parameters.
  // ---
//    getpriority(PRIO_PROCESS, 0);
//    if (setpriority(PRIO_PROCESS, 0, -10) == -1)
//    struct sched_param sparam;
//    sparam.sched_priority = 10;
//    if (sched_setscheduler(0, SCHED_FIFO, &sparam) == -1)
//      ecadebug->msg("(eca-mthreaded-processor) Unable to change priority.");
  
//    MESSAGE_ITEM mtemp;
//    mtemp << "(eca-mthreaded-processor) Raised submix-thread priority to SCHED_FIFO/10";
//    mtemp << ", PID: " << (int)getpid() << ".";
//    ecadebug->msg(1, mtemp.to_string());
 

  vector<AUDIO_IO*>* outputs = ecaparams->outputs;
  vector<CHAIN*>* chains = ecaparams->chains;

  int chain_count = static_cast<int>(chains->size());
  int output_count = static_cast<int>(outputs->size());

  int max_channels = 0;
  vector<int> output_chain_count (output_count);
  for(int adev_sizet = 0; adev_sizet < output_count; adev_sizet++) {
    output_chain_count[adev_sizet] =
      ecaparams->number_of_connected_chains_to_output((*outputs)[adev_sizet]);

    if ((*outputs)[adev_sizet]->channels() > max_channels)
      max_channels = (*outputs)[adev_sizet]->channels();
  }
  SAMPLE_BUFFER mixslot (ecaparams->connected_chainsetup->buffersize(), max_channels);

  vector<bool> chain_locked (output_count);

  while(true) {
    for(int n = 0; n != chain_count;) {
      pthread_mutex_lock(ecaparams->chain_muts[n]);
      chain_locked[n] = true;
      while(ecaparams->chain_ready_for_submix[n] == false) {
	pthread_cond_signal(ecaparams->chain_conds[n]);
	pthread_cond_wait(ecaparams->chain_conds[n],
			  ecaparams->chain_muts[n]);
      }

      (*chains)[n]->process();
    
      ++n;
    }

    for(int audioslot_sizet = 0; audioslot_sizet < output_count; audioslot_sizet++) {
      mixslot.make_silent();
      int count = 0;

      for(int n = 0; n != chain_count; n++) {
	// --
	// if chain is already released, skip
	// --
	if (chain_locked[n] == false) continue; 
	
	if ((*chains)[n]->output_id == 0) {
	  // --
	  // skip, if chain is not connected to any output or is
	  // disabled
	  // --
	  ecaparams->chain_ready_for_submix[n] = false;
	  chain_locked[n] = false;
	  pthread_cond_signal(ecaparams->chain_conds[n]);
	  pthread_mutex_unlock(ecaparams->chain_muts[n]);
	  continue;
	}

	if ((*chains)[n]->output_id == (*outputs)[audioslot_sizet]) {
	  // --
	  // output is connected to this chain
	  // --
	  if (output_chain_count[audioslot_sizet] == 1) {
	    // --
	    // there's only one output connected to this chain,
	    // so we don't need to mix anything
	    // --
	    (*outputs)[audioslot_sizet]->write_buffer(&(*chains)[n]->audioslot);
	  }
	  else {
	    ++count;
	    if (count == 1) {
	      // -- 
	      // this is the first chain connected to this output
	      // --
	      mixslot.copy((*chains)[n]->audioslot); 
	      mixslot.divide_by(output_chain_count[audioslot_sizet]);
	    }
	    else {
	      mixslot.add_with_weight((*chains)[n]->audioslot,
				      output_chain_count[audioslot_sizet]);
	      if (count == output_chain_count[audioslot_sizet]) {
		(*outputs)[audioslot_sizet]->write_buffer(&mixslot);
	      }
	    }
	  }
	  ecaparams->chain_ready_for_submix[n] = false;
	  chain_locked[n] = false;
	  //	  cerr << "rel:" << n << ">";
	  pthread_cond_signal(ecaparams->chain_conds[n]);
	  pthread_mutex_unlock(ecaparams->chain_muts[n]);
	}
      }
    }
    pthread_testcancel();
  }
  cerr << "(eca-main/submix) You should never see this message!\n";
}
