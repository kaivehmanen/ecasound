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

#include <kvutils/kvu_numtostr.h>

#include "samplebuffer.h"
#include "eca-chain.h"
#include "eca-session.h"
#include "eca-chainop.h"
#include "audioio.h"
#include "audioio-types.h"
#include "eca-error.h"
#include "eca-debug.h"
#include "eca-main.h"

ECA_PROCESSOR::ECA_PROCESSOR(ECA_SESSION* params) 
  :  eparams_repp(params),
     mixslot_rep(params->connected_chainsetup_repp->buffersize(), 
	     SAMPLE_SPECS::channel_count_default),
     buffersize_rep(params->connected_chainsetup_repp->buffersize()) {
  init();
}

ECA_PROCESSOR::ECA_PROCESSOR(void) : eparams_repp(0) { }

ECA_PROCESSOR::~ECA_PROCESSOR(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "ECA_PROCESSOR destructor!");

  if (eparams_repp != 0) {
    eparams_repp->status(ECA_SESSION::ep_status_notready);
    stop();

    vector<CHAIN*>::iterator q = csetup_repp->chains.begin();
    while(q != csetup_repp->chains.end()) {
      (*q)->disconnect_buffer();
      ++q;
    }
  }
  
  if (csetup_repp->double_buffering() == true) {
    pserver_rep.stop();
    while(pserver_rep.is_running() != true) usleep(50000);
  }

  vector<AUDIO_IO_BUFFERED_PROXY*>::iterator p = proxies_rep.begin();
  while(p != proxies_rep.end()) {
    delete *p;
    ++p;
  }

  ecadebug->control_flow("Engine/Exiting");
}

void ECA_PROCESSOR::init(ECA_SESSION* params) { 
  eparams_repp = params;
  mixslot_rep.length_in_samples(eparams_repp->connected_chainsetup_repp->buffersize());
  buffersize_rep = eparams_repp->connected_chainsetup_repp->buffersize();
  
  realtime_inputs_rep.clear();
  realtime_outputs_rep.clear();
  realtime_objects_rep.clear();
  non_realtime_inputs_rep.clear();
  non_realtime_outputs_rep.clear();
  non_realtime_objects_rep.clear();
  proxy_inputs_rep.clear();
  proxy_outputs_rep.clear();

  input_start_pos_rep.clear();
  output_start_pos_rep.clear();
  input_chain_count_rep.clear();
  output_chain_count_rep.clear();
  cslots_rep.clear();

  init();
}

void ECA_PROCESSOR::init(void) {
  assert(eparams_repp != 0);

  ecadebug->msg(ECA_DEBUG::system_objects,"Engine/Initializing");

  eparams_repp->status(ECA_SESSION::ep_status_stopped);

  init_variables();
  init_connection_to_chainsetup();
  init_multitrack_mode();
  init_mix_method();
}

void ECA_PROCESSOR::init_variables(void) {
  active_chain_index_rep = 0;
  max_channels_rep = 0;
  continue_request_rep = false;
  end_request_rep = false;
  rt_running_rep = false;
  trigger_counter_rep = 0;
}

void ECA_PROCESSOR::init_connection_to_chainsetup(void) {
  csetup_repp = eparams_repp->connected_chainsetup_repp;

  if (csetup_repp == 0 ) {
    cerr << "(eca-processor) Engine startup aborted, no chainsetup connected!";
    cerr << " Exiting..." << endl;
    exit(-1);
  }

  init_pserver();
  create_sorted_input_map();
  create_sorted_output_map();
  init_inputs(); // input-output order is important here (sync fix)
  init_outputs();
  init_chains();
}

void ECA_PROCESSOR::init_pserver(void) {
  pserver_rep.set_buffer_defaults(csetup_repp->double_buffer_size() / buffersize_rep, 
				  buffersize_rep,
				  csetup_repp->sample_rate());
  pserver_rep.set_schedpriority(eparams_repp->schedpriority_rep - 1);
}

/**
 * Assign input objects in vectors of realtime, nonrealtime vectors,
 * and store pointers to the original objects.
 */
