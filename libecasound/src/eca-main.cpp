// ------------------------------------------------------------------------
// eca-main.cpp: Main processing engine
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
#include "audioio-types.h"
#include "eca-error.h"
#include "eca-debug.h"
#include "eca-mthreaded-processor.h"
#include "eca-main.h"

VALUE_QUEUE ecasound_queue;

ECA_PROCESSOR::ECA_PROCESSOR(ECA_SESSION* params) 
  :  eparams(params),
     mixslot(params->connected_chainsetup->buffersize(), 
	     SAMPLE_SPECS::channel_count_default),
     buffersize_rep(params->connected_chainsetup->buffersize()) {
  init();
}

ECA_PROCESSOR::ECA_PROCESSOR(void) { }

ECA_PROCESSOR::~ECA_PROCESSOR(void) {
  ecadebug->msg(ECA_DEBUG::system_objects,"ECA_PROCESSOR destructor!");

  stop();
  if (eparams != 0) eparams->status(ep_status_notready);

  pthread_cancel(chain_thread);
  pthread_join(chain_thread,NULL);

  ecadebug->control_flow("Engine/Exiting");
}

void ECA_PROCESSOR::init(ECA_SESSION* params) { 
  eparams = params;
  mixslot.length_in_samples(eparams->connected_chainsetup->buffersize());
  buffersize_rep = eparams->connected_chainsetup->buffersize();

  realtime_inputs.clear();
  realtime_outputs.clear();
  realtime_objects.clear();
  non_realtime_inputs.clear();
  non_realtime_outputs.clear();
  non_realtime_objects.clear();

  chain_ready_for_submix.clear();
  chain_muts.clear();
  chain_conds.clear();
  input_start_pos.clear();
  output_start_pos.clear();
  input_chain_count.clear();
  output_chain_count.clear();
  cslots.clear();

  init();
}

void ECA_PROCESSOR::init(void) {
  assert(eparams != 0);

  ecadebug->control_flow("Engine/Initializing");

  eparams->status(ep_status_stopped);

  init_variables();
  init_connection_to_chainsetup();
  init_multitrack_mode();
  init_mix_method();

  // ---
  // Handle priority
  // ---
  if (eparams->raised_priority() == true) {
    struct sched_param sparam;
    sparam.sched_priority = 10;
    if (sched_setscheduler(0, SCHED_FIFO, &sparam) == -1)
      ecadebug->msg("(eca-main) Unable to change scheduling policy!");
    else 
      ecadebug->msg("(eca-main) Using realtime-scheduling (SCHED_FIFO/10).");
  }
}

void ECA_PROCESSOR::init_variables(void) {
  active_chain_index = 0;
  sleepcount.tv_sec = 1;
  sleepcount.tv_nsec = 0;
  max_channels = 0;
  continue_request = false;
  end_request = false;
}

void ECA_PROCESSOR::init_connection_to_chainsetup(void) throw(ECA_ERROR*) {
  csetup = eparams->connected_chainsetup;

  if (csetup == 0 )
    throw(new ECA_ERROR("ECA_PROCESSOR", "Engine startup aborted, no chainsetup connected!"));

  init_outputs(); // input-output order is important here (sync fix)
  init_inputs();
  init_chains();
}

void ECA_PROCESSOR::init_inputs(void) {
  input_not_finished = true;

  inputs = &(csetup->inputs);
  input_count = static_cast<int>(inputs->size());

  if (inputs == 0 || inputs->size() == 0) {
    throw(new ECA_ERROR("ECA_PROCESSOR", "Engine startup aborted, session in corrupted state: no inputs!"));
  }

  input_start_pos.resize(input_count);
  input_chain_count.resize(input_count);
  long int max_input_length = 0;
  for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
    (*inputs)[adev_sizet]->buffersize(buffersize_rep, csetup->sample_rate());

    if ((*inputs)[adev_sizet]->channels() > max_channels)
      max_channels = (*inputs)[adev_sizet]->channels();

    AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>((*inputs)[adev_sizet]);
    if (p != 0) {
      realtime_inputs.push_back(p);
      realtime_objects.push_back(p);
    }
    else {
      non_realtime_inputs.push_back((*inputs)[adev_sizet]);
      non_realtime_objects.push_back((*inputs)[adev_sizet]);
    }

    input_start_pos[adev_sizet] = (*inputs)[adev_sizet]->position_in_samples();
    input_chain_count[adev_sizet] =
      eparams->number_of_connected_chains_to_input((*inputs)[adev_sizet]);
    if ((*inputs)[adev_sizet]->length_in_samples() > max_input_length)
      max_input_length = (*inputs)[adev_sizet]->length_in_samples();

    // ---
    //      ecadebug->msg(ECA_DEBUG::system_objects, "Input \"" + (*inputs)[adev_sizet]->label() +  ": start position " +
    //  		     kvu_numtostr(input_start_pos[adev_sizet]) +  ", number of connected chain " +
    //  		     kvu_numtostr(input_chain_count[adev_sizet]) + " .\n");
  }

  if (csetup->length_set() == false) {
    processing_range_set = false;
    csetup->length_in_samples(max_input_length);
  }
  else
    processing_range_set = true;
}

