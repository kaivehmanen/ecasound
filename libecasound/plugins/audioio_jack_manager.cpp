// ------------------------------------------------------------------------
// audioio_jack_manager.cpp: Manager for JACK client objects
// Copyright (C) 2001-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <algorithm> /* std::count() */
#include <iostream>
#include <string>

#include <sys/time.h> /* gettimeofday() */
#include <errno.h> /* ETIMEDOUT */
#include <jack/jack.h>

#include <kvu_dbc.h>
#include <kvu_numtostr.h>
#include <kvu_procedure_timer.h>
#include <kvu_threads.h>

#include "eca-engine.h"
#include "eca-logger.h"

/**
 * Enable and disable features
 */

/* Debug control flow */ 
// #define DEBUG_CFLOW

/* Profile callback execution */
// #define PROFILE_CALLBACK_EXECUTION

/**
 * Local macro definitions
 */

#ifdef DEBUG_CFLOW
#define DEBUG_CFLOW_STATEMENT(x) (x)
#else
#define DEBUG_CFLOW_STATEMENT(x) ((void)0)
#endif

#ifdef PROFILE_CALLBACK_EXECUTION
#include <asm/msr.h>
#define PROFILE_CE_STATEMENT(x) (x)
static PROCEDURE_TIMER profile_callback_timer;
#else
#define PROFILE_CE_STATEMENT(x) ((void)0)
#endif

/**
 * Prototypes for static functions
 */

static int eca_jack_process(nframes_t nframes, void *arg);
static int eca_jack_bufsize (nframes_t nframes, void *arg);
static int eca_jack_srate (nframes_t nframes, void *arg);
static void eca_jack_shutdown (void *arg);

#include "audioio_jack_manager.h"

using std::cerr;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::vector;

/**
 * Implementations of static functions
 */

/**
 * How many ecasound JACK manager instances 
 * can run at the same time (affects connection
 * setup time in some situations).
 */
const int AUDIO_IO_JACK_MANAGER::instance_limit = 8;

static int eca_jack_process(nframes_t nframes, void *arg)
{
  AUDIO_IO_JACK_MANAGER* current = static_cast<AUDIO_IO_JACK_MANAGER*>(arg);

#if 0 /* PROFILE_CALLBACK_EXECUTION */
  unsigned long long start, stop;
  float difference;

  /* start client timer */
  rdtscll (start);			
#endif

  PROFILE_CE_STATEMENT(profile_callback_timer.start());
  DEBUG_CFLOW_STATEMENT(cerr << endl << "jack_process entry ----> ");

  int ret = pthread_mutex_trylock(&current->lock_rep);
  if (ret == 0) {
    /* 1. copy audio data from port input buffers to ecasound buffers */
    
    for(size_t n = 0; n < current->inports_rep.size(); n++) {
      if (current->inports_rep[n].cb_buffer != 0) {
	sample_t* in_cb_buffer = static_cast<sample_t*>(jack_port_get_buffer(current->inports_rep[n].jackport, nframes));
	memcpy(current->inports_rep[n].cb_buffer, in_cb_buffer, current->buffersize_rep * sizeof(sample_t));
      }
    }
    
    DEBUG_CFLOW_STATEMENT(cerr << endl << "process 1 iter_in");
    DBC_CHECK(current->buffersize_rep == static_cast<long int>(nframes));
    
    /* 2. execute one engine iteration */

    if (current->engine_repp->is_active()) {
      current->engine_repp->engine_iteration();
    }
    
    DEBUG_CFLOW_STATEMENT(cerr << endl << "process 2 iter_out");
    
    /* 3. copy data from ecasound buffers to port output buffers */
    
    for(size_t n = 0; n < current->outports_rep.size(); n++) {
      if (current->outports_rep[n].cb_buffer != 0) {
	sample_t* out_cb_buffer = static_cast<sample_t*>(jack_port_get_buffer(current->outports_rep[n].jackport, nframes));
	// cerr << "(audioio_jack_manager) portbuf=" << out_cb_buffer << ", count=" << current->buffersize_rep << endl;
	memcpy(out_cb_buffer, current->outports_rep[n].cb_buffer, current->buffersize_rep * sizeof(sample_t));
      }
    }
    
    /* 4. update engine status based on the last iteration */
    
    current->engine_repp->update_engine_state();

    pthread_mutex_unlock(&current->lock_rep);
  }

#if 0 /* PROFILE_CALLBACK_EXECUTION */
  /* stop client timer */
  rdtscll (stop);
  
  difference = (float)(stop - start)/466000.0f;
  if (difference > 1) 
    jack_error ("jack-profile: process cycle took %.6f msecs\n", difference);
#endif
  
  PROFILE_CE_STATEMENT(profile_callback_timer.stop());
  DEBUG_CFLOW_STATEMENT(cerr << endl << "process out" << endl);
  
#ifdef PROFILE_CALLBACK_EXECUTION
  if (profile_callback_timer.last_duration_seconds() > 0.005f) {
    cerr << "(audioio-jack-manager) event " << profile_callback_timer.event_count();
    cerr << ", process() took " << profile_callback_timer.last_duration_seconds() * 1000;
    cerr << " msecs." << endl;
  }
  else {
    if (profile_callback_timer.event_count() < 5) {
      cerr << "(audioio-jack-manager) event " << profile_callback_timer.event_count();
      cerr << ", process() took " << profile_callback_timer.last_duration_seconds() * 1000;
      cerr << " msecs." << endl;
      
    }
  }
#endif
  
  return(0);
}