void ECA_PROCESSOR::create_sorted_input_map(void) {
  if (csetup_repp->double_buffering() != true) {
    inputs_repp = csetup_inputs_repp = &(csetup_repp->inputs);
  }
  else {
    for(unsigned int adev_sizet = 0; adev_sizet < csetup_repp->inputs.size(); adev_sizet++) {
      proxy_inputs_rep.push_back(csetup_repp->inputs[adev_sizet]);
    }
    csetup_inputs_repp = &(csetup_repp->inputs);
    inputs_repp = &proxy_inputs_rep;
  }

  if (inputs_repp == 0 || inputs_repp->size() == 0) {
    cerr << "(eca-processor) Engine startup aborted, session in corrupted state: no inputs!";
    cerr << " Exiting..." << endl;
    exit(-1);
  }

  for(unsigned int adev_sizet = 0; adev_sizet < inputs_repp->size(); adev_sizet++) {
    AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>((*inputs_repp)[adev_sizet]);
    if (p != 0) {
      realtime_inputs_rep.push_back(p);
      realtime_objects_rep.push_back(p);
    }
    else {
      if (csetup_repp->double_buffering() == true) {
	AUDIO_IO_BUFFERED_PROXY* proxy_client = new AUDIO_IO_BUFFERED_PROXY(&pserver_rep, (*inputs_repp)[adev_sizet]);
	proxies_rep.push_back(proxy_client);
	(*inputs_repp)[adev_sizet] = proxy_client;
      }
      non_realtime_inputs_rep.push_back((*inputs_repp)[adev_sizet]);
      non_realtime_objects_rep.push_back((*inputs_repp)[adev_sizet]);
    }
    csetup_orig_ptr_map_rep[(*csetup_inputs_repp)[adev_sizet]] = (*inputs_repp)[adev_sizet];
  }
}

/**
 * Assign input objects in vectors of realtime, nonrealtime vectors,
 * and store pointers to the original objects.
 */
void ECA_PROCESSOR::create_sorted_output_map(void) {
  if (csetup_repp->double_buffering() != true) {
    outputs_repp = csetup_outputs_repp = &(csetup_repp->outputs);
  }
  else {
    for(unsigned int adev_sizet = 0; adev_sizet < csetup_repp->outputs.size(); adev_sizet++) {
      proxy_outputs_rep.push_back(csetup_repp->outputs[adev_sizet]);
    }
    csetup_outputs_repp = &(csetup_repp->outputs);
    outputs_repp = &proxy_outputs_rep;
  }

  if (outputs_repp  == 0 ||
      outputs_repp->size() == 0) {
    cerr << "(eca-processor) Engine startup aborted, session in corrupted state: no outputs!";
    cerr << " Exiting..." << endl;
    exit(-1);
  }

  for(unsigned int adev_sizet = 0; adev_sizet < outputs_repp->size(); adev_sizet++) {
    AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>((*outputs_repp)[adev_sizet]);
    if (p != 0) {
      realtime_outputs_rep.push_back(p);
      realtime_objects_rep.push_back(p);
    }
    else {
      if (csetup_repp->double_buffering() == true) {
	AUDIO_IO_BUFFERED_PROXY* proxy_client = new AUDIO_IO_BUFFERED_PROXY(&pserver_rep, (*outputs_repp)[adev_sizet]);
	proxies_rep.push_back(proxy_client);
	(*outputs_repp)[adev_sizet] = proxy_client;
      }
      non_realtime_outputs_rep.push_back((*outputs_repp)[adev_sizet]);
      non_realtime_objects_rep.push_back((*outputs_repp)[adev_sizet]);
    }
    csetup_orig_ptr_map_rep[(*csetup_outputs_repp)[adev_sizet]] = (*outputs_repp)[adev_sizet];
  }
}

/**
 * Init all input objects by setting buffersize, sample
 * rate and channel count parameters. Also store the 
 * input start positions.
 */
void ECA_PROCESSOR::init_inputs(void) {
  input_not_finished_rep = true;

  input_start_pos_rep.resize(inputs_repp->size());
  input_chain_count_rep.resize(inputs_repp->size());
  long int max_input_length = 0;
  for(unsigned int adev_sizet = 0; adev_sizet < inputs_repp->size(); adev_sizet++) {
    (*inputs_repp)[adev_sizet]->buffersize(buffersize_rep, csetup_repp->sample_rate());

    if (csetup_repp->inputs[adev_sizet]->channels() > max_channels_rep)
      max_channels_rep = csetup_repp->inputs[adev_sizet]->channels();

    input_start_pos_rep[adev_sizet] = csetup_repp->inputs[adev_sizet]->position_in_samples();
    (*inputs_repp)[adev_sizet]->seek_position_in_samples(input_start_pos_rep[adev_sizet]);

    input_chain_count_rep[adev_sizet] =
      eparams_repp->number_of_connected_chains_to_input(csetup_repp->inputs[adev_sizet]);

    if (csetup_repp->inputs[adev_sizet]->length_in_samples() > max_input_length)
      max_input_length = csetup_repp->inputs[adev_sizet]->length_in_samples();

    // ---
    ecadebug->msg(ECA_DEBUG::system_objects, "Input \"" +
		  (*inputs_repp)[adev_sizet]->label() +  
		  "\": start position " +
		  kvu_numtostr(input_start_pos_rep[adev_sizet]) +  
		  ", number of connected chain " +
		  kvu_numtostr(input_chain_count_rep[adev_sizet]) + " .\n");
  }
  
  if (csetup_repp->length_set() == false) {
    processing_range_set_rep = false;
    csetup_repp->length_in_samples(max_input_length);
  }
  else
    processing_range_set_rep = true;
}

