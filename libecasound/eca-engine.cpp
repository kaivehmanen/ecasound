// ------------------------------------------------------------------------
// eca-engine.cpp: Main processing engine
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <utility>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h> /* gettimeofday() */
#include <sys/resource.h>

#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>
#include <kvutils/procedure_timer.h>

#include "samplebuffer.h"
#include "audioio.h"
#include "audioio-types.h"
#include "audioio-buffered-proxy.h"
#include "midi-server.h"
#include "eca-chain.h"
#include "eca-session.h"
#include "eca-chainop.h"
#include "eca-error.h"
#include "eca-debug.h"
#include "eca-engine.h"
#include "eca-engine_impl.h"

/**
 * Class constructor. A pointer to an ECA_SESSION 
 * object must be given as argument. It must contain 
 * a connected chainsetup and that chainsetup must 
 * be in enabled state.
 *
 * @pre arg != 0
 * @pre arg->get_connected_chainsetup() != 0
 * @pre arg->get_connected_chainsetup()->is_enabled() == true
 * @post status() == ECA_ENGINE::engine_status_stopped

 * @post status() == ECA_ENGINE::engine_status_stopped
 */
ECA_ENGINE::ECA_ENGINE(ECA_SESSION* arg) 
  : session_repp(arg),
    mixslot_rep(arg->connected_chainsetup_repp->buffersize(),
		SAMPLE_SPECS::channel_count_default) {
  // --
  DBC_REQUIRE(arg != 0);
  DBC_REQUIRE(arg->get_connected_chainsetup() != 0);
  DBC_REQUIRE(arg->get_connected_chainsetup()->is_enabled() == true);
  // --

  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-engine) Engine/Initializing");

  impl_repp = new ECA_ENGINE_impl;

  init_variables();
  init_connection_to_chainsetup();
  init_multitrack_mode();
  init_mix_method();
  init_profiling();

  // --
  DBC_ENSURE(status() == ECA_ENGINE::engine_status_stopped);
  // --
}

/**
 * Class destructor.
 */
ECA_ENGINE::~ECA_ENGINE(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) ECA_ENGINE destructor!");

  if (session_repp != 0) {
    stop();

    if (csetup_repp != 0) {
      std::vector<CHAIN*>::iterator q = csetup_repp->chains.begin();
      while(q != csetup_repp->chains.end()) {
	if (*q != 0) (*q)->disconnect_buffer();
	++q;
      }
    }
  }

  dump_profile_info();

  delete impl_repp;

  set_status(ECA_ENGINE::engine_status_notready);
 
  ecadebug->control_flow("Engine exiting");
}

/**
 * Launches the engine. The requirement concerning the
 * session object apply as with the constructor function.
 *
 * @pre session_repp->get_connected_chainsetup() != 0
 * @pre session_repp->get_connected_chainsetup()->is_enabled() == true
 * @post status() == ECA_ENGINE::engine_status_stopped ||
 *       status() == ECA_ENGINE::engine_status_finished ||
 *	 status() == ECA_ENGINE::engine_status_error
 */
