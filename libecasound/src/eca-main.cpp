// ------------------------------------------------------------------------
// eca-main.cpp: Main processing engine
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
#include <vector>
#include <map>
#include <ctime>
#include <cmath>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <utility>

#include <kvutils.h>

#include "samplebuffer.h"
#include "eca-chain.h"
#include "eca-session.h"
#include "eca-chainop.h"
#include "audioio.h"
#include "eca-error.h"
#include "eca-debug.h"
#include "eca-mthreaded-processor.h"
#include "eca-main.h"

VALUE_QUEUE ecasound_queue;

ECA_PROCESSOR::ECA_PROCESSOR(void) :
  mixslot(0, SAMPLE_BUFFER::channel_count_default)
{
  eparams = 0;
  active_chain_index = 0;
}

ECA_PROCESSOR::ECA_PROCESSOR(ECA_SESSION* params) :
  mixslot(params->connected_chainsetup->buffersize(), SAMPLE_BUFFER::channel_count_default),
  buffersize_rep(params->connected_chainsetup->buffersize()),
  eparams(params)
{
  init();
  active_chain_index = 0;
}

void ECA_PROCESSOR::init(ECA_SESSION* params) { 
  eparams = params;
  active_chain_index = 0;
  mixslot.length_in_samples(eparams->connected_chainsetup->buffersize());
  mixslot.number_of_channels(SAMPLE_BUFFER::channel_count_default);
  buffersize_rep = eparams->connected_chainsetup->buffersize();
  init();
}

ECA_PROCESSOR::~ECA_PROCESSOR(void) {
  ecadebug->msg(1,"ECA_PROCESSOR destructor!");

  stop();
  if (eparams != 0) eparams->status(ep_status_notready);

  pthread_cancel(chain_thread);
  pthread_join(chain_thread,NULL);

  ecadebug->control_flow("Engine/Exiting");
}

void ECA_PROCESSOR::init(void) {
  assert(eparams != 0);

  ecadebug->control_flow("Engine/Initializing");

  eparams->status(ep_status_stopped);
  init_connection_to_chainsetup();
  init_status_variables();
  init_mix_method();

  // ---
  // Handle priority
  // ---
  if (csetup->raised_priority() == true) {
    struct sched_param sparam;
    sparam.sched_priority = 10;
    if (sched_setscheduler(0, SCHED_FIFO, &sparam) == -1)
      ecadebug->msg("(eca-main) Unable to change scheduling policy!");
    else 
      ecadebug->msg("(eca-main) Using realtime-scheduling (SCHED_FIFO/10).");
  }
}