static int eca_jack_bufsize (nframes_t nframes, void *arg)
{
  AUDIO_IO_JACK_MANAGER* current = static_cast<AUDIO_IO_JACK_MANAGER*>(arg);
  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack-manager) [callback] " +  current->jackname_rep + ": eca_jack_bufsize");

  if (static_cast<long int>(nframes) != current->buffersize_rep) {
    current->shutdown_request_rep = true;
    ECA_LOG_MSG(ECA_LOGGER::info, 
		  "(audioio-jack-manager) Invalid new buffersize, shutting down.");
  }
  
  return(0);
}

static int eca_jack_srate (nframes_t nframes, void *arg)
{
  AUDIO_IO_JACK_MANAGER* current = static_cast<AUDIO_IO_JACK_MANAGER*>(arg);

  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"(audioio-jack-manager) [callback] " + current->jackname_rep + 
		": setting srate to " + kvu_numtostr(nframes));

  if (static_cast<long int>(nframes) != current->srate_rep) {
    current->shutdown_request_rep = true;
    ECA_LOG_MSG(ECA_LOGGER::info, 
		  "(audioio-jack-manager) Invalid new samplerate, shutting down.");
  }

  return(0);
}

static void eca_jack_shutdown (void *arg)
{
  AUDIO_IO_JACK_MANAGER* current = static_cast<AUDIO_IO_JACK_MANAGER*>(arg);
  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"(audioio-jack-manager) " + current->jackname_rep + 
		": [callback] jackd shutdown, stopping processing");
  current->shutdown_request_rep = true;
}

/**
 * Implementations of non-static functions
 */

AUDIO_IO_JACK_MANAGER::AUDIO_IO_JACK_MANAGER(void)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) constructor");

  total_nodes_rep = 0;
  active_nodes_rep = 0;

  last_in_port_rep = 0;
  last_out_port_rep = 0;

  open_rep = false;
  connection_active_rep = false;

  last_id_rep = 1;
  jackname_rep = "ecasound";

  pthread_cond_init(&exit_cond_rep, NULL);
  pthread_mutex_init(&exit_mutex_rep, NULL);
  pthread_mutex_init(&lock_rep, NULL);

  cb_allocated_frames_rep = 0;
  buffersize_rep = 0;
}

AUDIO_IO_JACK_MANAGER::~AUDIO_IO_JACK_MANAGER(void)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) destructor");

  list<int>::iterator p = objlist_rep.begin();
  while(p != objlist_rep.end()) {
    jack_node_t* tmp = jacknodemap_rep[*p];

    close(*p);

    delete tmp->aobj;
    jacknodemap_rep.erase(*p);

    ++p;
  }

  vector<jack_port_data_t>::iterator q = inports_rep.begin();
  while(q != inports_rep.end()) {
    delete[] q->cb_buffer;
    q->cb_buffer = 0;
    ++q;
  }

  vector<jack_port_data_t>::iterator r = outports_rep.begin();
  while(r != outports_rep.end()) {
    delete[] r->cb_buffer;
    r->cb_buffer = 0;
    ++r;
  }
}