void ECA_ENGINE::exec(void) {
  // --
  DBC_REQUIRE(session_repp->get_connected_chainsetup() != 0);
  DBC_REQUIRE(session_repp->get_connected_chainsetup()->is_enabled() == true);
  // --

  switch(impl_repp->mixmode_rep) {
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

  if (outputs_finished_rep > 0) 
    ecadebug->msg("(eca-engine) Warning! An output object has raised an error! Out of disk space, permission denied, etc?");

  stop();
  std::vector<CHAIN*>::iterator q = csetup_repp->chains.begin();
  while(q != csetup_repp->chains.end()) {
    (*q)->disconnect_buffer();
    ++q;
  }

  // --
  DBC_ENSURE(status() == ECA_ENGINE::engine_status_stopped ||
	     status() == ECA_ENGINE::engine_status_finished ||
	     status() == ECA_ENGINE::engine_status_error);
  // --
}

/**
 * Sends 'cmd' to engines command queue. If 
 * session()->is_interactive() is true, commands are 
 * processed in the server's main loop. 
 */
void ECA_ENGINE::command(Engine_command_t cmd, double arg) {
  impl_repp->command_queue_rep.push_back(static_cast<int>(cmd),arg);
}

void ECA_ENGINE::set_status(ECA_ENGINE::Engine_status_t state) { 
  engine_status_rep = state; 
}

ECA_ENGINE::Engine_status_t ECA_ENGINE::status(void) const { 
  return(engine_status_rep); 
}

/**
 * Wait for a stop signal. Functions blocks until 
 * the signal is received or 'timeout' seconds
 * has elapsed.
 * 
 * @see signal_stop()
 */
void ECA_ENGINE::wait_for_stop(int timeout) {
  struct timeval now;
  gettimeofday(&now, 0);
  struct timespec sleepcount;
  sleepcount.tv_sec = now.tv_sec + timeout;
  sleepcount.tv_nsec = now.tv_usec * 1000;

  pthread_mutex_lock(&impl_repp->ecasound_stop_mutex_repp);
  int ret = pthread_cond_timedwait(&impl_repp->ecasound_stop_cond_repp, 
				   &impl_repp->ecasound_stop_mutex_repp, 
				    &sleepcount);
  pthread_mutex_unlock(&impl_repp->ecasound_stop_mutex_repp);

  if (ret == 0)
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Stop signal ok");
  else if (ret == -ETIMEDOUT)
    ecadebug->msg(ECA_DEBUG::info, "(eca-engine) Stop signal failed; timeout");
  else
    ecadebug->msg(ECA_DEBUG::info, "(eca-engine) Stop signal failed");
}

/**
 * Sends a stop signal indicating that engine
 * state has changed to stopped.
 * 
 * @see wait_for_stop()
 */
void ECA_ENGINE::signal_stop(void) {
  pthread_mutex_lock(&impl_repp->ecasound_stop_mutex_repp);
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Signaling stop");
  pthread_cond_broadcast(&impl_repp->ecasound_stop_cond_repp);
  pthread_mutex_unlock(&impl_repp->ecasound_stop_mutex_repp);
}

/**
 * Called only from class constructor.
 */
void ECA_ENGINE::init_profiling(void) {
  impl_repp->looptimer_low_rep = static_cast<double>(buffersize_rep) / csetup_repp->sample_rate();
  impl_repp->looptimer_mid_rep = static_cast<double>(buffersize_rep * 2) / csetup_repp->sample_rate();
  impl_repp->looptimer_high_rep = static_cast<double>(buffersize_rep) * prefill_threshold_rep / csetup_repp->sample_rate();

  impl_repp->looptimer_rep.set_lower_bound_seconds(impl_repp->looptimer_low_rep);
  impl_repp->looptimer_rep.set_upper_bound_seconds(impl_repp->looptimer_high_rep);
  impl_repp->looptimer_range_rep.set_lower_bound_seconds(impl_repp->looptimer_mid_rep);
  impl_repp->looptimer_range_rep.set_upper_bound_seconds(impl_repp->looptimer_mid_rep);
}

/**
 * Prints  profiling information to stderr.
 */
void ECA_ENGINE::dump_profile_info(void) {
  long int slower_than_rt = impl_repp->looptimer_rep.event_count() -
                            impl_repp->looptimer_rep.events_under_lower_bound() -
                            impl_repp->looptimer_rep.events_over_upper_bound();

  std::cerr << "(eca-engine) *** profile begin ***" << endl;
  std::cerr << "Total loops: " << kvu_numtostr(impl_repp->looptimer_rep.event_count()) << endl;
  std::cerr << "Loops faster than realtime: "  << kvu_numtostr(impl_repp->looptimer_rep.events_under_lower_bound());
  std::cerr << " (<" << kvu_numtostr(impl_repp->looptimer_low_rep * 1000, 1) << " msec)" << endl;
  std::cerr << "Loops slower than realtime: "  << kvu_numtostr(slower_than_rt);
  std::cerr << " (>=" << kvu_numtostr(impl_repp->looptimer_low_rep * 1000, 1) << " msec)" << endl;
  std::cerr << "Loops slower than realtime: "  << kvu_numtostr(impl_repp->looptimer_range_rep.events_over_upper_bound());
  std::cerr << " (>" << kvu_numtostr(impl_repp->looptimer_mid_rep * 1000, 1) << " msec)" << endl;
  std::cerr << "Loops exceeding all buffering: " << kvu_numtostr(impl_repp->looptimer_rep.events_over_upper_bound());
  std::cerr << " (>" << kvu_numtostr(impl_repp->looptimer_high_rep * 1000, 1) << " msec)" << endl;
  std::cerr << "Fastest loop: " << kvu_numtostr(impl_repp->looptimer_rep.min_duration_seconds() * 1000, 1);
  std::cerr << " msec." << endl;
  std::cerr << "Slowest loop: " << kvu_numtostr(impl_repp->looptimer_rep.max_duration_seconds() * 1000, 1);
  std::cerr << " msec." << endl;
  std::cerr << "Average loop time: " << kvu_numtostr(impl_repp->looptimer_rep.average_duration_seconds() * 1000, 1);
  std::cerr << " msec." << endl;
  std::cerr << "(eca-engine) *** profile end   ***" << endl;
}

/**
 * Called only from class constructor.
 */
void ECA_ENGINE::init_variables(void) {
  set_status(ECA_ENGINE::engine_status_stopped);
  buffersize_rep = session_repp->connected_chainsetup_repp->buffersize();
  use_midi_rep = false;
  max_channels_rep = 0;
  continue_request_rep = false;
  end_request_rep = false;
  rt_running_rep = false;
  trigger_counter_rep = 0;

  pthread_cond_init(&impl_repp->ecasound_stop_cond_repp, NULL);
  pthread_mutex_init(&impl_repp->ecasound_stop_mutex_repp, NULL);
}

/**
 * Called only from class constructor.
 */
void ECA_ENGINE::init_connection_to_chainsetup(void) {
  csetup_repp = session_repp->connected_chainsetup_repp;

  if (csetup_repp == 0 ) {
    std::cerr << "(eca-processor) Engine startup aborted, no chainsetup connected!";
    std::cerr << " Exiting..." << std::endl;
    exit(-1);
  }

  prefill_threshold_rep = 0;

  if (csetup_repp->max_buffers() == true) 
    prefill_threshold_rep = ECA_ENGINE::prefill_threshold_constant / buffersize_rep;

  if (prefill_threshold_rep < ECA_ENGINE::prefill_blocks_constant) 
    prefill_threshold_rep = ECA_ENGINE::prefill_blocks_constant;
  
  ecadebug->msg(ECA_DEBUG::system_objects,
		"(eca-engine) Prefill loops: " +
		kvu_numtostr(prefill_threshold_rep) +
		" (blocksize " +
		kvu_numtostr(buffersize_rep) +
		").");

  init_servers();
  init_sorted_input_map();
  init_sorted_output_map();
  init_inputs(); // input-output order is important here (sync fix)
  init_outputs();
  init_chains();
}

/**
 * 
 * Called only from init_connection_to_chainsetup().
 */
void ECA_ENGINE::init_servers(void) {
  if (csetup_repp->double_buffering() == true) {
    use_double_buffering_rep = true;
  }
  else
    use_double_buffering_rep = false;

  if (csetup_repp->midi_devices.size() > 0) {
    use_midi_rep = true;
    ecadebug->msg(ECA_DEBUG::info, "(eca-engine) Initializing MIDI-server.");
    csetup_repp->midi_server_repp->init();
  }
}

/**
 * Assign input objects in std::vectors of realtime, nonrealtime std::vectors,
 * and store pointers to the original objects.
 * 
 * Called only from init_connection_to_chainsetup().
 */
void ECA_ENGINE::init_sorted_input_map(void) {
  inputs_repp = &(csetup_repp->inputs);

  if (inputs_repp == 0 || inputs_repp->size() == 0) {
    std::cerr << "(eca-processor) Engine startup aborted, session in corrupted state: no inputs!";
    std::cerr << " Exiting..." << std::endl;
    exit(-1);
  }

  for(unsigned int adev_sizet = 0; adev_sizet < inputs_repp->size(); adev_sizet++) {
    if (AUDIO_IO_DEVICE::is_realtime_object((*inputs_repp)[adev_sizet]) == true) {
      realtime_inputs_rep.push_back(static_cast<AUDIO_IO_DEVICE*>((*inputs_repp)[adev_sizet]));
      realtime_objects_rep.push_back(static_cast<AUDIO_IO_DEVICE*>((*inputs_repp)[adev_sizet]));
    }
    else {
      non_realtime_inputs_rep.push_back((*inputs_repp)[adev_sizet]);
      non_realtime_objects_rep.push_back((*inputs_repp)[adev_sizet]);
    }
  }
  DBC_CHECK(static_cast<int>(realtime_inputs_rep.size()) == csetup_repp->number_of_realtime_inputs());
}

/**
 * Assign input objects in std::vectors of realtime, nonrealtime std::vectors,
 * and store pointers to the original objects.
 *
 * Called only from init_connection_to_chainsetup().
 */
void ECA_ENGINE::init_sorted_output_map(void) {
  outputs_repp =  &(csetup_repp->outputs);

  if (outputs_repp  == 0 ||
      outputs_repp->size() == 0) {
    std::cerr << "(eca-processor) Engine startup aborted, session in corrupted state: no outputs!";
    std::cerr << " Exiting..." << std::endl;
    exit(-1);
  }

  for(unsigned int adev_sizet = 0; adev_sizet < outputs_repp->size(); adev_sizet++) {
    if (AUDIO_IO_DEVICE::is_realtime_object((*outputs_repp)[adev_sizet]) == true) {
      realtime_outputs_rep.push_back(static_cast<AUDIO_IO_DEVICE*>((*outputs_repp)[adev_sizet]));
      realtime_objects_rep.push_back(static_cast<AUDIO_IO_DEVICE*>((*outputs_repp)[adev_sizet]));
    }
    else {
      non_realtime_outputs_rep.push_back((*outputs_repp)[adev_sizet]);
      non_realtime_objects_rep.push_back((*outputs_repp)[adev_sizet]);
    }
  }
  DBC_CHECK(static_cast<int>(realtime_outputs_rep.size()) == csetup_repp->number_of_realtime_outputs());
}

/**
 * Init all input objects by setting buffersize, sample
 * rate and channel count parameters. Also store the 
 * input start positions.
 *
 * Called only from init_connection_to_chainsetup().
 */
void ECA_ENGINE::init_inputs(void) {
  inputs_not_finished_rep = 0;

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
      csetup_repp->number_of_attached_chains_to_input(csetup_repp->inputs[adev_sizet]);

    if (csetup_repp->inputs[adev_sizet]->length_in_samples() > max_input_length)
      max_input_length = csetup_repp->inputs[adev_sizet]->length_in_samples();

    // ---
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Input \"" +
		  (*inputs_repp)[adev_sizet]->label() +  
		  "\": start position " +
		  kvu_numtostr(input_start_pos_rep[adev_sizet]) +  
		  ", number of connected chain " +
		  kvu_numtostr(input_chain_count_rep[adev_sizet]) + " .");
  }
  
  csetup_repp->set_position(0);
  if (csetup_repp->length_set() != true) {
    if (csetup_repp->looping_enabled() == true)
      processing_range_set_rep = true;
    else
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
 *
 * Called only from init_connection_to_chainsetup().
 */
void ECA_ENGINE::init_outputs(void) {
  trigger_outputs_request_rep = false;
  outputs_finished_rep = 0;

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
      csetup_repp->number_of_attached_chains_to_output(csetup_repp->outputs[adev_sizet]);

    // ---
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Output \"" +
		  (*outputs_repp)[adev_sizet]->label() +  
		  "\": start position " +
		  kvu_numtostr(output_start_pos_rep[adev_sizet]) + 
		  ", number of connected chain " +
		  kvu_numtostr(output_chain_count_rep[adev_sizet]) + " .");
  }
  mixslot_rep.number_of_channels(max_channels_rep);
  mixslot_rep.sample_rate(csetup_repp->sample_rate());
}