void ECA_PROCESSOR::init_connection_to_chainsetup(void) throw(ECA_ERROR*) {
  csetup = eparams->connected_chainsetup;

  if (csetup == 0 ) {
    throw(new ECA_ERROR("ECA_PROCESSOR", "Engine startup aborted, no chainsetup connected!"));
  }

  inputs = eparams->inputs;
  outputs = eparams->outputs;
  chains = eparams->chains;

  if (inputs == 0 ||
      inputs->size() == 0 ||
      outputs  == 0 ||
      outputs->size() == 0 ||
      chains == 0 ||
      chains->size() == 0) {
    throw(new ECA_ERROR("ECA_PROCESSOR", "Engine startup aborted, session in corrupted state!"));
  }

  input_count = static_cast<int>(inputs->size());
  output_count = static_cast<int>(outputs->size());
  chain_count = static_cast<int>(chains->size());

  input_start_pos.resize(input_count);
  input_chain_count.resize(input_count);
  for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
    input_start_pos[adev_sizet] =
      (*inputs)[adev_sizet]->position_in_samples();
    input_chain_count[adev_sizet] =
      eparams->number_of_connected_chains_to_input((*inputs)[adev_sizet]);
    ecadebug->msg(4, "Input \"" + (*inputs)[adev_sizet]->label() +
                     ": start position " +
		     kvu_numtostr(input_start_pos[adev_sizet]) +
		     ", number of connected chain " +
		     kvu_numtostr(input_chain_count[adev_sizet]) +
		     " .\n");
    (*inputs)[adev_sizet]->buffersize(buffersize_rep, SAMPLE_BUFFER::sample_rate);
  }

  output_start_pos.resize(output_count);
  output_chain_count.resize(output_count);
  for(int adev_sizet = 0; adev_sizet < output_count; adev_sizet++) {
    output_start_pos[adev_sizet] =
      (*outputs)[adev_sizet]->position_in_samples();
    output_chain_count[adev_sizet] =
      eparams->number_of_connected_chains_to_output((*outputs)[adev_sizet]);
    ecadebug->msg(4, "Output \"" + (*outputs)[adev_sizet]->label() +
                     ": start position " +
		     kvu_numtostr(output_start_pos[adev_sizet]) +
		     ", number of connected chain " +
		     kvu_numtostr(output_chain_count[adev_sizet]) +
		     " .\n");
    (*outputs)[adev_sizet]->buffersize(buffersize_rep, SAMPLE_BUFFER::sample_rate);
  }

  eparams->chain_ready_for_submix.resize(chain_count);
  eparams->chain_muts.resize(chain_count);
  eparams->chain_conds.resize(chain_count);

  // ---
  // Whether setup contains realtime input/output devices?
  // ---    
  
  rt_infiles = rt_outfiles = false;

  nonrt_infiles = false;
  nonrt_outfiles = false;
    
  for (audio_ci adev_citer = inputs->begin(); adev_citer != inputs->end(); adev_citer++) {
    if ((*adev_citer)->is_realtime()) rt_infiles = true;
    else nonrt_infiles = true;
  }

  for (audio_ci adev_citer = outputs->begin(); adev_citer != outputs->end(); adev_citer++) {
    if ((*adev_citer)->is_realtime()) rt_outfiles = true;
    else nonrt_outfiles = true;
  }
}

void ECA_PROCESSOR::init_status_variables(void) {
  end_request = false;
  finished_result = false;
  trigger_outputs_request = false;

  // ---
  // Are we doing multitrack-recording?
  // ---    

  if (rt_infiles == true && 
      rt_outfiles == true &&
      chain_count > 1 && 
      eparams->iactive &&
      nonrt_infiles == true &&
      nonrt_outfiles == true)  {
    ecadebug->msg("(eca-main) Multitrack-mode enabled. Changed mixmode to \"normal iactive\"");
    eparams->multitrack_mode = true;
    csetup->set_mixmode(ECA_CHAINSETUP::ep_mm_normal);
  }

  for(int n = 0; n < chain_count; n++) {
    eparams->chain_ready_for_submix[n] = false;

    pthread_mutex_t* mut = new pthread_mutex_t;
    pthread_mutex_init(mut, NULL);
    eparams->chain_muts[n] = mut;

    pthread_cond_t* cond = new pthread_cond_t;
    pthread_cond_init(cond, NULL);
    eparams->chain_conds[n] = cond;
  }
}


void ECA_PROCESSOR::init_mix_method(void) { 
  if (csetup->mixmode() == ECA_CHAINSETUP::ep_mm_auto) {
    if (chain_count == 1 &&
	input_count == 1 &&
	output_count == 1)
      csetup->set_mixmode(ECA_CHAINSETUP::ep_mm_simple);
    else if (csetup->buffersize() > 1024 &&
	     chain_count > 1)
      csetup->set_mixmode(ECA_CHAINSETUP::ep_mm_mthreaded);
    else 
      csetup->set_mixmode(ECA_CHAINSETUP::ep_mm_normal);
  }
}

void ECA_PROCESSOR::exec(void) {
  switch(csetup->mixmode()) {
  case ECA_CHAINSETUP::ep_mm_simple:
    {
      if (eparams->iactive) exec_simple_iactive();
      else exec_simple_passive();
      break;
    }
  case ECA_CHAINSETUP::ep_mm_normal:
    {
      if (eparams->iactive) exec_normal_iactive();
      else exec_normal_passive();
      break;
    }
  case ECA_CHAINSETUP::ep_mm_mthreaded:
    {
      if (eparams->iactive) exec_mthreaded_iactive();
      else exec_mthreaded_passive();
      break;
    }
  default: 
    {
      exec_normal_iactive();
    }
  }
}

void ECA_PROCESSOR::conditional_start(void) {
  if (was_running == true) start();
}