/**
 * Init all output objects by setting buffersize, sample
 * rate and channel count parameters. Also store the 
 * output start positions.
 */
void ECA_PROCESSOR::init_outputs(void) {
  trigger_outputs_request_rep = false;

  output_start_pos_rep.resize(outputs_repp->size());
  output_chain_count_rep.resize(outputs_repp->size());

  for(unsigned int adev_sizet = 0; adev_sizet < outputs_repp->size(); adev_sizet++) {
    (*outputs_repp)[adev_sizet]->buffersize(buffersize_rep, csetup_repp->sample_rate());

    if (csetup_repp->outputs[adev_sizet]->channels() > max_channels_rep)
      max_channels_rep = csetup_repp->outputs[adev_sizet]->channels();
    
    output_start_pos_rep[adev_sizet] =
      csetup_repp->outputs[adev_sizet]->position_in_samples();
    (*outputs_repp)[adev_sizet]->seek_position_in_samples(output_start_pos_rep[adev_sizet]);

    output_chain_count_rep[adev_sizet] =
      eparams_repp->number_of_connected_chains_to_output(csetup_repp->outputs[adev_sizet]);

    // ---
    ecadebug->msg(ECA_DEBUG::system_objects, "Output \"" +
		  (*outputs_repp)[adev_sizet]->label() +  
		  "\": start position " +
		  kvu_numtostr(output_start_pos_rep[adev_sizet]) + 
		  ", number of connected chain " +
		  kvu_numtostr(output_chain_count_rep[adev_sizet]) + " .\n");
  }
  mixslot_rep.number_of_channels(max_channels_rep);
  mixslot_rep.sample_rate(csetup_repp->sample_rate());
}

void ECA_PROCESSOR::init_chains(void) {
  chains_repp = &(csetup_repp->chains);

  if (chains_repp == 0 ||
      chains_repp->size() == 0) {
    cerr << "(eca-processor) Engine startup aborted, session in corrupted state: no chains!";
    cerr << " Exiting..." << endl;
    exit(-1);
  }

  while(cslots_rep.size() < chains_repp->size()) cslots_rep.push_back(SAMPLE_BUFFER(buffersize_rep, max_channels_rep, csetup_repp->sample_rate()));
}

void ECA_PROCESSOR::init_multitrack_mode(void) {
  // ---
  // Are we doing multitrack-recording?
  // ---    
  if (realtime_inputs_rep.size() > 0 && 
      realtime_outputs_rep.size() > 0 &&
      non_realtime_inputs_rep.size() > 0 && 
      non_realtime_outputs_rep.size() > 0 &&
      chains_repp->size() > 1) {
    ecadebug->msg("(eca-main) Multitrack-mode enabled. Changed mixmode to \"normal\"");
    eparams_repp->multitrack_mode_rep = true;
    ecadebug->msg(ECA_DEBUG::system_objects, "Using input " + realtime_inputs_rep[0]->label() + " for multitrack sync.");
    ecadebug->msg(ECA_DEBUG::system_objects, "Using output " + realtime_outputs_rep[0]->label() + " for multitrack sync.");
  }
}

void ECA_PROCESSOR::init_mix_method(void) { 
  mixmode_rep = csetup_repp->mixmode();

  if (eparams_repp->multitrack_mode_rep == true)  {
    mixmode_rep = ECA_CHAINSETUP::ep_mm_normal;
  }

  if (mixmode_rep == ECA_CHAINSETUP::ep_mm_auto) {
    if (chains_repp->size() == 1 &&
	inputs_repp->size() == 1 &&
	outputs_repp->size() == 1)
      mixmode_rep = ECA_CHAINSETUP::ep_mm_simple;
    else 
      mixmode_rep = ECA_CHAINSETUP::ep_mm_normal;
  }
  else if (mixmode_rep == ECA_CHAINSETUP::ep_mm_simple &&
	   (chains_repp->size() > 1 ||
	    inputs_repp->size() > 1 ||
	    outputs_repp->size() > 1)) {
    mixmode_rep = ECA_CHAINSETUP::ep_mm_normal;
    ecadebug->msg("(eca-main) Warning! Setup too complex for simple mixmode.");
  }
}