/**
 * Called only from init_connection_to_chainsetup().
 */
void ECA_ENGINE::init_chains(void) {
  chains_repp = &(csetup_repp->chains);

  if (chains_repp == 0 ||
      chains_repp->size() == 0) {
    std::cerr << "(eca-processor) Engine startup aborted, session in corrupted state: no chains!";
    std::cerr << " Exiting..." << std::endl;
    exit(-1);
  }

  while(cslots_rep.size() < chains_repp->size()) cslots_rep.push_back(SAMPLE_BUFFER(buffersize_rep, max_channels_rep, csetup_repp->sample_rate()));
}

/**
 * Called only from class constructor.
 */
void ECA_ENGINE::init_multitrack_mode(void) {
  // ---
  // Are we doing multitrack-recording?
  // ---    
  if (realtime_inputs_rep.size() > 0 && 
      realtime_outputs_rep.size() > 0 &&
      non_realtime_inputs_rep.size() > 0 && 
      non_realtime_outputs_rep.size() > 0 &&
      chains_repp->size() > 1) {
    ecadebug->msg("(eca-engine) Multitrack-mode enabled. Changed mixmode to \"normal\"");
    csetup_repp->multitrack_mode_rep = true;
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Using input " + realtime_inputs_rep[0]->label() + " for multitrack sync.");
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Using output " + realtime_outputs_rep[0]->label() + " for multitrack sync.");
  }
}