void ECA_PROCESSOR::conditional_stop(void) {
  if (eparams->status() == ep_status_running) {
    was_running = true;
    stop();
  }
  else was_running = false;
}

void ECA_PROCESSOR::exec_normal_iactive(void) {
  // ---
  // Enable devices (non-multitrack-mode)
  // ---

  stop();
  
  // ---
  // The main processing loop.
  // ---
  
  ecadebug->control_flow("Engine/Mixmode \"normal iactive\" selected");
  struct timespec sleepcount;
  sleepcount.tv_sec = 1;
  sleepcount.tv_nsec = 0;
  
  while (true) {
    if (finished() == true) stop();
    interpret_queue();
    if (end_request) break;
    if (eparams->status() != ep_status_running) {
      //      sched_yield();
      nanosleep(&sleepcount, NULL);
      continue;
    }

    prehandle_control_position();
    // ---
    // Mix from inputs to chains
    // ---
    inputs_to_chains();

    // ---
    // Chainoperator processing phase.
    // ---

    chain_i chain_iter = chains->begin();
    while(chain_iter != chains->end()) {
      (*chain_iter)->process();
      ++chain_iter;
    }

    // ---
    // Mix from chains to outputs.
    // ---
    mix_to_outputs();
    trigger_outputs();
    posthandle_control_position();
  }
}

void ECA_PROCESSOR::exec_normal_passive(void) {
  // ---
  // Enable devices.
  // ---
  start();
  
  ecadebug->control_flow("Engine/Mixmode \"normal passive\" selected");
  while (!finished() && !end_request) {
    inputs_to_chains();

    chain_i chain_iter = chains->begin();
    while(chain_iter != chains->end()) {
      (*chain_iter)->process();
      ++chain_iter;
    }

    mix_to_outputs();
    trigger_outputs();
  }
}


void ECA_PROCESSOR::exec_simple_iactive(void) {
  // ---
  // Enable devices (non-multitrack-mode)
  // ---
  stop();  
  struct timespec sleepcount;
  sleepcount.tv_sec = 1;
  sleepcount.tv_nsec = 0;

  ecadebug->control_flow("Engine/Mixmode \"simple iactive\" selected");
  while (true) {
    if (finished() == true) {
      stop();
    }
    interpret_queue();
    if (end_request) break;
    if (eparams->status() != ep_status_running) {
      //      sched_yield();
      nanosleep(&sleepcount, NULL);
      continue;
    }

    prehandle_control_position();
    (*inputs)[0]->read_buffer(&((*chains)[0]->audioslot));
    (*chains)[0]->process();
    (*outputs)[0]->write_buffer(&((*chains)[0]->audioslot));
    trigger_outputs();
    posthandle_control_position();
  }
}

void ECA_PROCESSOR::exec_simple_passive(void) {
  // ---
  // Enable devices.
  // ---
  start();
  ecadebug->control_flow("Engine/Mixmode \"simple passive\" selected");
  while (!finished() && !end_request) {
    prehandle_control_position();
    (*inputs)[0]->read_buffer(&((*chains)[0]->audioslot));
    (*chains)[0]->process();
    (*outputs)[0]->write_buffer(&((*chains)[0]->audioslot));
    trigger_outputs();
    posthandle_control_position();
  }
}

void ECA_PROCESSOR::set_position(double seconds) {
  conditional_stop();

  csetup->set_position(seconds * SAMPLE_BUFFER::sample_rate);

  for (int adev_sizet = 0; adev_sizet != input_count; adev_sizet++) {
    (*inputs)[adev_sizet]->seek_position_in_seconds(seconds);
  }

  for (int adev_sizet = 0; adev_sizet != output_count; adev_sizet++) {
    (*outputs)[adev_sizet]->seek_position_in_seconds(seconds);
  }

  conditional_start();
}

void ECA_PROCESSOR::set_position_chain(double seconds) {
  conditional_stop();

  (*chains)[active_chain_index]->input_id->seek_position_in_seconds(seconds);
  (*chains)[active_chain_index]->output_id->seek_position_in_seconds(seconds);

  conditional_start();
}