void ECA_PROCESSOR::exec(void) {
  if (csetup_repp->double_buffering() == true) {
    pserver_rep.start();
    ecadebug->msg(ECA_DEBUG::info, "(eca-main) Prefilling i/o buffers.");
    while(pserver_rep.is_full() != true) usleep(50000);
  }

  switch(mixmode_rep) {
  case ECA_CHAINSETUP::ep_mm_simple:
    {
      exec_simple_iactive();
      break;
    }
  case ECA_CHAINSETUP::ep_mm_normal:
    {
      exec_normal_iactive();
      break;
    }
  default: 
    {
      exec_normal_iactive();
    }
  }

  stop();
  vector<CHAIN*>::iterator q = csetup_repp->chains.begin();
  while(q != csetup_repp->chains.end()) {
    (*q)->disconnect_buffer();
    ++q;
  }
}

void ECA_PROCESSOR::conditional_start(void) {
  if (was_running_rep == true) {
    if (csetup_repp->double_buffering() != true) {
      start();
    }
    else {
      pserver_rep.start();
      while(pserver_rep.is_full() != true) usleep(50000);
    }
  }
}

void ECA_PROCESSOR::conditional_stop(void) {
  if (eparams_repp->status() == ECA_SESSION::ep_status_running) {
    was_running_rep = true;
    if (csetup_repp->double_buffering() != true) {
      stop();
    }
    else {
      pserver_rep.stop();
      while(pserver_rep.is_running() != true) usleep(50000);
    }
  }
  else was_running_rep = false;
}

void ECA_PROCESSOR::interactive_loop(void) {
  if (finished() == true) stop();
  if (eparams_repp->iactive_rep == true) interpret_queue();
  if (end_request_rep) return;
  if (eparams_repp->status() != ECA_SESSION::ep_status_running) {
    struct timespec sleepcount;
    sleepcount.tv_sec = 0;
    sleepcount.tv_nsec = 1000;
    nanosleep(&sleepcount, 0);
    continue_request_rep = true;
  }
  else 
    continue_request_rep = false;
}

void ECA_PROCESSOR::exec_simple_iactive(void) {
  (*chains_repp)[0]->init(&mixslot_rep);

  ecadebug->control_flow("Engine/Init - mixmode \"simple\"");
  if (eparams_repp->iactive_rep != true) start();
  while (true) {
    interactive_loop();
    if (end_request_rep) break;
    if (continue_request_rep) continue;
    input_not_finished_rep = false;

    prehandle_control_position();
    (*inputs_repp)[0]->read_buffer(&mixslot_rep);
    if ((*inputs_repp)[0]->finished() == false) input_not_finished_rep = true;
    (*chains_repp)[0]->process();
    (*outputs_repp)[0]->write_buffer(&mixslot_rep);
    trigger_outputs();
    posthandle_control_position();
    if (eparams_repp->iactive_rep != true &&
	finished()) break;
  }
  if (eparams_repp->iactive_rep != true) stop();
}

void ECA_PROCESSOR::exec_normal_iactive(void) {
  ecadebug->control_flow("Engine/Init - mixmode \"normal\"");

  for (unsigned int c = 0; c != chains_repp->size(); c++) 
    (*chains_repp)[c]->init(&(cslots_rep[c]));

  if (eparams_repp->iactive_rep != true) start();
  while (true) {
    if (eparams_repp->iactive_rep == true) {
      interactive_loop();
      if (end_request_rep) break;
      if (continue_request_rep) continue;
    }
    input_not_finished_rep = false;

    prehandle_control_position();
    inputs_to_chains();
    chain_i chain_iter = chains_repp->begin();
    while(chain_iter != chains_repp->end()) {
      (*chain_iter)->process();
      ++chain_iter;
    }
    mix_to_outputs();
    trigger_outputs();
    posthandle_control_position();
    if (eparams_repp->iactive_rep != true &&
	finished()) break;
  }
  if (eparams_repp->iactive_rep != true) stop();
}