/**
 * Called from class constructor.
 */
void ECA_ENGINE::init_mix_method(void) { 
  impl_repp->mixmode_rep = csetup_repp->mixmode();

  if (csetup_repp->multitrack_mode_rep == true)  {
    impl_repp->mixmode_rep = ECA_CHAINSETUP::ep_mm_normal;
  }

  if (impl_repp->mixmode_rep == ECA_CHAINSETUP::ep_mm_auto) {
    if (chains_repp->size() == 1 &&
	inputs_repp->size() == 1 &&
	outputs_repp->size() == 1)
      impl_repp->mixmode_rep = ECA_CHAINSETUP::ep_mm_simple;
    else 
      impl_repp->mixmode_rep = ECA_CHAINSETUP::ep_mm_normal;
  }
  else if (impl_repp->mixmode_rep == ECA_CHAINSETUP::ep_mm_simple &&
	   (chains_repp->size() > 1 ||
	    inputs_repp->size() > 1 ||
	    outputs_repp->size() > 1)) {
    impl_repp->mixmode_rep = ECA_CHAINSETUP::ep_mm_normal;
    ecadebug->msg("(eca-engine) Warning! Setup too complex for simple mixmode.");
  }
}


void ECA_ENGINE::conditional_start(void) {
  if (was_running_rep == true) {
      start();
  }
}

void ECA_ENGINE::conditional_stop(void) {
  if (status() == ECA_ENGINE::engine_status_running) {
    ecadebug->msg(ECA_DEBUG::system_objects,"(eca-engine) conditional stop");
    was_running_rep = true;
    stop();
  }
  else was_running_rep = false;
}

/**
 * Checks for incoming messages and controls
 * internal state flags. 
 */
void ECA_ENGINE::update_engine_state(void) {
  
  // --
  // Updates engine status (if necessary).
  if (inputs_not_finished_rep == 0) {
    if (status() == ECA_ENGINE::engine_status_running) {
      ecadebug->msg(ECA_DEBUG::system_objects,"(eca-engine) input not finished / stop");
      stop();

      if (outputs_finished_rep > 0)
	set_status(ECA_ENGINE::engine_status_error);
      else
	set_status(ECA_ENGINE::engine_status_finished);
    }
  }

  // --
  // check whether processing has finished 
  if (status() == ECA_ENGINE::engine_status_finished ||
      status() == ECA_ENGINE::engine_status_error) {
    if (session_repp->iactive_rep != true) end_request_rep = true;
  }

  // --
  // process the command queue
  if (session_repp->iactive_rep == true) interpret_queue();

  // --
  // if in running state, poll for incoming commands
  if (end_request_rep != true &&
      session_repp->iactive_rep == true) {
    if (status() != ECA_ENGINE::engine_status_running) {
      impl_repp->command_queue_rep.poll(1, 0);
      continue_request_rep = true;
    }
    else 
      continue_request_rep = false;
  }
}