void ECA_PROCESSOR::change_position(double seconds) {
  conditional_stop();

  csetup->change_position(seconds);

  for (int adev_sizet = 0; adev_sizet != input_count; adev_sizet++) {
    (*inputs)[adev_sizet]->seek_position_in_seconds((*inputs)[adev_sizet]->position_in_seconds()
                                           + seconds);
  }

  for (int adev_sizet = 0; adev_sizet != output_count; adev_sizet++) {
    (*outputs)[adev_sizet]->seek_position_in_seconds((*outputs)[adev_sizet]->position_in_seconds()
                                           + seconds);
  }

  conditional_start();
}

void ECA_PROCESSOR::change_position_chain(double seconds) {
  conditional_stop();

  (*chains)[active_chain_index]->input_id->seek_position_in_seconds(
								    (*chains)[active_chain_index]->input_id->position_in_seconds() + seconds);
  (*chains)[active_chain_index]->output_id->seek_position_in_seconds(
								     (*chains)[active_chain_index]->output_id->position_in_seconds() + seconds);
  
  conditional_start();
}

double ECA_PROCESSOR::current_position(void) const {
  return(eparams->master_input()->position_in_seconds_exact());
}

double ECA_PROCESSOR::current_position_chain(void) const {
  return((*chains)[active_chain_index]->input_id->position_in_seconds_exact());
}

void ECA_PROCESSOR::prehandle_control_position(void) {
  csetup->change_position(buffersize_rep);
  if (csetup->is_over() == true) {
    int buffer_remain = csetup->position_in_samples() -
                        csetup->length_in_samples();
    for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
      (*inputs)[adev_sizet]->buffersize(buffer_remain, SAMPLE_BUFFER::sample_rate);
    }
  }
}

void ECA_PROCESSOR::posthandle_control_position(void) {
  if (csetup->is_over() == true) {
    if (csetup->looping_enabled() == false) {
      end_request = true;
    }
    else {
      rewind_to_start_position();
      csetup->set_position(0);
      for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
	(*inputs)[adev_sizet]->buffersize(buffersize_rep, SAMPLE_BUFFER::sample_rate);
      }
    }
  }
}

void ECA_PROCESSOR::rewind_to_start_position(void) {
  conditional_stop();

  for (int adev_sizet = 0; adev_sizet != input_count; adev_sizet++) {
    (*inputs)[adev_sizet]->seek_position_in_samples(input_start_pos[adev_sizet]);
  }

  for (int adev_sizet = 0; adev_sizet != output_count; adev_sizet++) {
    (*outputs)[adev_sizet]->seek_position_in_samples(output_start_pos[adev_sizet]);
  }

  conditional_start();
}


void ECA_PROCESSOR::stop(void) { 
  if (eparams->status() != ep_status_running) return;
  ecadebug->msg(1, "(eca-main) Stop");

  for (int adev_sizet = 0; adev_sizet != input_count; adev_sizet++)
    (*inputs)[adev_sizet]->stop();

  for (int adev_sizet = 0; adev_sizet != output_count; adev_sizet++) 
    (*outputs)[adev_sizet]->stop();

  eparams->status(ep_status_stopped);
}

void ECA_PROCESSOR::start(void) {
  if (eparams->status() == ep_status_running) return;
  ecadebug->msg(1, "(eca-main) Start");

  if (eparams->multitrack_mode == true) {
    multitrack_sync();
    for (int adev_sizet = 0; adev_sizet != output_count; adev_sizet++)
      (*outputs)[adev_sizet]->start();
    for (int adev_sizet = 0; adev_sizet != input_count; adev_sizet++)
      (*inputs)[adev_sizet]->start();
  }
  else {
    assert(csetup->mixmode() != ECA_CHAINSETUP::ep_mm_mthreaded);
    for (int adev_sizet = 0; adev_sizet != input_count; adev_sizet++)
      (*inputs)[adev_sizet]->start();
    trigger_outputs_request = true;
  }

  eparams->status(ep_status_running);
}

void ECA_PROCESSOR::trigger_outputs(void) {
  if (trigger_outputs_request == true) {
    trigger_outputs_request = false;
    for (int adev_sizet = 0; adev_sizet != output_count; adev_sizet++)
      (*outputs)[adev_sizet]->start();
  }
}