void ECA_PROCESSOR::set_position(double seconds) {
  conditional_stop();

  csetup_repp->set_position(seconds * csetup_repp->sample_rate());

  for (unsigned int adev_sizet = 0; adev_sizet != non_realtime_objects_rep.size(); adev_sizet++) {
    non_realtime_objects_rep[adev_sizet]->seek_position_in_seconds(seconds);
  }

  conditional_start();
}

void ECA_PROCESSOR::set_position_chain(double seconds) {
  conditional_stop();

  AUDIO_IO* ptr = (*chains_repp)[active_chain_index_rep]->input_id_repp;
  if (csetup_orig_ptr_map_rep.find(ptr) != csetup_orig_ptr_map_rep.end())
    csetup_orig_ptr_map_rep[ptr]->seek_position_in_seconds(seconds);

  ptr = (*chains_repp)[active_chain_index_rep]->output_id_repp;
  if (csetup_orig_ptr_map_rep.find(ptr) != csetup_orig_ptr_map_rep.end())
    csetup_orig_ptr_map_rep[ptr]->seek_position_in_seconds(seconds);

  conditional_start();
}

void ECA_PROCESSOR::change_position(double seconds) {
  conditional_stop();

  csetup_repp->change_position(seconds);

  for (unsigned int adev_sizet = 0; adev_sizet != non_realtime_objects_rep.size(); adev_sizet++) {
    non_realtime_objects_rep[adev_sizet]->seek_position_in_seconds(non_realtime_objects_rep[adev_sizet]->position_in_seconds()
                                           + seconds);
  }

  conditional_start();
}

void ECA_PROCESSOR::rewind_to_start_position(void) {
  conditional_stop();

  for (unsigned int adev_sizet = 0; adev_sizet != inputs_repp->size(); adev_sizet++) {
    (*inputs_repp)[adev_sizet]->seek_position_in_samples(input_start_pos_rep[adev_sizet]);
  }

  for (unsigned int adev_sizet = 0; adev_sizet != outputs_repp->size(); adev_sizet++) {
    (*outputs_repp)[adev_sizet]->seek_position_in_samples(output_start_pos_rep[adev_sizet]);
  }

  conditional_start();
}

void ECA_PROCESSOR::change_position_chain(double seconds) {
  conditional_stop();

  AUDIO_IO* ptr = (*chains_repp)[active_chain_index_rep]->input_id_repp; 
  if (csetup_orig_ptr_map_rep.find(ptr) != csetup_orig_ptr_map_rep.end())
    csetup_orig_ptr_map_rep[ptr]->seek_position_in_seconds(csetup_orig_ptr_map_rep[ptr]->position_in_seconds() + seconds);
  
  ptr = (*chains_repp)[active_chain_index_rep]->output_id_repp;
  if (csetup_orig_ptr_map_rep.find(ptr) != csetup_orig_ptr_map_rep.end())
    csetup_orig_ptr_map_rep[ptr]->seek_position_in_seconds(csetup_orig_ptr_map_rep[ptr]->position_in_seconds() + seconds);

  conditional_start();
}

double ECA_PROCESSOR::current_position(void) const { return(csetup_repp->position_in_seconds_exact()); }

double ECA_PROCESSOR::current_position_chain(void) const {
  AUDIO_IO* ptr = (*chains_repp)[active_chain_index_rep]->input_id_repp; 
  if (csetup_orig_ptr_map_rep.find(ptr) != csetup_orig_ptr_map_rep.end())
    return(csetup_orig_ptr_map_rep[ptr]->position_in_seconds_exact());
  return(0.0f);
}

void ECA_PROCESSOR::prehandle_control_position(void) {
  csetup_repp->change_position(buffersize_rep);
  if (csetup_repp->is_over() == true &&
      processing_range_set_rep == true) {
    int buffer_remain = csetup_repp->position_in_samples() -
                        csetup_repp->length_in_samples();
    for(unsigned int adev_sizet = 0; adev_sizet < inputs_repp->size(); adev_sizet++) {
      (*inputs_repp)[adev_sizet]->buffersize(buffer_remain, csetup_repp->sample_rate());
    }
  }
}

void ECA_PROCESSOR::posthandle_control_position(void) {
  if (csetup_repp->is_over() == true &&
      processing_range_set_rep == true) {
    if (csetup_repp->looping_enabled() == true) {
      rewind_to_start_position();
      csetup_repp->set_position(0);
      for(unsigned int adev_sizet = 0; adev_sizet < inputs_repp->size(); adev_sizet++) {
	(*inputs_repp)[adev_sizet]->buffersize(buffersize_rep, csetup_repp->sample_rate());
      }
    }
    else {
      stop();
      csetup_repp->set_position(0);
      eparams_repp->status(ECA_SESSION::ep_status_finished);
    }
  }
}