void ECA_ENGINE::interpret_queue(void) {
  while(impl_repp->command_queue_rep.is_empty() != true) {
    std::pair<int,double> item = impl_repp->command_queue_rep.front();
//      std::cerr << "(eca-engine) ecasound_queue: cmds available; first one is "
//  	 << item.first << "." << std::endl;
    switch(item.first) {
    // ---
    // Basic commands.
    // ---            
    case ep_exit:
      {
	while(impl_repp->command_queue_rep.is_empty() == false) impl_repp->command_queue_rep.pop_front();
	ecadebug->msg(ECA_DEBUG::system_objects,"(eca-engine) ecasound_queue: exit!");
	stop();
	end_request_rep = true;
	return;
      }
    case ep_start: { start(); break; }
    case ep_stop: { stop(); break; }

    // ---
    // Section/chain (en/dis)abling commands.
    // ---
    case ep_c_select: {	csetup_repp->active_chain_index_rep = static_cast<size_t>(item.second); break; }
    case ep_c_mute: { chain_muting(); break; }
    case ep_c_bypass: { chain_processing(); break; }
    case ep_c_rewind: { change_position_chain(- item.second); break; }
    case ep_c_forward: { change_position_chain(item.second); break; }
    case ep_c_setpos: { set_position_chain(item.second); break; }

    // ---
    // Chain operators
    // ---
    case ep_cop_select: { 
      csetup_repp->active_chainop_index_rep =  static_cast<size_t>(item.second);
      if (csetup_repp->active_chainop_index_rep - 1 < static_cast<int>((*chains_repp)[csetup_repp->active_chain_index_rep]->number_of_chain_operators()))
	(*chains_repp)[csetup_repp->active_chain_index_rep]->select_chain_operator(csetup_repp->active_chainop_index_rep);
      else 
	csetup_repp->active_chainop_index_rep = 0;
      break;
    }
    case ep_copp_select: { 
      csetup_repp->active_chainop_param_index_rep = static_cast<size_t>(item.second);
      (*chains_repp)[csetup_repp->active_chain_index_rep]->select_chain_operator_parameter(csetup_repp->active_chainop_param_index_rep);
      break;
    }
    case ep_copp_value: { 
      assert(chains_repp != 0);
      (*chains_repp)[csetup_repp->active_chain_index_rep]->set_parameter(item.second);
      break;
    }

    // ---
    // Global position
    // ---
    case ep_rewind: { change_position(- item.second); break; }
    case ep_forward: { change_position(item.second); break; }
    case ep_setpos: { set_position(item.second); break; }
    }
    impl_repp->command_queue_rep.pop_front();
  }
}

void ECA_ENGINE::stop(void) { 
  // --
  // as an exception, stop() will be performed even 
  // though status is not running anymore iff 
  // rt_running_rep is true (ie. rt-objects are still
  // in running state and have to be stopped)

  if (status() != ECA_ENGINE::engine_status_running && 
      rt_running_rep != true) return;

  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Stop");

  for (unsigned int adev_sizet = 0; adev_sizet != realtime_objects_rep.size(); adev_sizet++) {
    if (realtime_objects_rep[adev_sizet]->is_running() == true) realtime_objects_rep[adev_sizet]->stop();
  }
  
  if (csetup_repp->multitrack_mode_rep == true) {
    // ---
    // Reset all rt-devices to make sure device buffers
    // are in a clean state (to avoid sync problems)
    reset_realtime_devices();
  }

  stop_servers();
  rt_running_rep = false;

  // ---
  // Handle priority
  // ---
  if (csetup_repp->raised_priority() == true) {
    struct sched_param sparam;
    sparam.sched_priority = 0;
    if (::sched_setscheduler(0, SCHED_OTHER, &sparam) == -1)
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Unable to change scheduling back to SCHED_OTHER!");
    else
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Changed back to non-realtime scheduling SCHED_OTHER.");
  }

  set_status(ECA_ENGINE::engine_status_stopped);
  signal_stop();
}

void ECA_ENGINE::start(void) {
  if (status() == ECA_ENGINE::engine_status_running) return;
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Start");
  outputs_finished_rep = 0;

  // ---
  // Handle priority
  // ---
  if (csetup_repp->raised_priority() == true) {
    struct sched_param sparam;
    sparam.sched_priority = csetup_repp->sched_priority();
    if (::sched_setscheduler(0, SCHED_FIFO, &sparam) == -1)
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) Unable to change scheduling policy!");
    else 
      ecadebug->msg(ECA_DEBUG::info, "(eca-engine) Using realtime-scheduling (SCHED_FIFO).");
  }

  // ---
  // Start servers and devices 
  start_servers();

  // ---
  // Prepare rt-objects for processing
  for (unsigned int adev_sizet = 0; adev_sizet != realtime_objects_rep.size(); adev_sizet++) {
    realtime_objects_rep[adev_sizet]->prepare();
  }

  // ---
  // Handle multitrack and normal start differently
  if (csetup_repp->multitrack_mode_rep == true) {
    multitrack_start();
  }
  else {
    for (unsigned int adev_sizet = 0; adev_sizet != realtime_inputs_rep.size(); adev_sizet++)
      realtime_inputs_rep[adev_sizet]->start();

    trigger_outputs_request_rep = true;
  }

  // --- !!! ---
  //    char buf = 0xfa;
  //    int temp = ::write(midi_fd, &buf, 1);
  //    if (temp < 0) {
  //      std::cerr << "ERROR: Can't write to MIDI-device.\n";
  //    }
  // --- !!! ---

  rt_running_rep = true;
  set_status(ECA_ENGINE::engine_status_running);
}

