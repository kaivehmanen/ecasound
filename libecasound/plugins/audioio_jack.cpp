// ------------------------------------------------------------------------
// audioio-jack.cpp: Interface to JACK audio framework
// Copyright (C) 2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <iostream>

#include <sys/time.h> /* gettimeofday() */
#include <errno.h> /* ETIMEDOUT */

#include <jack/jack.h>
#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>

#include "audioio.h"
#include "eca-version.h"
#include "eca-debug.h"

/* Debug control flow */ 
// #define DEBUG_CFLOW

#ifdef DEBUG_CFLOW
#define DEBUG_CFLOW_STATEMENT(x) (x)
#else
#define DEBUG_CFLOW_STATEMENT(x) ((void)0)
#endif

static int eca_jack_process(nframes_t nframes, void *arg);
static int eca_jack_bufsize (nframes_t nframes, void *arg);
static int eca_jack_srate (nframes_t nframes, void *arg);
static void eca_jack_shutdown (void *arg);

#include "audioio_jack.h"

static const char* audio_io_keyword_const = "jack";
static const char* audio_io_keyword_regex_const = "^jack$";

const char* audio_io_keyword(void){return(audio_io_keyword_const); }
const char* audio_io_keyword_regex(void){return(audio_io_keyword_regex_const); }
int audio_io_interface_version(void) { return(ECASOUND_LIBRARY_VERSION_CURRENT); }

static int eca_jack_process(nframes_t nframes, void *arg) {
  JACK_INTERFACE* current = static_cast<JACK_INTERFACE*>(arg);
  sample_t *portbuf = static_cast<sample_t*>(jack_port_get_buffer(current->port_repp, nframes));

  DEBUG_CFLOW_STATEMENT(std::cerr << "p0");
  if (current->is_running() == true) {
    DEBUG_CFLOW_STATEMENT(std::cerr << "p1");
    current->cb_buffer_repp = static_cast<void*>(portbuf);
    current->cb_nframes_rep = static_cast<long int>(nframes);
    DEBUG_CFLOW_STATEMENT(std::cerr << "p2");
    current->signal_token();
    DEBUG_CFLOW_STATEMENT(std::cerr << "p3");
    current->wait_for_completion();
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "p4");
  }

  return(0);
}

static int eca_jack_bufsize (nframes_t nframes, void *arg) {
  JACK_INTERFACE* current = static_cast<JACK_INTERFACE*>(arg);
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack) " +  current->jackname_rep + ": eca_jack_bufsize");

  if (nframes != static_cast<nframes_t>(current->buffersize())) {
    ecadebug->msg(ECA_DEBUG::info, 
		  "(audioio-jack) " + current->jackname_rep + 
		  ": warning! jackd and ecasound buffersizes don't match!");
  }

  return(0);
}

static int eca_jack_srate (nframes_t nframes, void *arg) {
  JACK_INTERFACE* current = static_cast<JACK_INTERFACE*>(arg);

  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-jack) " + current->jackname_rep + 
		  ": setting srate to " + kvu_numtostr(nframes));
  current->set_samples_per_second(nframes);

  return(0);
}

static void eca_jack_shutdown (void *arg) {
  JACK_INTERFACE* current = static_cast<JACK_INTERFACE*>(arg);
  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-jack) " + current->jackname_rep + 
		  ": jackd shutdown, stopping processing");
  current->stop();
}

JACK_INTERFACE::JACK_INTERFACE (void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) constructor");

  pthread_cond_init(&token_cond_rep, NULL);
  pthread_mutex_init(&token_mutex_rep, NULL);
  pthread_cond_init(&completion_cond_rep, NULL);
  pthread_mutex_init(&completion_mutex_rep, NULL);
}

JACK_INTERFACE::~JACK_INTERFACE(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) destructor");
}
  
void JACK_INTERFACE::open(void) throw(AUDIO_IO::SETUP_ERROR&) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) open");

  cb_buffer_repp = 0;
  cb_nframes_rep = 0;
  set_channels(1);
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_f32_le);

  /* connect to server */
  if ((client_repp = jack_client_new (jackname_rep.c_str())) == 0) {
    throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-JACK: Can't connect to jack server with client name " + jackname_rep));
  }

  /* set callbacks */
  jack_set_process_callback(client_repp, eca_jack_process, static_cast<void*>(this));
  jack_set_buffer_size_callback(client_repp, eca_jack_bufsize, static_cast<void*>(this));
  jack_set_sample_rate_callback(client_repp, eca_jack_srate, static_cast<void*>(this));
  jack_on_shutdown(client_repp, eca_jack_shutdown, static_cast<void*>(this));

  ecadebug->msg(ECA_DEBUG::system_objects, "engine sample rate: " + kvu_numtostr(jack_get_sample_rate(client_repp)));

  toggle_open_state(true);
}

void JACK_INTERFACE::close(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) close");

  jack_client_close (client_repp);

  toggle_open_state(false);
}

long int JACK_INTERFACE::read_samples(void* target_buffer, long int samples) {
  DEBUG_CFLOW_STATEMENT(std::cerr << "r0");
  if (is_running() == true) {
    DEBUG_CFLOW_STATEMENT(std::cerr << "r1");
    wait_for_token();
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "r2");
    memcpy(target_buffer, cb_buffer_repp, cb_nframes_rep * frame_size());
    DEBUG_CFLOW_STATEMENT(std::cerr << "r3");
    signal_completion();
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "r4");
  }

  return(cb_nframes_rep);
}