void ECA_PROCESSOR::multitrack_sync(void) {
  // ---
  // Read and mix inputs (skips realtime inputs)
  // ---

  for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
    if ((*inputs)[audioslot_sizet]->is_realtime()) continue;
    if (input_chain_count[audioslot_sizet] > 1) {
      (*inputs)[audioslot_sizet]->read_buffer(&mixslot);
    }
    for (int c = 0; c != chain_count; c++) {
      if ((*inputs)[audioslot_sizet] == (*chains)[c]->input_id) {
	if (input_chain_count[audioslot_sizet] == 1) {
	  (*inputs)[audioslot_sizet]->read_buffer(&(*chains)[c]->audioslot);
	  break;
	}
	else {
	  (*chains)[c]->audioslot.operator=(mixslot);
	}
      }
    }
  }

  // ---
  // Chainoperator processing phase
  // ---
  chain_i chain_iter = chains->begin();
  while(chain_iter != chains->end()) {
    (*chain_iter)->process();
    ++chain_iter;
  }

  // ---
  // Mix to outputs (skip outputs which are connected to realtime inputs)
  // ---
  for(int audioslot_sizet = 0; audioslot_sizet < output_count; audioslot_sizet++) {
    if (eparams->is_slave_output((*outputs)[audioslot_sizet]) == true) continue;
    mixslot.make_silent();
    int count = 0;
    
    for(int n = 0; n != chain_count; n++) {
      if ((*chains)[n]->output_id == 0) {
	continue;
      }

      if ((*chains)[n]->output_id == (*outputs)[audioslot_sizet]) {
	if (output_chain_count[audioslot_sizet] == 1) {
	  (*outputs)[audioslot_sizet]->write_buffer(&(*chains)[n]->audioslot);
	  break;
	}
	else {
	  ++count;
	  if (count == 1) {
	    mixslot = (*chains)[n]->audioslot; 
	  }
	  else {
	    mixslot.add_with_weight((*chains)[n]->audioslot,
				    output_chain_count[audioslot_sizet]);
	    
	    if (count == output_chain_count[audioslot_sizet]) {
	      (*outputs)[audioslot_sizet]->write_buffer(&mixslot);
	    }
	  }
	}
      }
    }
  }
}

void ECA_PROCESSOR::interpret_queue(void) {
  while(ecasound_queue.is_empty() == false) {
    ecadebug->msg(2,"(eca-main) ecasound_queue: cmds available");
    pair<int,double> item = ecasound_queue.front();
    switch(item.first) {
    // ---
    // Basic commands.
    // ---            
    case ep_exit:
      {
	while(ecasound_queue.is_empty() == false) ecasound_queue.pop_front();
	ecadebug->msg(2,"(eca-main) ecasound_queue: exit!");
	stop();
	end_request = true;
	return;
      }
    case ep_start: { start(); break; }
    case ep_stop: { stop(); break; }

    // ---
    // Section/chain (en/dis)abling commands.
    // ---
    case ep_c_select: {	active_chain_index = static_cast<size_t>(item.second); break; }
    case ep_c_mute: { chain_muting(); break; }
    case ep_c_bypass: { chain_processing(); break; }
    case ep_c_rewind: { change_position_chain(- item.second); break; }
    case ep_c_forward: { change_position_chain(item.second); break; }
    case ep_c_setpos: { set_position_chain(item.second); break; }

    // ---
    // Chain operators
    // ---
    case ep_cop_select: { active_chainop_index = static_cast<size_t>(item.second); break; }
    case ep_copp_select: { active_chainop_param_index = static_cast<size_t>(item.second); break; }
    case ep_copp_value: { 
      assert(chains != 0);
      if (active_chainop_index - 1 < (*chains)[active_chain_index]->chainops.size()) {
	(*chains)[active_chain_index]->select_chain_operator(active_chainop_index);
	(*chains)[active_chain_index]->set_parameter(active_chainop_param_index, item.second);
      }
      break;
    }

    // ---
    // Global position
    // ---
    case ep_rewind: { change_position(- item.second); break; }
    case ep_forward: { change_position(item.second); break; }
    case ep_setpos: { set_position(item.second); break; }
    }
    ecasound_queue.pop_front();
  }
}