bool AUDIO_IO_JACK_MANAGER::is_managed_type(const AUDIO_IO* aobj) const
{
  // ---
  DBC_REQUIRE(aobj != 0);
  // ---

  if (aobj->name() == "JACK interface") {
    DBC_CHECK(dynamic_cast<const AUDIO_IO_JACK*>(aobj) != 0);
    return(true);
  }

  return(false);
}

void AUDIO_IO_JACK_MANAGER::register_object(AUDIO_IO* aobj)
{
  // ---
  DBC_REQUIRE(aobj != 0);
  DBC_REQUIRE(is_managed_type(aobj) == true);
  // ---

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack-manager) register object " + aobj->label());  

  objlist_rep.push_back(last_id_rep);
  AUDIO_IO_JACK* jobj = static_cast<AUDIO_IO_JACK*>(aobj);

  jack_node_t* tmp = new jack_node_t;
  tmp->aobj = jobj;
  tmp->origptr = aobj;
  tmp->in_ports = tmp->out_ports = 0;
  tmp->first_in_port = tmp->first_out_port = -1;
  jacknodemap_rep[last_id_rep] = tmp;

  jobj->set_manager(this, last_id_rep);

  ++last_id_rep;
  ++total_nodes_rep;

  // ---
  DBC_ENSURE(is_managed_type(aobj) == true);
  // ---
}

int AUDIO_IO_JACK_MANAGER::get_object_id(const AUDIO_IO* aobj) const
{
  // ---
  DBC_REQUIRE(is_managed_type(aobj) == true);
  // ---

  map<int,jack_node_t*>::const_iterator p = jacknodemap_rep.begin();
  while(p != jacknodemap_rep.end()) {
    if (p->second->origptr == aobj) {
      ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		    "(audioio-jack-manager) found object id for aobj " +
		    aobj->name() + ": " + kvu_numtostr(p->first));
      return(p->first);
    }
    ++p;
  }

  return(-1);
}

const list<int>& AUDIO_IO_JACK_MANAGER::get_object_list(void) const
{
  return(objlist_rep);
}

/**
 * Unregisters object previously registered with register_object()
 * from the manager.
 *
 * @param id unique identifier for managed objects; @see
 *        get_object_id
 *
 */
void AUDIO_IO_JACK_MANAGER::unregister_object(int id)
{
  // ---
  DBC_DECLARE(int old_total_nodes = total_nodes_rep);
  // ---

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack-manager) unregister object ");

  objlist_rep.remove(id);

  map<int,jack_node_t*>::iterator p = jacknodemap_rep.begin();
  while(p != jacknodemap_rep.end()) {
    if (p->first == id) {
      ECA_LOG_MSG(ECA_LOGGER::system_objects,
		    "(audioio-jack-manager) removing object " + p->second->aobj->label());
      p->second->aobj->set_manager(0, -1);

      --total_nodes_rep;

      delete p->second;
      jacknodemap_rep.erase(p);
      break;
    }
    ++p;
  }

  // ---
  DBC_ENSURE(total_nodes_rep == old_total_nodes - 1);
  DBC_ENSURE(std::count(get_object_list().begin(), get_object_list().end(), id) == 0);
  // ---
}

void AUDIO_IO_JACK_MANAGER::exec(ECA_ENGINE* engine, ECA_CHAINSETUP* csetup)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) driver exec");

  engine_repp = engine;

  engine->init_engine_state();

  shutdown_request_rep = false;
  stop_request_rep = false;
  exit_request_rep = false;

  while(true) {

    engine_repp->wait_for_commands();

    engine_repp->check_command_queue();

    /* case 1: external exit request */
    if (exit_request_rep == true) {
      ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) exit request in exec");
      if (is_connection_active() == true) stop_connection();
      break;
    }

    /* case 2: external stop request */
    if (stop_request_rep == true) {
      stop_request_rep = false;
      ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) stop request in exec");
      if (is_connection_active() == true) stop_connection();
    }

    /* case 3: engine finished and in batch mode -> exit */
    if ((engine_repp->status() == ECA_ENGINE::engine_status_finished ||
	 engine_repp->status() == ECA_ENGINE::engine_status_error) &&
	engine->batch_mode() == true) {

      /* batch operation finished (or error occured) */
      ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) batch finished in exec");
      if (is_connection_active() == true) stop_connection();
      break;
    }

    /* case 4: problems with jack callbacks -> exit */
    if (shutdown_request_rep == true) {
      ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) problems with JACK callbacks");
      if (is_connection_active() == true) stop_connection();
      break;
    }
  }

  /* signal exit() that we are done */
  signal_exit();
}