void ECA_ENGINE::multitrack_start(void) {

  // --
  // prefill rt-output buffers before actually startin them
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-engine) multitrack sync (" + kvu_numtostr(prefill_threshold_rep) + " loops)");
  for(int n = 0; n < prefill_threshold_rep; n++) {
      multitrack_sync();
  }

  DBC_CHECK(realtime_inputs_rep.size() > 0);
  DBC_CHECK(realtime_outputs_rep.size() > 0);

  // ---
  // Start rt-inputs
  for (unsigned int adev_sizet = 0; adev_sizet != realtime_inputs_rep.size(); adev_sizet++)
    realtime_inputs_rep[adev_sizet]->start();

  // --
  // store time stamp when rt-inputs were started
  gettimeofday(&impl_repp->multitrack_input_stamp_rep, NULL);
  
  // --
  // start rt-outputs
  for (unsigned int adev_sizet = 0; adev_sizet != realtime_outputs_rep.size(); adev_sizet++)
    realtime_outputs_rep[adev_sizet]->start();
 
  struct timeval now;
  gettimeofday(&now, NULL);
  double time = now.tv_sec * 1000000.0 + now.tv_usec -
    impl_repp->multitrack_input_stamp_rep.tv_sec * 1000000.0 - impl_repp->multitrack_input_stamp_rep.tv_usec;
  long int sync_fix = static_cast<long>(time * csetup_repp->sample_rate() / 1000000.0);

  std::cerr << "(eca-engine) sync fix is " << time << " usecs." << endl;

  //  sync_fix -= prefill_threshold_rep * buffersize_rep;
  
  // sync_fix = total_samples_recorded - 
  //            processed_recorded_samples, 
  //            calculated when playback_samples==0 (just started)
  // ==> result: first byte written to non-rt outputs 
  //             was recorded exactly at 'now - sync_fix'
  
  // this would be a serious problem
  if (sync_fix < 0) {
    std::cerr << "(eca-engine) Negative multitrack-sync " 
              << sync_fix 
              << "; problems with hardware? " << std::endl;
  }
  else if (sync_fix > 0) {
    // write 'syncfix' samples of silence to nonrt outputs
    for (size_t n = 0; n != outputs_repp->size(); n++) {
      if (AUDIO_IO_DEVICE::is_realtime_object((*outputs_repp)[n]) != true) {
	for(size_t m = 0; m != chains_repp->size(); m++) {
	  if ((*chains_repp)[m]->connected_output() == static_cast<int>(n)) {
	    cslots_rep[m].length_in_samples(sync_fix);
	    cslots_rep[m].make_silent();
	    (*outputs_repp)[n]->write_buffer(&(cslots_rep[m]));
	    cslots_rep[m].length_in_samples(buffersize_rep);
	    ecadebug->msg(ECA_DEBUG::system_objects, 
			  "(eca-engine) sync fix: " + 
			  kvu_numtostr(sync_fix) + 
			  " for " +
			  (*outputs_repp)[n]->label() + 
			  ".");
    	  }
	}
      }
    }
  }
}

/**
 * Performs a close-open cycle for all realtime 
 * devices.
 */
void ECA_ENGINE::reset_realtime_devices(void) {
  for (size_t n = 0; n < realtime_objects_rep.size(); n++) {
    if (realtime_objects_rep[n]->is_open() == true) {
      ecadebug->msg(ECA_DEBUG::user_objects, 
		    "(eca-engine) Reseting rt-object " + 
		    realtime_objects_rep[n]->label());
      realtime_objects_rep[n]->close();
    }
  }
  for (size_t n = 0; n < realtime_objects_rep.size(); n++) {
    realtime_objects_rep[n]->open();
  }  
}


/**
 * The main engine loop for 'simple' mix-mode.
 */
void ECA_ENGINE::exec_simple_iactive(void) {
  int inch = (*inputs_repp)[(*chains_repp)[0]->connected_input()]->channels();
  int outch = (*outputs_repp)[(*chains_repp)[0]->connected_output()]->channels();
  (*chains_repp)[0]->init(&mixslot_rep, inch, outch);

  ecadebug->control_flow("Engine init - mixmode \"simple\"");
  if (session_repp->iactive_rep != true) start();
  inputs_not_finished_rep = 1; // for the 1st iteration
  while (true) {
    // --
    // handle state logic
    update_engine_state();
    if (end_request_rep) break;
    if (continue_request_rep) continue;

    if (rt_running_rep == true) { impl_repp->looptimer_rep.start(); impl_repp->looptimer_range_rep.start(); }

    // --
    // read from inputs to chain '0'
    inputs_not_finished_rep = 0;
    prehandle_control_position();
    (*inputs_repp)[0]->read_buffer(&mixslot_rep);
    if ((*inputs_repp)[0]->finished() == false) inputs_not_finished_rep++;

    // --
    // process chain '0'
    (*chains_repp)[0]->process();

    // --
    // mix to outputs
    (*outputs_repp)[0]->write_buffer(&mixslot_rep);
    if ((*outputs_repp)[0]->finished() == true) outputs_finished_rep++;

    if (rt_running_rep == true) { impl_repp->looptimer_rep.stop(); impl_repp->looptimer_range_rep.stop(); }

    trigger_outputs();
    posthandle_control_position();
  }
}

/**
 * The main engine loop for 'normal' mix-mode.
 */