bool ECA_PROCESSOR::finished(void) {
  finished_result = true;
  for (int a = 0; a != output_count; a++) {
    if ((*outputs)[a]->finished() == true) {
      ecadebug->msg(2, "(eca-main) Finished... outfile.");
      stop();
      eparams->status(ep_status_finished);
      return(true);
    }
  }

  for (int a = 0; a != input_count; a++) {
    if (!((*inputs)[a]->finished())) {
      finished_result = false;
      break;
    }
    else finished_result = true;
  }
  
  if (finished_result == true && eparams->iactive == false)
    ecadebug->msg(2, "(eca-main) Finished... through.");
  if (finished_result) {
    stop();
    eparams->status(ep_status_finished);
  }
  return(finished_result);
}

void ECA_PROCESSOR::inputs_to_chains(void) {
  for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
    if (input_chain_count[audioslot_sizet] > 1) {
      (*inputs)[audioslot_sizet]->read_buffer(&mixslot);
    }
    for (int c = 0; c != chain_count; c++) {
      if ((*chains)[c]->input_id == (*inputs)[audioslot_sizet]) {
	if (input_chain_count[audioslot_sizet] == 1) {
	  (*inputs)[audioslot_sizet]->read_buffer(&(*chains)[c]->audioslot);
	  break;
	}
	else {
	  (*chains)[c]->audioslot.operator=(mixslot);
	}
      }
    }
  }
}

void ECA_PROCESSOR::mix_to_chains(void) {
  for (int c = 0; c != chain_count; c++) {
    for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
      if ((*chains)[c]->input_id ==  (*inputs)[audioslot_sizet]) {
	(*chains)[c]->audioslot.operator=(eparams->inslots[audioslot_sizet]);
	// --- for debugging signal flow
	//	cerr << "[1]Mixing from sbuf eparams->inslots[]" << " nro " << eparams->inslots[audioslot_sizet]->nro <<  " " << eparams->inslots[audioslot_sizet]->average_volume() <<".\n";
	//	cerr << "[1]Mixing to sbuf audioslot[c]" << " nro " << (*chains)[c]->audioslot->nro << " " << (*chains)[c]->audioslot->average_volume() << ".\n";
	// -----------------------------
      }
    }
  }
}

void ECA_PROCESSOR::mix_to_outputs(void) {
  for(int audioslot_sizet = 0; audioslot_sizet < output_count; audioslot_sizet++) {
    mixslot.number_of_channels((*outputs)[audioslot_sizet]->channels());
    int count = 0;
    
    for(int n = 0; n != chain_count; n++) {
      // --
      // if chain is already released, skip
      // --
      if ((*chains)[n]->output_id == 0) {
	// --
	// skip, if chain is not connected
	// --
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
	  break;
	}
	else {
	  ++count;
	  if (count == 1) {
	    // -- 
	    // this is the first output connected to this chain
	    // --
	    mixslot = (*chains)[n]->audioslot;
	    mixslot.divide_by(output_chain_count[audioslot_sizet]);
	  }
	  else {
	    mixslot.add_with_weight((*chains)[n]->audioslot,
				    output_chain_count[audioslot_sizet]);
	  }
	  
	  if (count == output_chain_count[audioslot_sizet]) {
	    (*outputs)[audioslot_sizet]->write_buffer(&mixslot);
	  }
	}
      }
    }
  } 
}

void ECA_PROCESSOR::chain_muting(void) {
  if ((*chains)[active_chain_index]->is_muted()) 
    (*chains)[active_chain_index]->toggle_muting(false);
  else
    (*chains)[active_chain_index]->toggle_muting(true);
}

void ECA_PROCESSOR::chain_processing(void) {
  if ((*chains)[active_chain_index]->is_processing()) 
    (*chains)[active_chain_index]->toggle_processing(false);
  else
    (*chains)[active_chain_index]->toggle_processing(true);
}