void ECA_PROCESSOR::stop(void) { 
  if (eparams_repp->status() != ECA_SESSION::ep_status_running && rt_running_rep == false) return;
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Stop");

  if (rt_running_rep == true) {
    for (unsigned int adev_sizet = 0; adev_sizet != realtime_objects_rep.size(); adev_sizet++) {
      realtime_objects_rep[adev_sizet]->stop();
    }
  }
  rt_running_rep = false;

  // ---
  // Handle priority
  // ---
  if (eparams_repp->raised_priority() == true) {
    struct sched_param sparam;
    sparam.sched_priority = 0;
    if (::sched_setscheduler(0, SCHED_OTHER, &sparam) == -1)
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Unable to change scheduling back to SCHED_OTHER!");
    else
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Changed back to non-realtime scheduling SCHED_OTHER.");
  }

  eparams_repp->status(ECA_SESSION::ep_status_stopped);
  ::pthread_mutex_lock(eparams_repp->ecasound_stop_mutex_repp);
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Signaling stop-cond");
  ::pthread_cond_signal(eparams_repp->ecasound_stop_cond_repp);
  ::pthread_mutex_unlock(eparams_repp->ecasound_stop_mutex_repp);
}

void ECA_PROCESSOR::start(void) {
  if (eparams_repp->status() == ECA_SESSION::ep_status_running) return;
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Start");

  // ---
  // Handle priority
  // ---
  if (eparams_repp->raised_priority() == true) {
    struct sched_param sparam;
    sparam.sched_priority = eparams_repp->schedpriority_rep;
    if (::sched_setscheduler(0, SCHED_FIFO, &sparam) == -1)
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Unable to change scheduling policy!");
    else 
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) Using realtime-scheduling (SCHED_FIFO).");
  }

  for (unsigned int adev_sizet = 0; adev_sizet != realtime_objects_rep.size(); adev_sizet++) {
    realtime_objects_rep[adev_sizet]->prepare();
  }

  if (eparams_repp->multitrack_mode_rep == true) {
    for (unsigned int adev_sizet = 0; adev_sizet != realtime_inputs_rep.size(); adev_sizet++)
      realtime_inputs_rep[adev_sizet]->start();

    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) multitrack sync");
    multitrack_sync();
    multitrack_sync();

    for (unsigned int adev_sizet = 0; adev_sizet != realtime_outputs_rep.size(); adev_sizet++)
      realtime_outputs_rep[adev_sizet]->start();

    assert(realtime_inputs_rep.size() > 0);
    assert(realtime_outputs_rep.size() > 0);

    struct timeval now;
    gettimeofday(&now, NULL);
    double time = now.tv_sec * 1000000.0 + now.tv_usec -
      multitrack_input_stamp_rep.tv_sec * 1000000.0 - multitrack_input_stamp_rep.tv_usec;
    long int sync_fix = static_cast<long>(time * csetup_repp->sample_rate() / 1000000.0);

    // - sync_fix = time elapsed since recording the first input
    // fragment (not meant to be discarded) was started
    // ==> result: first byte written to non-realtime outputs 
    //     was recorded exactly at 'now - output_sync'

    // this would be a serious problem
    if (sync_fix < 0) {
      cerr << "(eca-main) Aborting! Negative multitrack-sync; problems with hardware?" << endl;
      exit(-1);
    }
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-main) sync fix: " + kvu_numtostr(sync_fix));
    for (unsigned int adev_sizet = 0; adev_sizet != non_realtime_outputs_rep.size(); adev_sizet++) {
      non_realtime_outputs_rep[adev_sizet]->seek_position_in_samples_advance(sync_fix);
    }
  }
  else {
    for (unsigned int adev_sizet = 0; adev_sizet != realtime_inputs_rep.size(); adev_sizet++)
      realtime_inputs_rep[adev_sizet]->start();
    trigger_outputs_request_rep = true;
  }

  rt_running_rep = true;
  eparams_repp->status(ECA_SESSION::ep_status_running);
}