void JACK_INTERFACE::write_samples(void* target_buffer, long int samples) { 
  DEBUG_CFLOW_STATEMENT(std::cerr << "w0");
  if (is_running() == true) {
    DEBUG_CFLOW_STATEMENT(std::cerr << "w1");
    wait_for_token();
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "w2");
    memcpy(cb_buffer_repp, target_buffer, cb_nframes_rep * frame_size());
    DEBUG_CFLOW_STATEMENT(std::cerr << "w3");
    signal_completion();
    DEBUG_CFLOW_STATEMENT(std::cerr << "w4");
  }
}

void JACK_INTERFACE::stop(void) { 
  AUDIO_IO_DEVICE::stop();

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) stop");

  if (io_mode() == AUDIO_IO::io_read) {
    if (jack_port_disconnect (client_repp, "ALSA I/O:Input 1", jack_port_name(port_repp))) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) Error! Cannot disconnect to ALSA input 1!");
    } 
  }
  else {
    if (jack_port_disconnect (client_repp, jack_port_name(port_repp), "ALSA I/O:Output 1")) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) Error! Cannot disconnect to ALSA output 1!");
    } 
  }

  if (jack_deactivate (client_repp)) {
    ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) Error! Cannot deactive client!");
  }

  if (jack_port_unregister(client_repp, port_repp)) {
    ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) Error! Cannot unregister client!");
  }
}

void JACK_INTERFACE::start(void) { 
  AUDIO_IO_DEVICE::start();

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) start");

  // FIXME: jackd doesn't seem to handle stop-start without
  //        re-registering the ports... bug or feature?

  /* register one JACK port */
  if (io_mode() == AUDIO_IO::io_read) 
    port_repp = jack_port_register(client_repp, jackname_rep.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  else
    port_repp = jack_port_register(client_repp, jackname_rep.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

  if (jack_activate (client_repp)) {
    ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) Error! Cannot active client!");
  }

  if (io_mode() == AUDIO_IO::io_read) {
    if (jack_port_connect (client_repp, "ALSA I/O:Input 1", jack_port_name(port_repp))) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) Error! Cannot connect to ALSA input 1!");
    } 
  }
  else {
    if (jack_port_connect (client_repp, jack_port_name(port_repp), "ALSA I/O:Output 1")) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) Error! Cannot connect to ALSA output 1!");
    } 
  }
}

void JACK_INTERFACE::prepare(void) {
  AUDIO_IO_DEVICE::prepare();

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) prepare");
}

SAMPLE_SPECS::sample_pos_t JACK_INTERFACE::position_in_samples(void) const {
  return(0);
}

void JACK_INTERFACE::set_parameter(int param, std::string value) {
  switch(param) 
    {
    case 1: { label(value); break; }
    case 2: { jackname_rep = value; break; }
    }
}

std::string JACK_INTERFACE::get_parameter(int param) const {
  switch(param) 
    {
    case 1: { return(label()); }
    case 2: { return(jackname_rep); }
    }  
  return("");
}

void JACK_INTERFACE::wait_for_token(void) {
  struct timeval now;
  gettimeofday(&now, 0);
  struct timespec sleepcount;
  sleepcount.tv_sec = now.tv_sec + 5;
  sleepcount.tv_nsec = now.tv_usec * 1000;
  int ret = 0;
  pthread_mutex_lock(&token_mutex_rep);
  if (token_rep != true) {
    ret = pthread_cond_timedwait(&token_cond_rep, 
				 &token_mutex_rep,
				 &sleepcount);
  }
  token_rep = false;
  pthread_mutex_unlock(&token_mutex_rep);

  if (ret == 0)
    ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) wait_for_token ok");
  else if (ret == -ETIMEDOUT)
    ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) wait_for_token failed; timeout");
  else
    ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) wait_for_token failed");
}

void JACK_INTERFACE::wait_for_completion(void) {
  struct timeval now;
  gettimeofday(&now, 0);
  struct timespec sleepcount;
  sleepcount.tv_sec = now.tv_sec + 5;
  sleepcount.tv_nsec = now.tv_usec * 1000;
  int ret = 0;
  pthread_mutex_lock(&completion_mutex_rep);
  if (completion_rep != true) {
    ret = pthread_cond_timedwait(&completion_cond_rep, 
				 &completion_mutex_rep,
				 &sleepcount);
  }
  completion_rep = false;
  pthread_mutex_unlock(&completion_mutex_rep);

  if (ret == 0)
    ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) wait_for_completion ok");
  else if (ret == -ETIMEDOUT)
    ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) wait_for_completion failed; timeout");
  else
    ecadebug->msg(ECA_DEBUG::info, "(audioio-jack) wait_for_completion failed");
}

void JACK_INTERFACE::signal_token(void) {
  pthread_mutex_lock(&token_mutex_rep);
  token_rep = true;
  pthread_cond_broadcast(&token_cond_rep);
  pthread_mutex_unlock(&token_mutex_rep);
}

void JACK_INTERFACE::signal_completion(void) {
  pthread_mutex_lock(&completion_mutex_rep);
  completion_rep = true;
  pthread_cond_broadcast(&completion_cond_rep);
  pthread_mutex_unlock(&completion_mutex_rep);
}