void ECA_PROCESSOR::exec_mthreaded_iactive(void) throw(ECA_ERROR*) {

  ecadebug->control_flow("Engine/Mixmode \"multithreaded interactive\" selected");
  stop();
  struct timespec sleepcount;
  sleepcount.tv_sec = 1;
  sleepcount.tv_nsec = 0;

  int submix_pid = pthread_create(&chain_thread, NULL, mthread_process_chains, ((void*)eparams));
  if (submix_pid != 0)
    throw(new ECA_ERROR("ECA-MAIN", "Unable to create a new thread (mthread_process_chains)."));

  if (sched_getscheduler(0) == SCHED_FIFO) {
    struct sched_param sparam;
    sparam.sched_priority = 10;
    if (pthread_setschedparam(chain_thread, SCHED_FIFO, &sparam) != 0)
      ecadebug->msg("(eca-main) Unable to change scheduling policy (mthread)!");
    else 
      ecadebug->msg("(eca-main) Using realtime-scheduling (SCHED_FIFO/10, mthread).");
  }

  while (true) {
    if (finished() == true) stop();
    interpret_queue();
    if (end_request) break;

    if (eparams->status() != ep_status_running) {
      //      sched_yield();
      nanosleep(&sleepcount, NULL);
      continue;
    }

    prehandle_control_position();
    for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
      (*inputs)[adev_sizet]->read_buffer(&eparams->inslots[adev_sizet]);
    }
    
    for(int chain_sizet = 0; chain_sizet != chain_count; chain_sizet++) {
      if ((*chains)[chain_sizet]->output_id == 0) {
	(*chains)[chain_sizet]->audioslot.make_silent();
	continue;
      }

      pthread_mutex_lock(eparams->chain_muts[chain_sizet]);
      while(eparams->chain_ready_for_submix[chain_sizet] == true) {
	pthread_cond_signal(eparams->chain_conds[chain_sizet]);
	pthread_cond_wait(eparams->chain_conds[chain_sizet],
			  eparams->chain_muts[chain_sizet]);
      }

      for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
	if ((*chains)[chain_sizet]->input_id == (*inputs)[audioslot_sizet]) {
	  (*chains)[chain_sizet]->audioslot.operator=(eparams->inslots[audioslot_sizet]);
	}
      }
	
      eparams->chain_ready_for_submix[chain_sizet] = true;
      pthread_cond_signal(eparams->chain_conds[chain_sizet]);
      pthread_mutex_unlock(eparams->chain_muts[chain_sizet]);
    }
    posthandle_control_position();
  }
  pthread_cancel(chain_thread);
  pthread_join(chain_thread,NULL);
}

void ECA_PROCESSOR::exec_mthreaded_passive(void) throw(ECA_ERROR*) {

  ecadebug->control_flow("Engine/Mixmode \"multithreaded passive\" selected");
  start();

  for (int chain_sizet = 0; chain_sizet < chain_count; chain_sizet++)
    eparams->chain_ready_for_submix[chain_sizet] = false;

  int submix_pid = pthread_create(&chain_thread, NULL, mthread_process_chains, ((void*)eparams));

  if (submix_pid != 0)
    throw(new ECA_ERROR("ECA-MAIN", "Unable to create a new thread (mthread_process_chains)."));

  while (!finished() && !end_request) {
    prehandle_control_position();
    for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
      (*inputs)[adev_sizet]->read_buffer(&eparams->inslots[adev_sizet]);
    }

    for(int chain_sizet = 0; chain_sizet < chain_count; chain_sizet++) {
      pthread_mutex_lock(eparams->chain_muts[chain_sizet]);

      while(eparams->chain_ready_for_submix[chain_sizet] == true) {
	pthread_cond_wait(eparams->chain_conds[chain_sizet],
			  eparams->chain_muts[chain_sizet]);
      }

      for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
	if ((*chains)[chain_sizet]->input_id == (*inputs)[audioslot_sizet]) {
	  (*chains)[chain_sizet]->audioslot.operator=(eparams->inslots[audioslot_sizet]);
	}
      }
      eparams->chain_ready_for_submix[chain_sizet] = true;
      pthread_cond_signal(eparams->chain_conds[chain_sizet]);
      pthread_mutex_unlock(eparams->chain_muts[chain_sizet]);
    }
    posthandle_control_position();
  }
  pthread_cancel(chain_thread);
  pthread_join(chain_thread,NULL);
}