/**
 * Activate connection to the JACK server.
 *
 * @pre is_connection_active() != true
 * @post is_connection_active() == true
 */
void AUDIO_IO_JACK_MANAGER::start(void)
{
  // --
  DBC_REQUIRE(is_connection_active() != true);
  // --

  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) driver start");

  /* prepare chainsetup for callbacks */
  engine_repp->start_operation();

  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) jack_activate()");
  if (jack_activate (client_repp)) {
    ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-jack-manager) Error! Cannot active client!");
  }

  /* connect all clients */
  map<int,jack_node_t*>::iterator p = jacknodemap_rep.begin();
  while(p != jacknodemap_rep.end()) {
    connect_node(p->second);
    ++p;
  }

  connection_active_rep = true;
}

void AUDIO_IO_JACK_MANAGER::stop(bool blocking)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) driver stop");

  stop_request_rep = true;

  if (blocking == true) {
    wait_for_stop();
  }
}

/**
 * Disconnects all connected ports and then
 * deactives the client.
 *
 * Takes care of locking, so is safe to 
 * call even when client is active.
 *
 * @pre is_connection_active() == true
 * @post is_connection_active() != true
 */
void AUDIO_IO_JACK_MANAGER::stop_connection(void)
{
  // --
  DBC_REQUIRE(is_connection_active() == true);
  // --

  pthread_mutex_lock(&lock_rep);

  /* disconnect all clients */
  map<int,jack_node_t*>::iterator p = jacknodemap_rep.begin();
  while(p != jacknodemap_rep.end()) {
    disconnect_node(p->second);
    ++p;
  }

  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) jack_deactivate() ");
  if (jack_deactivate (client_repp)) {
    ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-jack-manager) Error! Cannot deactive client!");
  }
 
  if (engine_repp->is_active() == true) engine_repp->stop_operation();

  connection_active_rep = false;

  pthread_mutex_unlock(&lock_rep);

  signal_stop();
}

void AUDIO_IO_JACK_MANAGER::exit(bool blocking)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) driver exit");

  exit_request_rep = true;

  if (blocking == true) {
    wait_for_exit();
  }
}

/**
 * Sets up automatic port connection for client_id's port
 * 'portnum'. When jack client is activated, this port
 * is automatically connected to port 'portname'. The 
 * direction of the connection is based on audio objects I/O mode 
 * (@see AUDIO_IO::io_mode()).
 *
 * @pre std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1
 * @pre is_open() == true
 @ @pre portnum > 0
 */
void AUDIO_IO_JACK_MANAGER::auto_connect_jack_port(int client_id, int portnum, const string& portname)
{
  // ---
  DBC_REQUIRE(std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1);
  DBC_REQUIRE(is_open() == true);
  DBC_REQUIRE(portnum > 0);
  // ---

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack-manager) auto-connect jack ports for client " + kvu_numtostr(client_id));

  jack_node_t* node = jacknodemap_rep[client_id];
  if (node->aobj->io_mode() == AUDIO_IO::io_read) {
    int index = node->first_in_port + portnum - 1;
    inports_rep[index].autoconnect = portname;
  }
  else {
    int index = node->first_out_port + portnum - 1;
    outports_rep[index].autoconnect = portname;
  }
}

/**
 * Registers new JACK port for client 'client_id'. The direction of
 * the port is based on audio objects I/O mode (@see
 * AUDIO_IO::io_mode()). If 'portname' is a non-empty string, 
 * the port will be automatically connected to the 'portname' 
 * port once jack client is activated.
 *
 * The final port names are of the form 'clientname:portprefix_N', 
 * where N is 1...max_port.
 *
 * @pre std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1
 * @pre is_open() == true
 */