void ECA_PROCESSOR::init_outputs(void) {
  trigger_outputs_request = false;

  outputs = &(csetup->outputs);
  output_count = static_cast<int>(outputs->size());

  if (outputs  == 0 ||
      outputs->size() == 0) {
    throw(new ECA_ERROR("ECA_PROCESSOR", "Engine startup aborted, session in corrupted state: no outputs!"));
  }

  output_start_pos.resize(output_count);
  output_chain_count.resize(output_count);

  for(int adev_sizet = 0; adev_sizet < output_count; adev_sizet++) {
    (*outputs)[adev_sizet]->buffersize(buffersize_rep, csetup->sample_rate());

    if ((*outputs)[adev_sizet]->channels() > max_channels)
      max_channels = (*outputs)[adev_sizet]->channels();
		
    AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>((*outputs)[adev_sizet]);
    if (p != 0) {
      realtime_outputs.push_back(p);
      realtime_objects.push_back(p);
    }
    else {
      non_realtime_outputs.push_back((*outputs)[adev_sizet]);
      non_realtime_objects.push_back((*outputs)[adev_sizet]);
    }

    output_start_pos[adev_sizet] =
      (*outputs)[adev_sizet]->position_in_samples();

    output_chain_count[adev_sizet] =
      eparams->number_of_connected_chains_to_output((*outputs)[adev_sizet]);

    // ---
    //      ecadebug->msg(ECA_DEBUG::system_objects, "Output \"" + (*outputs)[adev_sizet]->label() +  ": start position " +
    //  		     kvu_numtostr(output_start_pos[adev_sizet]) + ", number of connected chain " +
    //  		     kvu_numtostr(output_chain_count[adev_sizet]) + " .\n");
  }
  mixslot.number_of_channels(max_channels);
  mixslot.sample_rate(csetup->sample_rate());
}

void ECA_PROCESSOR::init_chains(void) {
  chains = &(csetup->chains);
  chain_count = static_cast<int>(chains->size());

  if (chains == 0 ||
      chains->size() == 0) {
    throw(new ECA_ERROR("ECA_PROCESSOR", "Engine startup aborted, session in corrupted state: no chains!"));
  }

  chain_ready_for_submix.resize(chain_count);
  chain_muts.resize(chain_count);
  chain_conds.resize(chain_count);

  for(int n = 0; n < chain_count; n++) {
    chain_ready_for_submix[n] = false;

    pthread_mutex_t* mut = new pthread_mutex_t;
    pthread_mutex_init(mut, NULL);
    chain_muts[n] = mut;

    pthread_cond_t* cond = new pthread_cond_t;
    pthread_cond_init(cond, NULL);
    chain_conds[n] = cond;
  }

  while(cslots.size() < chains->size()) cslots.push_back(SAMPLE_BUFFER(buffersize_rep, max_channels, csetup->sample_rate()));
}

void ECA_PROCESSOR::init_multitrack_mode(void) {
  // ---
  // Are we doing multitrack-recording?
  // ---    
  if (realtime_inputs.size() > 0 && 
      realtime_outputs.size() > 0 &&
      non_realtime_inputs.size() > 0 && 
      non_realtime_outputs.size() > 0 && 
      chain_count > 1 && 
      eparams->iactive == true)  {
    ecadebug->msg("(eca-main) Multitrack-mode enabled. Changed mixmode to \"normal iactive\"");
    eparams->multitrack_mode = true;
    ecadebug->msg(ECA_DEBUG::system_objects, "Using input " + realtime_inputs[0]->label() + " for multitrack sync.");
    ecadebug->msg(ECA_DEBUG::system_objects, "Using output " + realtime_outputs[0]->label() + " for multitrack sync.");
  }
}