void ECA_ENGINE::exec_normal_iactive(void) {
  ecadebug->control_flow("Engine init - mixmode \"normal\"");

  for (unsigned int c = 0; c != chains_repp->size(); c++) {
    int inch = (*inputs_repp)[(*chains_repp)[c]->connected_input()]->channels();
    int outch = (*outputs_repp)[(*chains_repp)[c]->connected_output()]->channels();
    (*chains_repp)[c]->init(&(cslots_rep[c]), inch, outch);
  }

  if (session_repp->iactive_rep != true) start();
  inputs_not_finished_rep = 1; // for the 1st iteration
  while (true) {
    // --
    // handle state logic
    update_engine_state();
    if (end_request_rep) break;
    if (continue_request_rep) continue;

    if (rt_running_rep == true) { impl_repp->looptimer_rep.start(); impl_repp->looptimer_range_rep.start(); }

    // --
    // inputs -> chains -> outputs
    inputs_not_finished_rep = 0;
    prehandle_control_position();
    inputs_to_chains(false);
    process_chains();
    mix_to_outputs(false);

    if (rt_running_rep == true) { impl_repp->looptimer_rep.stop(); impl_repp->looptimer_range_rep.stop(); }

    trigger_outputs();
    posthandle_control_position();

  }
}

/**
 * Performs one processing loop skipping all outputs
 * which are not rt-targets. The idea is to prefill all 
 * the output buffers before starting actual recording.
 */
void ECA_ENGINE::multitrack_sync(void) {
  inputs_to_chains(true);
  process_chains();
  mix_to_outputs(true);
}

void ECA_ENGINE::process_chains(void) {
    chain_i chain_iter = chains_repp->begin();
    while(chain_iter != chains_repp->end()) {
      (*chain_iter)->process();
      ++chain_iter;
    }
}

void ECA_ENGINE::inputs_to_chains(bool skip_realtime_inputs) {
  for(unsigned int audioslot_sizet = 0; audioslot_sizet < inputs_repp->size(); audioslot_sizet++) {

    if (skip_realtime_inputs == true) {
      if (AUDIO_IO_DEVICE::is_realtime_object((*inputs_repp)[audioslot_sizet]) == true) {
	//  std::cerr << "(eca-engine) Skipping rt-input " << (*inputs_repp)[audioslot_sizet]->label() << "." << endl;
	continue;
      }
    }

    if (input_chain_count_rep[audioslot_sizet] > 1) {
      (*inputs_repp)[audioslot_sizet]->read_buffer(&mixslot_rep);
      if ((*inputs_repp)[audioslot_sizet]->finished() == false) inputs_not_finished_rep++;
    }
    for (unsigned int c = 0; c != chains_repp->size(); c++) {
      if ((*chains_repp)[c]->connected_input() == static_cast<int>(audioslot_sizet)) {
	if (input_chain_count_rep[audioslot_sizet] == 1) {
	  (*inputs_repp)[audioslot_sizet]->read_buffer(&(cslots_rep[c]));
	  if ((*inputs_repp)[audioslot_sizet]->finished() == false) inputs_not_finished_rep++;
	  break;
	}
	else {
	  cslots_rep[c].operator=(mixslot_rep);
	}
      }
    }
  }
}

void ECA_ENGINE::mix_to_outputs(bool skip_realtime_target_outputs) {
  for(unsigned int audioslot_sizet = 0; audioslot_sizet < outputs_repp->size(); audioslot_sizet++) {
    mixslot_rep.number_of_channels((*outputs_repp)[audioslot_sizet]->channels());

    if (skip_realtime_target_outputs == true) {
      if (csetup_repp->is_realtime_target_output(audioslot_sizet) == true) {
	//  std::cerr << "(eca-engine) Skipping rt-target output " << (*outputs_repp)[audioslot_sizet]->label() << "." << endl;
	continue;
      }
    }

    int count = 0;
    
    for(unsigned int n = 0; n != chains_repp->size(); n++) {
      // --
      // if chain is already released, skip
      // --
      if ((*chains_repp)[n]->connected_output() == -1) {
	// --
	// skip, if chain is not connected
	// --
	continue;
      }

      if ((*chains_repp)[n]->connected_output() == static_cast<int>(audioslot_sizet)) {
	// --
	// output is connected to this chain
	// --
	if (output_chain_count_rep[audioslot_sizet] == 1) {
	  // --
	  // there's only one output connected to this chain,
	  // so we don't need to mix anything
	  // --
	  (*outputs_repp)[audioslot_sizet]->write_buffer(&(cslots_rep[n]));
	  if ((*outputs_repp)[audioslot_sizet]->finished() == true) outputs_finished_rep++;
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
	    if ((*outputs_repp)[audioslot_sizet]->finished() == true) outputs_finished_rep++;
	    mixslot_rep.length_in_samples(buffersize_rep);
	  }
	}
      }
    }
  } 
}

