// ------------------------------------------------------------------------
// eca-mthreaded-processor.cpp: External functions for multithreaded 
//                              processing.
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include "eca-main.h"
#include "audioio.h"
#include "samplebuffer.h"
#include "eca-chain.h"
#include "eca-session.h"
#include "eca-debug.h"
#include "eca-mthreaded-processor.h"

void *mthread_process_chains(void* params) {
  ::pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); // other threads can stop this one
  ::pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  ECA_PROCESSOR* ecamain = static_cast<ECA_PROCESSOR*>(params);

  ecadebug->control_flow("Submix-thread ready");
  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-main) Submix-pid: " + kvu_numtostr(getpid()));

  vector<CHAIN*>* chains = ecamain->chains;
  vector<AUDIO_IO*>* outputs = ecamain->outputs;
  SAMPLE_BUFFER* mixslot = &(ecamain->mixslot);

  vector<bool> chain_locked (ecamain->output_count);
  while(true) {
    for(int n = 0; n != ecamain->chain_count;) {
      ::pthread_mutex_lock(ecamain->chain_muts[n]);
      chain_locked[n] = true;
      while(ecamain->chain_ready_for_submix[n] == false) {
	::pthread_cond_signal(ecamain->chain_conds[n]);
	::pthread_cond_wait(ecamain->chain_conds[n], ecamain->chain_muts[n]);
      }
      (*chains)[n]->process();
      ++n;
    }

    for(int audioslot_sizet = 0; audioslot_sizet < ecamain->output_count; audioslot_sizet++) {
      mixslot->number_of_channels((*outputs)[audioslot_sizet]->channels());
      int count = 0;
      for(int n = 0; n != ecamain->chain_count; n++) {
	// --
	// if chain is already released, skip
	// --
	if (chain_locked[n] == false) continue; 
	
	if ((*chains)[n]->output_id == 0) {
	  // --
	  // skip, if chain is not connected to any output or is
	  // disabled
	  // --
	  ecamain->chain_ready_for_submix[n] = false;
	  chain_locked[n] = false;
	  ::pthread_cond_signal(ecamain->chain_conds[n]);
	  ::pthread_mutex_unlock(ecamain->chain_muts[n]);
	  continue;
	}

	if ((*chains)[n]->output_id == (*outputs)[audioslot_sizet]) {
	  // --
	  // output is connected to this chain
	  // --
	  if (ecamain->output_chain_count[audioslot_sizet] == 1) {
	    // --
	    // there's only one output connected to this chain,
	    // so we don't need to mix anything
	    // --
	    (*outputs)[audioslot_sizet]->write_buffer(&(ecamain->cslots[n]));
	  }
	  else {
	    ++count;
	    if (count == 1) {
	      // -- 
	      // this is the first chain connected to this output
	      // --
	      mixslot->copy(ecamain->cslots[n]);
	      mixslot->divide_by(ecamain->output_chain_count[audioslot_sizet]);
	    }
	    else {
	      mixslot->add_with_weight(ecamain->cslots[n], ecamain->output_chain_count[audioslot_sizet]);
	      if (count == ecamain->output_chain_count[audioslot_sizet]) {
		(*outputs)[audioslot_sizet]->write_buffer(mixslot);
	      }
	    }
	  }
	  ecamain->chain_ready_for_submix[n] = false;
	  chain_locked[n] = false;
	  ::pthread_cond_signal(ecamain->chain_conds[n]);
	  ::pthread_mutex_unlock(ecamain->chain_muts[n]);
	}
      }
    }
    ecamain->trigger_outputs();
    ::pthread_testcancel();
  }
  cerr << "(eca-main/submix) You should never see this message!\n";
}