void AUDIO_IO_JACK_MANAGER::register_jack_ports(int client_id, int ports, const string& portprefix)
{
  // ---
  DBC_REQUIRE(std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1);
  DBC_REQUIRE(is_open() == true);
  // ---

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack-manager) register jack ports for client " + kvu_numtostr(client_id));

  jack_node_t* node = jacknodemap_rep[client_id];
  if (node->aobj->io_mode() == AUDIO_IO::io_read) {
    node->first_in_port = last_in_port_rep;
    node->first_out_port = -1;
    node->in_ports = node->out_ports = 0;
    for(int n = 0; n < ports; n++) {
      node->in_ports++;
      inports_rep.push_back(jack_port_data_t ());
      inports_rep.back().cb_buffer = new sample_t [cb_allocated_frames_rep];
      inports_rep.back().autoconnect = "";
      string tport = portprefix + "_" + kvu_numtostr(last_in_port_rep + 1);
      inports_rep[last_in_port_rep].jackport = jack_port_register(client_repp, tport.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
      ++last_in_port_rep;
    }

    DBC_CHECK(inports_rep.size() == static_cast<size_t>(last_in_port_rep));

  }
  else {
    node->first_out_port = last_out_port_rep;
    node->first_in_port = -1;
    node->in_ports = node->out_ports = 0;
    for(int n = 0; n < ports; n++) {
      node->out_ports++;
      outports_rep.push_back(jack_port_data_t ());
      outports_rep.back().cb_buffer = new sample_t [cb_allocated_frames_rep];
      outports_rep.back().autoconnect = "";
      string tport = portprefix + "_" + kvu_numtostr(last_out_port_rep + 1);
      outports_rep[last_out_port_rep].jackport = jack_port_register(client_repp, tport.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
      ++last_out_port_rep;
    }

    DBC_CHECK(outports_rep.size() == static_cast<size_t>(last_out_port_rep));

  }
  
  // ---
  DBC_ENSURE(node->in_ports == 0 || node->out_ports == 0);
  // ---
}

/**
 * Unregisters all JACK ports for client 'client_id'.
 *
 * @pre std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1
 * @pre is_open() == true
 * @post node->in_ports == 0 && node->out_ports == 0
 */
void AUDIO_IO_JACK_MANAGER::unregister_jack_ports(int client_id)
{
  // ---
  DBC_REQUIRE(std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1);
  DBC_REQUIRE(is_open() == true);
  // ---

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack-manager) unregister all jack ports for client " + kvu_numtostr(client_id));

  /* FIXME: last_in_port_rep and last_out_port_rep
   *        are never reseted...
   */

  jack_node_t* node = jacknodemap_rep[client_id];

  for(int n = 0; n < static_cast<int>(inports_rep.size()); n++) {
    if (n >= node->first_in_port && 
	n < node->first_in_port + node->in_ports) {
      if (open_rep == true) 
	jack_port_unregister(client_repp, inports_rep[n].jackport);
      delete[] inports_rep[n].cb_buffer;
      inports_rep[n].cb_buffer = 0;
    }
  }

  node->first_in_port = -1;
  node->in_ports = 0;

  for(int n = 0; n < static_cast<int>(outports_rep.size()); n++) {
    if (n >= node->first_out_port && 
	n < node->first_out_port + node->out_ports) {
      if (open_rep == true)
	jack_port_unregister(client_repp, outports_rep[n].jackport);
      delete[] outports_rep[n].cb_buffer;
      outports_rep[n].cb_buffer = 0;
    }
  }

  node->first_out_port = -1;
  node->out_ports = 0;

  // ---
  DBC_ENSURE(node->in_ports == 0 && node->out_ports == 0);
  // ---
}

void AUDIO_IO_JACK_MANAGER::open(int client_id)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack-manager) open for client " + kvu_numtostr(client_id));

  DBC_CHECK(shutdown_request_rep != true);

  /* only for the first client */
  if (is_open() != true) {
    open_connection();
  }
}

void AUDIO_IO_JACK_MANAGER::close(int client_id)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack-manager) close for client " + kvu_numtostr(client_id));

  /* count how many open clients */
  int open_clients = 0;
  map<int,jack_node_t*>::iterator p = jacknodemap_rep.begin();
  while(p != jacknodemap_rep.end()) {
    if (p->second->aobj->is_open() == true) open_clients++;
    ++p;
  }

  DBC_CHECK(open_clients > 0);

  /* only for the last client */
  if (open_clients == 1) {
    if (is_open() == true) close_connection();
  }
}

/**
 * Returns current buffersize in sample frames. 
 * Always returns 0 if manager is not connected.
 */
long int AUDIO_IO_JACK_MANAGER::buffersize(void) const
{
  if (is_open() != true) return(0);

  return(buffersize_rep);
}

/**
 * Returns the current JACK engine sample rate.
 * Always returns 0 if manager is not connected.
 */
SAMPLE_SPECS::sample_rate_t AUDIO_IO_JACK_MANAGER::samples_per_second(void) const
{
  if (is_open() != true) return(0);

  return(srate_rep);
}