void ECA_ENGINE::prehandle_control_position(void) {
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

void ECA_ENGINE::posthandle_control_position(void) {
  if (csetup_repp->is_over() == true &&
      processing_range_set_rep == true) {
    if (csetup_repp->looping_enabled() == true) {
      inputs_not_finished_rep = 1;
      rewind_to_start_position();
      csetup_repp->set_position(0);
      for(unsigned int adev_sizet = 0; adev_sizet < inputs_repp->size(); adev_sizet++) {
	(*inputs_repp)[adev_sizet]->buffersize(buffersize_rep, csetup_repp->sample_rate());
      }
    }
    else {
      ecadebug->msg(ECA_DEBUG::system_objects,"(eca-engine) posthandle_c_p / stop");
      stop();
      csetup_repp->set_position(0);
      set_status(ECA_ENGINE::engine_status_finished);
    }
  }
}

void ECA_ENGINE::trigger_outputs(void) {
  if (trigger_outputs_request_rep == true) {
    ++trigger_counter_rep;
    if (trigger_counter_rep == prefill_threshold_rep) {
      trigger_outputs_request_rep = false;
      trigger_counter_rep = 0;
      for (unsigned int adev_sizet = 0; adev_sizet != realtime_outputs_rep.size(); adev_sizet++)
	realtime_outputs_rep[adev_sizet]->start();
      rt_running_rep = true;
    }
  }
}

void ECA_ENGINE::start_servers(void) {
  if (use_double_buffering_rep == true) {
    csetup_repp->pserver_repp->start();
    ecadebug->msg(ECA_DEBUG::info, "(eca-engine) Prefilling i/o buffers.");
    csetup_repp->pserver_repp->wait_for_full();
  }
  
  if (use_midi_rep == true) {
    csetup_repp->midi_server_repp->start();
  }
}

void ECA_ENGINE::stop_servers(void) { 
  if (use_double_buffering_rep == true) {
    csetup_repp->pserver_repp->stop();
    csetup_repp->pserver_repp->wait_for_stop();
  }

  if (use_midi_rep == true) {
    csetup_repp->midi_server_repp->stop();
  }
}

void ECA_ENGINE::chain_muting(void) {
  if ((*chains_repp)[csetup_repp->active_chain_index_rep]->is_muted()) 
    (*chains_repp)[csetup_repp->active_chain_index_rep]->toggle_muting(false);
  else
    (*chains_repp)[csetup_repp->active_chain_index_rep]->toggle_muting(true);
}

void ECA_ENGINE::chain_processing(void) {
  if ((*chains_repp)[csetup_repp->active_chain_index_rep]->is_processing()) 
    (*chains_repp)[csetup_repp->active_chain_index_rep]->toggle_processing(false);
  else
    (*chains_repp)[csetup_repp->active_chain_index_rep]->toggle_processing(true);
}

/**
 * Seeks to position 'seconds'. Affects all input and 
 * outputs objects, and the chainsetup object position.
 */
void ECA_ENGINE::set_position(double seconds) {
  conditional_stop();

  csetup_repp->set_position_exact(seconds);
  csetup_repp->seek_position();
  if (csetup_repp->double_buffering() == true) csetup_repp->pserver_repp->flush();

  conditional_start();
}

/**
 * Seeks to position 'seconds'. Affects all input and 
 * outputs objects, but not the chainsetup object position.
 */
void ECA_ENGINE::set_position_chain(double seconds) {
  conditional_stop();

  int id = (*chains_repp)[csetup_repp->active_chain_index_rep]->connected_input();
  AUDIO_IO* ptr = (*inputs_repp)[id];
  ptr->seek_position_in_seconds(seconds);

  id = (*chains_repp)[csetup_repp->active_chain_index_rep]->connected_output();
  ptr = (*outputs_repp)[id];
  ptr->seek_position_in_seconds(seconds);

  conditional_start();
}

/**
 * Seeks to position 'seconds'. Affects all input and 
 * outputs objects, and the chainsetup object position.
 */
void ECA_ENGINE::change_position(double seconds) {
  conditional_stop();

  csetup_repp->change_position_exact(seconds);
  csetup_repp->seek_position();
  if (csetup_repp->double_buffering() == true) csetup_repp->pserver_repp->flush();

  conditional_start();
}

/**
 * Seeks to begin position. Affects all input and 
 * outputs objects, but not the chainsetup object position.
 */
void ECA_ENGINE::rewind_to_start_position(void) {
  conditional_stop();

  for (unsigned int adev_sizet = 0; adev_sizet != inputs_repp->size(); adev_sizet++) {
    (*inputs_repp)[adev_sizet]->seek_position_in_samples(input_start_pos_rep[adev_sizet]);
  }

  for (unsigned int adev_sizet = 0; adev_sizet != outputs_repp->size(); adev_sizet++) {
    (*outputs_repp)[adev_sizet]->seek_position_in_samples(output_start_pos_rep[adev_sizet]);
  }

  conditional_start();
}

/**
 * Seeks to position 'now+seconds'. Affects all input and 
 * outputs objects, but not the chainsetup object position.
 */
void ECA_ENGINE::change_position_chain(double seconds) {
  conditional_stop();

  int id = (*chains_repp)[csetup_repp->active_chain_index_rep]->connected_input();
  AUDIO_IO* ptr = (*inputs_repp)[id]; 
  ptr->seek_position_in_seconds(ptr->position_in_seconds_exact() + seconds);
  
  id = (*chains_repp)[csetup_repp->active_chain_index_rep]->connected_output();
  ptr = (*outputs_repp)[id];
  ptr->seek_position_in_seconds(ptr->position_in_seconds_exact() + seconds);

  conditional_start();
}

double ECA_ENGINE::current_position(void) const { return(csetup_repp->position_in_seconds_exact()); }

double ECA_ENGINE::current_position_chain(void) const {
  AUDIO_IO* ptr = (*inputs_repp)[(*chains_repp)[csetup_repp->active_chain_index_rep]->connected_input()]; 
    return(ptr->position_in_seconds_exact());
  return(0.0f);
}