void ECA_PROCESSOR::trigger_outputs(void) {
  if (trigger_outputs_request_rep == true) {
    ++trigger_counter_rep;
    if (trigger_counter_rep == 2) {
      trigger_outputs_request_rep = false;
      trigger_counter_rep = 0;
      for (unsigned int adev_sizet = 0; adev_sizet != realtime_outputs_rep.size(); adev_sizet++)
	realtime_outputs_rep[adev_sizet]->start();
      rt_running_rep = true;
    }
  }
}

void ECA_PROCESSOR::multitrack_sync(void) {
  // ---
  // Read and mix inputs
  // ---
  inputs_to_chains();
  gettimeofday(&multitrack_input_stamp_rep, NULL);
  
  // ---
  // Chainoperator processing phase
  // ---
  chain_i chain_iter = chains_repp->begin();
  while(chain_iter != chains_repp->end()) {
    (*chain_iter)->process();
    ++chain_iter;
  }

  // ---
  // Mix to outputs (skip non-realtime outputs which are connected to realtime inputs)
  // ---
  for(unsigned int audioslot_sizet = 0; audioslot_sizet < outputs_repp->size(); audioslot_sizet++) {
    if (is_slave_output((*outputs_repp)[audioslot_sizet]) == true) {
      continue;
    }
    mixslot_rep.make_silent();
    int count = 0;
    
    for(unsigned int n = 0; n != chains_repp->size(); n++) {
      if ((*chains_repp)[n]->output_id_repp == 0) {
	continue;
      }

      if ((*chains_repp)[n]->output_id_repp == (*csetup_outputs_repp)[audioslot_sizet]) {
	if (output_chain_count_rep[audioslot_sizet] == 1) {
	  (*outputs_repp)[audioslot_sizet]->write_buffer(&(cslots_rep[n]));
	  cslots_rep[n].length_in_samples(buffersize_rep);
	  break;
	}
	else {
	  ++count;
	  if (count == 1) {
	    mixslot_rep.copy(cslots_rep[n]);
	    mixslot_rep.divide_by(output_chain_count_rep[audioslot_sizet]);
	  }
	  else {
	    mixslot_rep.add_with_weight(cslots_rep[n],
					output_chain_count_rep[audioslot_sizet]);
	    
	    if (count == output_chain_count_rep[audioslot_sizet]) {
	      (*outputs_repp)[audioslot_sizet]->write_buffer(&mixslot_rep);
	      mixslot_rep.length_in_samples(buffersize_rep);
	    }
	  }
	}
      }
    }
  }
}