long int AUDIO_IO_JACK_MANAGER::read_samples(int client_id, void* target_buffer, long int samples)
{
  DBC_CHECK(samples == buffersize_rep);
  DEBUG_CFLOW_STATEMENT(cerr << endl << "read_samples:" << client_id);

  sample_t* ptr = static_cast<sample_t*>(target_buffer);
  jack_node_t* node = jacknodemap_rep[client_id];

  for(int n = node->first_in_port; n < node->first_in_port + node->in_ports; n++) {

    DBC_CHECK(n < static_cast<int>(inports_rep.size()));

    if (inports_rep[n].cb_buffer != 0) {
      memcpy(ptr, inports_rep[n].cb_buffer, buffersize_rep * sizeof(sample_t));
      ptr += buffersize_rep;
    }
  }

  return(buffersize_rep);
}

void AUDIO_IO_JACK_MANAGER::write_samples(int client_id, void* target_buffer, long int samples)
{
  DEBUG_CFLOW_STATEMENT(cerr << endl << "write_samples:" << client_id);

  long int writesamples = (samples <= buffersize_rep) ? samples : buffersize_rep;
  sample_t* ptr = static_cast<sample_t*>(target_buffer);
  jack_node_t* node = jacknodemap_rep[client_id];

  for(int n = node->first_out_port; n < node->first_out_port + node->out_ports; n++) {

    DBC_CHECK(n < static_cast<int>(outports_rep.size()));

    if (outports_rep[n].cb_buffer != 0) {
      memcpy(outports_rep[n].cb_buffer, ptr, writesamples * sizeof(sample_t));
      ptr += writesamples;
    }
  }
}

/**
 * Opens connection to the JACK server. Sets
 * is_open() to 'true' if connection is 
 * succesfully opened.
 *
 * @pre is_open() != true
 */
void AUDIO_IO_JACK_MANAGER::open_connection(void)
{
  // --
  DBC_REQUIRE(is_open() != true);
  // --

  string client_name ("ecasound");
  int n;

  for(n = 0; n < AUDIO_IO_JACK_MANAGER::instance_limit; n++) {
    client_repp = jack_client_new (client_name.c_str());
    if (client_repp != 0) break;
    client_name = "ecasound_" + kvu_numtostr(n + 2);
  }

  if (n != AUDIO_IO_JACK_MANAGER::instance_limit) {
    srate_rep = static_cast<long int>(jack_get_sample_rate(client_repp));
    /* FIXME: add better control of allocated memory */
    cb_allocated_frames_rep = buffersize_rep = static_cast<long int>(jack_get_buffer_size(client_repp));
    shutdown_request_rep = false;

    /* set callbacks */
    jack_set_process_callback(client_repp, eca_jack_process, static_cast<void*>(this));
    jack_set_buffer_size_callback(client_repp, eca_jack_bufsize, static_cast<void*>(this));
    jack_set_sample_rate_callback(client_repp, eca_jack_srate, static_cast<void*>(this));
    jack_on_shutdown(client_repp, eca_jack_shutdown, static_cast<void*>(this));
      
    open_rep = true;

#ifdef PROFILE_CALLBACK_EXECUTION
    profile_callback_timer.set_lower_bound_seconds(0.001f);
    profile_callback_timer.set_upper_bound_seconds(0.005f);
#endif

  }
  else {
    ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-jack-manager) Error! Cannot connect to JACK server!");
    open_rep = false;
  }
}

/**
 * Closes connection to the JACK server.
 *
 * @pre is_open() == true
 * @post is_open() != true
 */
void AUDIO_IO_JACK_MANAGER::close_connection(void)
{
  // --
  DBC_REQUIRE(is_open() == true);
  // --

  // FIXME: add proper unregistration
  // iterate over cids: unregister_jack_ports()

  jack_client_close (client_repp);

  open_rep = false;

  ECA_LOG_MSG(ECA_LOGGER::info, 
		"(audioio-jack-manager) Connection closed!");

#ifdef PROFILE_CALLBACK_EXECUTION
  cerr << profile_callback_timer.to_string() << endl;
#endif 

  // --
  DBC_ENSURE(is_open() != true);
  // --
}

/**
 * Connects ports of node 'node'. 
 *
 * @param node pointers to a node object
 * @param connect whether to connect (true) or disconnect (false)
 */