void ECA_PROCESSOR::init_mix_method(void) { 
  mixmode = csetup->mixmode();

  if (eparams->multitrack_mode == true)  {
    mixmode = ECA_CHAINSETUP::ep_mm_normal;
  }

  if (mixmode == ECA_CHAINSETUP::ep_mm_auto) {
    if (chain_count == 1 &&
	input_count == 1 &&
	output_count == 1)
      mixmode = ECA_CHAINSETUP::ep_mm_simple;
    else if (csetup->buffersize() > 1024 &&
	     chain_count > 1)
      mixmode = ECA_CHAINSETUP::ep_mm_mthreaded;
    else 
      mixmode = ECA_CHAINSETUP::ep_mm_normal;
  }
  else if (mixmode == ECA_CHAINSETUP::ep_mm_simple &&
	   (chain_count > 1 ||
	   input_count > 1 ||
	    output_count > 1)) {
    mixmode = ECA_CHAINSETUP::ep_mm_normal;
    ecadebug->msg("(eca-main) Warning! Setup too complex for simple mixmode.");
  }
}

void ECA_PROCESSOR::exec(void) {
  switch(mixmode) {
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

void ECA_PROCESSOR::interactive_loop(void) {
  if (finished() == true) stop();
  interpret_queue();
  if (eparams->status() != ep_status_running) {
    //      sched_yield();
    sleepcount.tv_sec = 1;
    sleepcount.tv_nsec = 0;
    nanosleep(&sleepcount, NULL);
    continue_request = true;
  }
  else 
    continue_request = false;
}

void ECA_PROCESSOR::exec_normal_iactive(void) {
  ecadebug->control_flow("Engine/Mixmode \"normal iactive\" selected");

  for (int c = 0; c != chain_count; c++) 
    (*chains)[c]->init(&(cslots[c]));
  
  while (true) {
    interactive_loop();
    if (end_request) break;
    if (continue_request) continue;
    input_not_finished = false;

    prehandle_control_position();
    inputs_to_chains();
    chain_i chain_iter = chains->begin();
    while(chain_iter != chains->end()) {
      (*chain_iter)->process();
      ++chain_iter;
    }
    mix_to_outputs();
    trigger_outputs();
    posthandle_control_position();
  }
}

void ECA_PROCESSOR::exec_normal_passive(void) {
  for (int c = 0; c != chain_count; c++) 
    (*chains)[c]->init(&(cslots[c]));
  start();
  
  ecadebug->control_flow("Engine/Mixmode \"normal passive\" selected");
  while (!finished()) {
    input_not_finished = false;
    prehandle_control_position();
    inputs_to_chains();
    chain_i chain_iter = chains->begin();
    while(chain_iter != chains->end()) {
      (*chain_iter)->process();
      ++chain_iter;
    }
    mix_to_outputs();
    trigger_outputs();
    posthandle_control_position();
  }
}


void ECA_PROCESSOR::exec_simple_iactive(void) {
  (*chains)[0]->init(&mixslot);

  ecadebug->control_flow("Engine/Mixmode \"simple iactive\" selected");
  while (true) {
    interactive_loop();
    if (end_request) break;
    if (continue_request) continue;
    input_not_finished = false;

    prehandle_control_position();
    (*inputs)[0]->read_buffer(&mixslot);
    if ((*inputs)[0]->finished() == false) input_not_finished = true;
    (*chains)[0]->process();
    (*outputs)[0]->write_buffer(&mixslot);
    trigger_outputs();
    posthandle_control_position();
  }
}

void ECA_PROCESSOR::exec_simple_passive(void) {
  (*chains)[0]->init(&mixslot);
  start();
  ecadebug->control_flow("Engine/Mixmode \"simple passive\" selected");
  while (!finished()) {
    input_not_finished = false;
    prehandle_control_position();
    (*inputs)[0]->read_buffer(&mixslot);
    if ((*inputs)[0]->finished() == false) input_not_finished = true;
    (*chains)[0]->process();
    (*outputs)[0]->write_buffer(&mixslot);
    trigger_outputs();
    posthandle_control_position();
  }
}

void ECA_PROCESSOR::set_position(double seconds) {
  conditional_stop();

  csetup->set_position(seconds * csetup->sample_rate());

  for (int adev_sizet = 0; adev_sizet != static_cast<int>(non_realtime_objects.size()); adev_sizet++) {
    non_realtime_objects[adev_sizet]->seek_position_in_seconds(seconds);
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

  for (int adev_sizet = 0; adev_sizet != static_cast<int>(non_realtime_objects.size()); adev_sizet++) {
    non_realtime_objects[adev_sizet]->seek_position_in_seconds(non_realtime_objects[adev_sizet]->position_in_seconds()
                                           + seconds);
  }

  conditional_start();
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

void ECA_PROCESSOR::change_position_chain(double seconds) {
  conditional_stop();

  (*chains)[active_chain_index]->input_id->seek_position_in_seconds(
								    (*chains)[active_chain_index]->input_id->position_in_seconds() + seconds);
  (*chains)[active_chain_index]->output_id->seek_position_in_seconds(
								     (*chains)[active_chain_index]->output_id->position_in_seconds() + seconds);
  
  conditional_start();
}

double ECA_PROCESSOR::current_position(void) const { return(csetup->position_in_seconds_exact()); }

double ECA_PROCESSOR::current_position_chain(void) const {
  return((*chains)[active_chain_index]->input_id->position_in_seconds_exact());
}

void ECA_PROCESSOR::prehandle_control_position(void) {
  csetup->change_position(buffersize_rep);
  if (csetup->is_over() == true &&
      processing_range_set == true) {
    int buffer_remain = csetup->position_in_samples() -
                        csetup->length_in_samples();
    for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
      (*inputs)[adev_sizet]->buffersize(buffer_remain, csetup->sample_rate());
    }
  }
}

void ECA_PROCESSOR::posthandle_control_position(void) {
  if (csetup->is_over() == true &&
      processing_range_set == true) {
    if (csetup->looping_enabled() == true) {
      rewind_to_start_position();
      csetup->set_position(0);
      for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
	(*inputs)[adev_sizet]->buffersize(buffersize_rep, csetup->sample_rate());
      }
    }
    else
      eparams->status(ep_status_finished);
  }
}

void ECA_PROCESSOR::stop(void) { 
  if (eparams->status() != ep_status_running) return;
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Stop");

  for (int adev_sizet = 0; adev_sizet != static_cast<int>(realtime_objects.size()); adev_sizet++) {
    realtime_objects[adev_sizet]->stop();
  }

  eparams->status(ep_status_stopped);
}

void ECA_PROCESSOR::start(void) {
  if (eparams->status() == ep_status_running) return;
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Start");

  for (int adev_sizet = 0; adev_sizet != static_cast<int>(realtime_objects.size()); adev_sizet++) {
    realtime_objects[adev_sizet]->prepare();
  }

  if (eparams->multitrack_mode == true) {
    assert(mixmode != ECA_CHAINSETUP::ep_mm_mthreaded);
    multitrack_sync();
    for (int adev_sizet = 0; adev_sizet != static_cast<int>(realtime_objects.size()); adev_sizet++) {
      realtime_objects[adev_sizet]->start();
    }

    assert(realtime_inputs.size() > 0);
    assert(realtime_outputs.size() > 0);

    long int output_sync = realtime_outputs[0]->position_in_samples();
    long int input_sync = realtime_inputs[0]->position_in_samples();

    ecadebug->msg(ECA_DEBUG::system_objects, "sync fix: " + kvu_numtostr(output_sync - input_sync));

    if (output_sync - input_sync > 0) {
      for (int adev_sizet = 0; adev_sizet != static_cast<int>(non_realtime_outputs.size()); adev_sizet++) {
	non_realtime_outputs[adev_sizet]->seek_position_in_samples(output_sync - input_sync);
      }
    }
  }
  else {
    for (int adev_sizet = 0; adev_sizet != static_cast<int>(realtime_inputs.size()); adev_sizet++)
      realtime_inputs[adev_sizet]->start();
    trigger_outputs_request = true;
  }

  eparams->status(ep_status_running);
}

void ECA_PROCESSOR::trigger_outputs(void) {
  if (trigger_outputs_request == true) {
    trigger_outputs_request = false;
    for (int adev_sizet = 0; adev_sizet != static_cast<int>(realtime_outputs.size()); adev_sizet++)
      realtime_outputs[adev_sizet]->start();
  }
}

void ECA_PROCESSOR::multitrack_sync(void) {
  // ---
  // Read and mix inputs (skips realtime inputs)
  // ---

  AUDIO_IO_DEVICE* p;
  for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
    p = dynamic_cast<AUDIO_IO_DEVICE*>((*inputs)[audioslot_sizet]);
    if (p != 0) continue;

    if (input_chain_count[audioslot_sizet] > 1) {
      (*inputs)[audioslot_sizet]->read_buffer(&mixslot);
      if ((*inputs)[audioslot_sizet]->finished() == false) input_not_finished = true;
    }
    for (int c = 0; c != chain_count; c++) {
      if ((*inputs)[audioslot_sizet] == (*chains)[c]->input_id) {
	if (input_chain_count[audioslot_sizet] == 1) {
	  (*inputs)[audioslot_sizet]->read_buffer(&(cslots[c]));
	  if ((*inputs)[audioslot_sizet]->finished() == false) input_not_finished = true;
	  break;
	}
	else {
	  cslots[c].operator=(mixslot);
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
    if (is_slave_output((*outputs)[audioslot_sizet]) == true) continue;
    mixslot.make_silent();
    int count = 0;
    
    for(int n = 0; n != chain_count; n++) {
      if ((*chains)[n]->output_id == 0) {
	continue;
      }

      if ((*chains)[n]->output_id == (*outputs)[audioslot_sizet]) {
	if (output_chain_count[audioslot_sizet] == 1) {
	  (*outputs)[audioslot_sizet]->write_buffer(&(cslots[n]));
	  break;
	}
	else {
	  ++count;
	  if (count == 1) {
	    mixslot.copy(cslots[n]);
	  }
	  else {
	    mixslot.add_with_weight(cslots[n],
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
    pair<int,double> item = ecasound_queue.front();
    ecadebug->msg(ECA_DEBUG::system_objects,"(eca-main) ecasound_queue: cmds available; first one is " 
		  + kvu_numtostr(item.first));
    switch(item.first) {
    // ---
    // Basic commands.
    // ---            
    case ep_exit:
      {
	while(ecasound_queue.is_empty() == false) ecasound_queue.pop_front();
	ecadebug->msg(ECA_DEBUG::system_objects,"(eca-main) ecasound_queue: exit!");
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
  if (input_not_finished == true &&
      eparams->status() != ep_status_finished) return(false);

  stop();
  eparams->status(ep_status_finished);
  return(true);
}

void ECA_PROCESSOR::inputs_to_chains(void) {
  for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
    if (input_chain_count[audioslot_sizet] > 1) {
      (*inputs)[audioslot_sizet]->read_buffer(&mixslot);
      if ((*inputs)[audioslot_sizet]->finished() == false) input_not_finished = true;
    }
    for (int c = 0; c != chain_count; c++) {
      if ((*chains)[c]->input_id == (*inputs)[audioslot_sizet]) {
	if (input_chain_count[audioslot_sizet] == 1) {
	  (*inputs)[audioslot_sizet]->read_buffer(&(cslots[c]));
	  if ((*inputs)[audioslot_sizet]->finished() == false) input_not_finished = true;
	  break;
	}
	else {
	  cslots[c].operator=(mixslot);
	}
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
	  (*outputs)[audioslot_sizet]->write_buffer(&(cslots[n]));
	  break;
	}
	else {
	  ++count;
	  if (count == 1) {
	    // -- 
	    // this is the first output connected to this chain
	    // --
	    mixslot.copy(cslots[n]);
	    mixslot.divide_by(output_chain_count[audioslot_sizet]);
	  }
	  else {
	    mixslot.add_with_weight(cslots[n],
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

bool ECA_PROCESSOR::is_slave_output(AUDIO_IO* aiod) const {
  // --------
  // require:
  assert(csetup != 0);
  // --------

  AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>(aiod);
  if (p != 0) return(false);
  vector<CHAIN*>::iterator q = csetup->chains.begin();
  while(q != csetup->chains.end()) {
    if ((*q)->output_id == aiod) {
      p = dynamic_cast<AUDIO_IO_DEVICE*>((*q)->input_id);
      if (p != 0) {
	ecadebug->msg(ECA_DEBUG::system_objects,"(eca-main) slave output detected: " + (*q)->output_id->label());
	return(true);
      }
    }
    ++q;
  }
  return(false);
}

void ECA_PROCESSOR::exec_mthreaded_iactive(void) throw(ECA_ERROR*) {
  for (int c = 0; c != chain_count; c++) 
    (*chains)[c]->init(&(cslots[c]));

  ecadebug->control_flow("Engine/Mixmode \"multithreaded interactive\" selected");
  int submix_pid = pthread_create(&chain_thread, NULL, mthread_process_chains, ((void*)this));
  if (submix_pid != 0)
    throw(new ECA_ERROR("ECA-MAIN", "Unable to create a new thread (mthread_process_chains)."));

  for (int chain_sizet = 0; chain_sizet < chain_count; chain_sizet++)
    chain_ready_for_submix[chain_sizet] = false;

  if (sched_getscheduler(0) == SCHED_FIFO) {
    struct sched_param sparam;
    sparam.sched_priority = 10;
    if (pthread_setschedparam(chain_thread, SCHED_FIFO, &sparam) != 0)
      ecadebug->msg("(eca-main) Unable to change scheduling policy (mthread)!");
    else 
      ecadebug->msg("(eca-main) Using realtime-scheduling (SCHED_FIFO/10, mthread).");
  }

  vector<SAMPLE_BUFFER> inslots (input_count, SAMPLE_BUFFER(buffersize_rep, max_channels, csetup->sample_rate()));

  while (true) {
    interactive_loop();
    if (end_request) break;
    if (continue_request) continue;
    input_not_finished = false;

    prehandle_control_position();
    for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
      (*inputs)[adev_sizet]->read_buffer(&inslots[adev_sizet]);
      if ((*inputs)[adev_sizet]->finished() == false) input_not_finished = true;
    }
    
    for(int chain_sizet = 0; chain_sizet != chain_count; chain_sizet++) {
      if ((*chains)[chain_sizet]->output_id == 0) {
	cslots[chain_sizet].make_silent();
	continue;
      }

      pthread_mutex_lock(chain_muts[chain_sizet]);
      while(chain_ready_for_submix[chain_sizet] == true) {
	pthread_cond_signal(chain_conds[chain_sizet]);
	pthread_cond_wait(chain_conds[chain_sizet], chain_muts[chain_sizet]);
      }

      for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
	if ((*chains)[chain_sizet]->input_id == (*inputs)[audioslot_sizet]) {
	  cslots[chain_sizet].operator=(inslots[audioslot_sizet]);
	}
      }
	
      chain_ready_for_submix[chain_sizet] = true;
      pthread_cond_signal(chain_conds[chain_sizet]);
      pthread_mutex_unlock(chain_muts[chain_sizet]);
    }
    posthandle_control_position();
  }
  pthread_cancel(chain_thread);
  pthread_join(chain_thread,NULL);
}

void ECA_PROCESSOR::exec_mthreaded_passive(void) throw(ECA_ERROR*) {
  for (int c = 0; c != chain_count; c++) 
    (*chains)[c]->init(&(cslots[c]));

  ecadebug->control_flow("Engine/Mixmode \"multithreaded passive\" selected");
  start();

  for (int chain_sizet = 0; chain_sizet < chain_count; chain_sizet++)
    chain_ready_for_submix[chain_sizet] = false;

  int submix_pid = pthread_create(&chain_thread, NULL, mthread_process_chains, ((void*)this));

  if (submix_pid != 0)
    throw(new ECA_ERROR("ECA-MAIN", "Unable to create a new thread (mthread_process_chains)."));

  vector<SAMPLE_BUFFER> inslots (input_count,
				 SAMPLE_BUFFER(buffersize_rep,
					       max_channels, 
					       csetup->sample_rate()));

  while (!finished()) {
    input_not_finished = false;
    prehandle_control_position();
    for(int adev_sizet = 0; adev_sizet < input_count; adev_sizet++) {
      (*inputs)[adev_sizet]->read_buffer(&inslots[adev_sizet]);
      if ((*inputs)[adev_sizet]->finished() == false) input_not_finished = true;
    }

    for(int chain_sizet = 0; chain_sizet < chain_count; chain_sizet++) {
      pthread_mutex_lock(chain_muts[chain_sizet]);

      while(chain_ready_for_submix[chain_sizet] == true) {
	pthread_cond_wait(chain_conds[chain_sizet], chain_muts[chain_sizet]);
      }

      for(int audioslot_sizet = 0; audioslot_sizet < input_count; audioslot_sizet++) {
	if ((*chains)[chain_sizet]->input_id == (*inputs)[audioslot_sizet]) {
	  cslots[chain_sizet].operator=(inslots[audioslot_sizet]);
	}
      }
      chain_ready_for_submix[chain_sizet] = true;
      pthread_cond_signal(chain_conds[chain_sizet]);
      pthread_mutex_unlock(chain_muts[chain_sizet]);
    }
    posthandle_control_position();
  }
  pthread_cancel(chain_thread);
  pthread_join(chain_thread,NULL);
}