void ECA_PROCESSOR::interpret_queue(void) {
  while(eparams_repp->ecasound_queue_rep.is_empty() != true) {
    pair<int,double> item = eparams_repp->ecasound_queue_rep.front();
//      cerr << "(eca-main) ecasound_queue: cmds available; first one is "
//  	 << item.first << "." << endl;
    switch(item.first) {
    // ---
    // Basic commands.
    // ---            
    case ep_exit:
      {
	while(eparams_repp->ecasound_queue_rep.is_empty() == false) eparams_repp->ecasound_queue_rep.pop_front();
	ecadebug->msg(ECA_DEBUG::system_objects,"(eca-main) ecasound_queue: exit!");
	stop();
	end_request_rep = true;
	return;
      }
    case ep_start: { start(); break; }
    case ep_stop: { stop(); break; }

    // ---
    // Section/chain (en/dis)abling commands.
    // ---
    case ep_c_select: {	active_chain_index_rep = static_cast<size_t>(item.second); break; }
    case ep_c_mute: { chain_muting(); break; }
    case ep_c_bypass: { chain_processing(); break; }
    case ep_c_rewind: { change_position_chain(- item.second); break; }
    case ep_c_forward: { change_position_chain(item.second); break; }
    case ep_c_setpos: { set_position_chain(item.second); break; }

    // ---
    // Chain operators
    // ---
    case ep_cop_select: { active_chainop_index_rep = static_cast<size_t>(item.second); break; }
    case ep_copp_select: { active_chainop_param_index_rep = static_cast<size_t>(item.second); break; }
    case ep_copp_value: { 
      assert(chains_repp != 0);
      if (active_chainop_index_rep - 1 < (*chains_repp)[active_chain_index_rep]->chainops_rep.size()) {
	(*chains_repp)[active_chain_index_rep]->select_chain_operator(active_chainop_index_rep);
	(*chains_repp)[active_chain_index_rep]->set_parameter(active_chainop_param_index_rep, item.second);
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
    eparams_repp->ecasound_queue_rep.pop_front();
  }
}

bool ECA_PROCESSOR::finished(void) {
  if (input_not_finished_rep == true &&
      eparams_repp->status() != ECA_SESSION::ep_status_finished) return(false);
  
  eparams_repp->status(ECA_SESSION::ep_status_finished);
  return(true);
}

void ECA_PROCESSOR::inputs_to_chains(void) {
  for(unsigned int audioslot_sizet = 0; audioslot_sizet < inputs_repp->size(); audioslot_sizet++) {
    if (input_chain_count_rep[audioslot_sizet] > 1) {
      (*inputs_repp)[audioslot_sizet]->read_buffer(&mixslot_rep);
      if ((*inputs_repp)[audioslot_sizet]->finished() == false) input_not_finished_rep = true;
    }
    for (unsigned int c = 0; c != chains_repp->size(); c++) {
      if ((*chains_repp)[c]->input_id_repp == (*inputs_repp)[audioslot_sizet]) {
	if (input_chain_count_rep[audioslot_sizet] == 1) {
	  (*inputs_repp)[audioslot_sizet]->read_buffer(&(cslots_rep[c]));
	  if ((*inputs_repp)[audioslot_sizet]->finished() == false) input_not_finished_rep = true;
	  break;
	}
	else {
	  cslots_rep[c].operator=(mixslot_rep);
	}
      }
    }
  }
}

void ECA_PROCESSOR::mix_to_outputs(void) {
  for(unsigned int audioslot_sizet = 0; audioslot_sizet < outputs_repp->size(); audioslot_sizet++) {
    mixslot_rep.number_of_channels((*outputs_repp)[audioslot_sizet]->channels());
    int count = 0;
    
    for(unsigned int n = 0; n != chains_repp->size(); n++) {
      // --
      // if chain is already released, skip
      // --
      if ((*chains_repp)[n]->output_id_repp == 0) {
	// --
	// skip, if chain is not connected
	// --
	continue;
      }

      if ((*chains_repp)[n]->output_id_repp == (*csetup_outputs_repp)[audioslot_sizet]) {
	// --
	// output is connected to this chain
	// --
	if (output_chain_count_rep[audioslot_sizet] == 1) {
	  // --
	  // there's only one output connected to this chain,
	  // so we don't need to mix anything
	  // --
	  (*outputs_repp)[audioslot_sizet]->write_buffer(&(cslots_rep[n]));
	  cslots_rep[n].length_in_samples(buffersize_rep);
	  break;
	}
	else {
	  ++count;
	  if (count == 1) {
	    // -- 
	    // this is the first output connected to this chain
	    // --
	    mixslot_rep.copy(cslots_rep[n]);
	    mixslot_rep.divide_by(output_chain_count_rep[audioslot_sizet]);
	  }
	  else {
	    mixslot_rep.add_with_weight(cslots_rep[n],
				    output_chain_count_rep[audioslot_sizet]);
	  }
	  
	  if (count == output_chain_count_rep[audioslot_sizet]) {
	    (*outputs_repp)[audioslot_sizet]->write_buffer(&mixslot_rep);
	    mixslot_rep.length_in_samples(buffersize_rep);
	  }
	}
      }
    }
  } 
}

void ECA_PROCESSOR::chain_muting(void) {
  if ((*chains_repp)[active_chain_index_rep]->is_muted()) 
    (*chains_repp)[active_chain_index_rep]->toggle_muting(false);
  else
    (*chains_repp)[active_chain_index_rep]->toggle_muting(true);
}

void ECA_PROCESSOR::chain_processing(void) {
  if ((*chains_repp)[active_chain_index_rep]->is_processing()) 
    (*chains_repp)[active_chain_index_rep]->toggle_processing(false);
  else
    (*chains_repp)[active_chain_index_rep]->toggle_processing(true);
}

/**
 * Slave output is a non-realtime output which is not 
 * connected to any realtime inputs.
 */
bool ECA_PROCESSOR::is_slave_output(AUDIO_IO* aiod) const {
  // --------
  // require:
  assert(csetup_repp != 0);
  // --------

  AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>(aiod);
  if (p != 0) return(false);
  vector<CHAIN*>::iterator q = csetup_repp->chains.begin();
  while(q != csetup_repp->chains.end()) {
    if ((*q)->output_id_repp == aiod) {
      p = dynamic_cast<AUDIO_IO_DEVICE*>((*q)->input_id_repp);
      if (p != 0) {
	ecadebug->msg(ECA_DEBUG::system_objects,"(eca-main) slave output detected: " + (*q)->output_id_repp->label());
	return(true);
      }
    }
    ++q;
  }
  return(false);
}