void AUDIO_IO_JACK_MANAGER::set_node_connection(jack_node_t* node, bool connect)
{
  if (node->aobj->io_mode() == AUDIO_IO::io_read) {
    for(int n = node->first_in_port; n < node->first_in_port + node->in_ports; n++) {
      if (inports_rep[n].cb_buffer != 0) {
	string tport = inports_rep[n].autoconnect;
	if (tport.size() > 0) {
	  if (connect == true) {
	    ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) jack_port_connect() ");
	    if (jack_connect (client_repp,
			      tport.c_str(), 
			      jack_port_name(inports_rep[n].jackport))) {
	      ECA_LOG_MSG(ECA_LOGGER::info, 
			    "(audioio-jack-manager) Error! Cannot connect input " + tport);
	    }
	  }
	  else {
	    ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) jack_port_disconnect()");
	    /* don't call jack_disconnect() if engine has shut down */
	    if (shutdown_request_rep != true &&
		jack_disconnect(client_repp, 
				tport.c_str(),
				jack_port_name(inports_rep[n].jackport))) {
	      ECA_LOG_MSG(ECA_LOGGER::info, 
			    "(audioio-jack-manager) Error! Cannot disconnect input " + tport);
	    }
	  }
	}
      }
    }
  }
  else {
    for(int n = node->first_out_port; n < node->first_out_port + node->out_ports; n++) {
      if (outports_rep[n].cb_buffer != 0) {
	string tport = outports_rep[n].autoconnect;
	if (tport.size() > 0) {
	  if (connect == true) {
	    ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) jack_port_connect()");
	    if (jack_connect(client_repp, 
			     jack_port_name(outports_rep[n].jackport), 
			     tport.c_str())) {
	      ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-jack-manager) Error! Cannot connect output " + tport);
	    }
	  }
	  else {
	    ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack-manager) jack_port_disconnect()");
	    /* don't call jack_disconnect() if engine has shut down */
	    if (shutdown_request_rep != true &&
		jack_disconnect(client_repp, 
				jack_port_name(outports_rep[n].jackport),
				tport.c_str())) {
	      ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-jack-manager) Error! Cannot disconnect output " + tport);
	    }
	  }
	}
      }
    }
  }

  if (connect == true)
    ++active_nodes_rep;
  else
    --active_nodes_rep;
}

/**
 * Connects ports of node 'node'.
 *
 * @see set_node_connection()
 */
void AUDIO_IO_JACK_MANAGER::connect_node(jack_node_t* node)
{ 
  if (shutdown_request_rep != true) {
    set_node_connection(node, true);
  }
  else {
    if (is_open() == true) close_connection();
  }
}

/**
 * Disconnects ports of node 'node'.
 *
 * @see set_node_connection()
 */
void AUDIO_IO_JACK_MANAGER::disconnect_node(jack_node_t* node)
{
  set_node_connection(node, false);
}

/**
 * Signals that exec() has exited.
 *
 * @see wait_for_exit();
 */
void AUDIO_IO_JACK_MANAGER::signal_exit(void)
{
  pthread_mutex_lock(&exit_mutex_rep);
  pthread_cond_signal(&exit_cond_rep);
  pthread_mutex_unlock(&exit_mutex_rep);
}

/**
 * Waits until exec() has exited.
 */
void AUDIO_IO_JACK_MANAGER::wait_for_exit(void)
{
  int ret = kvu_pthread_timed_wait(&exit_mutex_rep, &exit_cond_rep, 5);
  ECA_LOG_MSG(ECA_LOGGER::info, 
		kvu_pthread_timed_wait_result(ret, "(audioio_jack_manager) wait_for_exit"));
}

/**
 * Signals that client has stopped.
 *
 * @see wait_for_stop()
 */
void AUDIO_IO_JACK_MANAGER::signal_stop(void)
{
  pthread_mutex_lock(&exit_mutex_rep);
  pthread_cond_signal(&exit_cond_rep);
  pthread_mutex_unlock(&exit_mutex_rep);
}

/**
 * Waits until client has stopped (no more
 * callbacks).
 */
void AUDIO_IO_JACK_MANAGER::wait_for_stop(void)
{
  int ret = kvu_pthread_timed_wait(&stop_mutex_rep, &stop_cond_rep, 5);
  ECA_LOG_MSG(ECA_LOGGER::info, 
		kvu_pthread_timed_wait_result(ret, "(audioio_jack_manager) wait_for_stop"));
}
