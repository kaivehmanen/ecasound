// ------------------------------------------------------------------------
// audioio_jack_manager.cpp: Manager for JACK client objects
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

#include <algorithm> /* std::count() */

#include <sys/time.h> /* gettimeofday() */
#include <errno.h> /* ETIMEDOUT */
#include <jack/jack.h>

#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>

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

#include "audioio_jack_manager.h"

static int eca_jack_process(nframes_t nframes, void *arg) {
  AUDIO_IO_JACK_MANAGER* current = static_cast<AUDIO_IO_JACK_MANAGER*>(arg);
  if (current->active_rep == true) {
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "process1 entry --> ");
    for(size_t n = 0; n < current->in_ports_rep.size(); n++) {
      sample_t* in_cb_buffer = static_cast<sample_t*>(jack_port_get_buffer(current->in_ports_rep[n], nframes));
      memcpy(current->in_cb_buffers_rep[n], in_cb_buffer, current->cb_nframes_rep * sizeof(sample_t));
    }
    current->cb_nframes_rep = static_cast<long int>(nframes);
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "process2");
    current->signal_token();
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "process3");
    if (current->wait_for_completion() != true) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) ecasound didn't return token; stopping!");
      // current->stop();
    }
    else {
      DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "process3 last node -- ");
      for(size_t n = 0; n < current->out_ports_rep.size(); n++) {
	DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "process3.6");
	sample_t* out_cb_buffer = static_cast<sample_t*>(jack_port_get_buffer(current->out_ports_rep[n], nframes));
	memcpy(out_cb_buffer, current->out_cb_buffers_rep[n], current->cb_nframes_rep * sizeof(sample_t));
      }
    }
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "process4 exit <--");
  }

  return(0);
}

static int eca_jack_bufsize (nframes_t nframes, void *arg) {
  AUDIO_IO_JACK_MANAGER* current = static_cast<AUDIO_IO_JACK_MANAGER*>(arg);
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) " +  current->jackname_rep + ": eca_jack_bufsize");

  // FIXME: !!!
  /***
      if (nframes != static_cast<nframes_t>(current->buffersize())) {
      ecadebug->msg(ECA_DEBUG::info, 
      "(audioio-jack-manager) " + current->jackname_rep + 
      ": warning! jackd and ecasound buffersizes don't match!");
      }
  ***/

  return(0);
}

static int eca_jack_srate (nframes_t nframes, void *arg) {
  AUDIO_IO_JACK_MANAGER* current = static_cast<AUDIO_IO_JACK_MANAGER*>(arg);

  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-jack-manager) " + current->jackname_rep + 
		": setting srate to " + kvu_numtostr(nframes));

  // FIXME: !!!
  /*
    current->set_samples_per_second(nframes);
  */

  return(0);
}

static void eca_jack_shutdown (void *arg) {
  AUDIO_IO_JACK_MANAGER* current = static_cast<AUDIO_IO_JACK_MANAGER*>(arg);
  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-jack-manager) " + current->jackname_rep + 
		": jackd shutdown, stopping processing");

  // FIXME: hmm, is this legal from a callback?
  // current->stop();
}

AUDIO_IO_JACK_MANAGER::AUDIO_IO_JACK_MANAGER(void)
{
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack-manager) constructor");

  total_nodes_rep = 0;
  active_nodes_rep = 0;

  last_in_port_rep = 0;
  last_out_port_rep = 0;

  active_rep = false;
  open_rep = false;

  node_callback_counter_rep = 0;
  total_nodes_rep = 0;

  last_id_rep = 1;
  jackname_rep = "ecasound"; // FIXME: !!!

  pthread_cond_init(&token_cond_rep, NULL);
  pthread_mutex_init(&token_mutex_rep, NULL);
  pthread_cond_init(&completion_cond_rep, NULL);
  pthread_mutex_init(&completion_mutex_rep, NULL);

  cb_nframes_rep = 0;
  cb_allocated_frames_rep = 0;
}

AUDIO_IO_JACK_MANAGER::~AUDIO_IO_JACK_MANAGER(void)
{
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack-manager) destructor");

  std::list<int>::iterator p = objlist_rep.begin();
  while(p != objlist_rep.end()) {
    jack_node_t* tmp = jacknodemap_rep[*p];
    delete tmp->aobj;
    jacknodemap_rep.erase(*p);
    ++p;
  }

  std::vector<sample_t*>::iterator q = in_cb_buffers_rep.begin();
  while(q != in_cb_buffers_rep.end()) {
    delete[] *q;
    *q = 0;
    ++q;
  }

  std::vector<sample_t*>::iterator r = out_cb_buffers_rep.begin();
  while(r != out_cb_buffers_rep.end()) {
    delete[] *r;
    *r = 0;
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

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) register object " + aobj->label());  
  objlist_rep.push_back(last_id_rep);
  AUDIO_IO_JACK* jobj = static_cast<AUDIO_IO_JACK*>(aobj);
  jack_node_t* tmp = new jack_node_t;
  tmp->aobj = jobj;
  tmp->origptr = aobj;
  tmp->in_ports = tmp->out_ports = 0;
  tmp->first_in_port = tmp->first_out_port = 0;
  jacknodemap_rep[last_id_rep] = tmp;
  jobj->set_manager(this, last_id_rep);

  if (open_rep == true) {
    close(last_id_rep);
    open(last_id_rep);
  }

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

  std::map<int,jack_node_t*>::const_iterator p = jacknodemap_rep.begin();
  while(p != jacknodemap_rep.end()) {
    std::cerr << "Comparing " << p->second->aobj 
	      << " to " << aobj << std::endl;
               
    if (p->second->origptr == aobj) {
      ecadebug->msg(ECA_DEBUG::system_objects, 
		    "(audioio-jack-manager) found object id for aobj " +
		    aobj->name() + ": " + kvu_numtostr(p->first));
      return(p->first);
    }
    ++p;
  }

  return(-1);
}

const std::list<int>& AUDIO_IO_JACK_MANAGER::get_object_list(void) const
{
  return(objlist_rep);
}

void AUDIO_IO_JACK_MANAGER::unregister_object(int id)
{
  // ---
  DBC_DECLARE(int old_total_nodes = total_nodes_rep);
  // ---

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) unregister object ");

  objlist_rep.remove(id);

  std::map<int,jack_node_t*>::iterator p = jacknodemap_rep.begin();
  while(p != jacknodemap_rep.end()) {
    if (p->first == id) {
      ecadebug->msg(ECA_DEBUG::system_objects,
		    "(audioio-jack-manager) removing object " + p->second->aobj->label());
      p->second->aobj->set_manager(0, -1);
      if (p->second->aobj->is_running() != true) --active_nodes_rep;
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

/**
 * Sets up automatic port connection for client_id's port
 * 'portnum'. When jack client is activated, this port
 * is automatically connected to port 'portname'. The 
 * direction of the connection is based on audio objects I/O mode 
 * (@see AUDIO_IO::io_mode()).
 *
 * @pre std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1
 * @pre open_rep == true
 @ @pre portnum > 0
 */
void AUDIO_IO_JACK_MANAGER::auto_connect_jack_port(int client_id, int portnum, const string& portname)
{
  // ---
  DBC_REQUIRE(std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1);
  DBC_REQUIRE(open_rep == true);
  DBC_REQUIRE(portnum > 0);
  // ---

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) auto-connect jack ports for client " + kvu_numtostr(client_id));

  jack_node_t* node = jacknodemap_rep[client_id];
  if (node->aobj->io_mode() == AUDIO_IO::io_read) {
    int index = node->first_in_port + portnum - 1;
    in_port_autoconnect_rep[index] = portname;
  }
  else {
    int index = node->first_out_port + portnum - 1;
    out_port_autoconnect_rep[index] = portname;
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
 * @pre open_rep == true
 */
void AUDIO_IO_JACK_MANAGER::register_jack_ports(int client_id, int ports, const std::string& portprefix)
{
  // ---
  DBC_REQUIRE(std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1);
  DBC_REQUIRE(open_rep == true);
  // ---

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) register jack ports for client " + kvu_numtostr(client_id));

  cb_nframes_rep = 0;

  jack_node_t* node = jacknodemap_rep[client_id];
  if (node->aobj->io_mode() == AUDIO_IO::io_read) {
    node->first_in_port = last_in_port_rep;
    node->first_out_port = 0;
    node->in_ports = node->out_ports = 0;
    for(int n = 0; n < ports; n++) {
      node->in_ports++;
      in_ports_rep.push_back(0);
      in_cb_buffers_rep.push_back(new sample_t [cb_allocated_frames_rep]);
      in_port_autoconnect_rep.push_back("");
      std::string tport = portprefix + "_" + kvu_numtostr(last_in_port_rep + 1);
      in_ports_rep[last_in_port_rep] = jack_port_register(client_repp, tport.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
      ++last_in_port_rep;
    }
    DBC_CHECK(in_ports_rep.size() == in_cb_buffers_rep.size());
    DBC_CHECK(in_ports_rep.size() == static_cast<size_t>(last_in_port_rep));
    DBC_CHECK(in_ports_rep.size() == in_port_autoconnect_rep.size());
  }
  else {
    node->first_out_port = last_out_port_rep;
    node->first_in_port = 0;
    node->in_ports = node->out_ports = 0;
    for(int n = 0; n < ports; n++) {
      node->out_ports++;
      out_ports_rep.push_back(0);
      out_cb_buffers_rep.push_back(new sample_t [cb_allocated_frames_rep]);
      out_port_autoconnect_rep.push_back("");
      std::string tport = portprefix + "_" + kvu_numtostr(last_out_port_rep + 1);
      out_ports_rep[last_out_port_rep] = jack_port_register(client_repp, tport.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
      ++last_out_port_rep;
    }
    DBC_CHECK(out_ports_rep.size() == out_cb_buffers_rep.size());
    DBC_CHECK(out_ports_rep.size() == static_cast<size_t>(last_out_port_rep));
    DBC_CHECK(out_ports_rep.size() == out_port_autoconnect_rep.size());
  }
  
  // ---
  DBC_ENSURE(node->in_ports == 0 || node->out_ports == 0);
  // ---
}

/**
 * Unregisters all JACK ports for client 'client_id'.
 *
 * @pre std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1
 * @post node->in_ports == 0 && node->out_ports == 0
 */
void AUDIO_IO_JACK_MANAGER::unregister_jack_ports(int client_id)
{
  // ---
  DBC_REQUIRE(std::count(get_object_list().begin(), get_object_list().end(), client_id) == 1);
  // ---

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) unregister all jack ports for client " + kvu_numtostr(client_id));

  jack_node_t* node = jacknodemap_rep[client_id];
  int portcount = node->in_ports + node->out_ports;

  // FIXME: add code for unregistering all ports

  // ---
  DBC_ENSURE(node->in_ports == 0 && node->out_ports == 0);
  // ---
}

void AUDIO_IO_JACK_MANAGER::open(int client_id)
{
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) open for client " + kvu_numtostr(client_id));

  /* only for the first client */
  if (open_rep != true) {
    /* connect to server - FIXME: add configurable name */
    if ((client_repp = jack_client_new ("ecasound")) == 0) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) Error! Cannot connect to JACK server!");
      open_rep = false;
    }
    else {
      /* set callbacks */
      jack_set_process_callback(client_repp, eca_jack_process, static_cast<void*>(this));
      jack_set_buffer_size_callback(client_repp, eca_jack_bufsize, static_cast<void*>(this));
      jack_set_sample_rate_callback(client_repp, eca_jack_srate, static_cast<void*>(this));
      jack_on_shutdown(client_repp, eca_jack_shutdown, static_cast<void*>(this));
      
      open_rep = true;

      ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack-manager) engine sample rate: " + kvu_numtostr(jack_get_sample_rate(client_repp)));
      cb_allocated_frames_rep = static_cast<long int>(jack_get_buffer_size(client_repp));
      ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack-manager) engine buffersize: " + kvu_numtostr(cb_allocated_frames_rep));      
    }
  }
}

void AUDIO_IO_JACK_MANAGER::close(int client_id)
{
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) close for client " + kvu_numtostr(client_id));

  /* only for the first client */
  if (open_rep == true) {
    // FIXME: add proper unregistration
    // iterate over cids: unregister_jack_ports()

    jack_client_close (client_repp);
    open_rep = false;
  }
}

/**
 * Executed from all read_samples() and write_samples() calls.
 */
void AUDIO_IO_JACK_MANAGER::node_control_entry(void) {
  if (active_nodes_rep > 0 && node_callback_counter_rep == 0) {
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "entry1");
    if (wait_for_token() != true) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) token not received from jackd; stopping!");
      // FIXME: !!!
      /**
	 stop();
      **/
    }
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "entry2");
  }
}

/**
 * Executed from all read_samples() and write_samples() calls.
 */
void AUDIO_IO_JACK_MANAGER::node_control_exit(void) {
  ++node_callback_counter_rep;
  
  DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "exit1");
  if (node_callback_counter_rep == active_nodes_rep) {
    signal_completion();
    DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "exit2");
    node_callback_counter_rep = 0;
  }
}

long int AUDIO_IO_JACK_MANAGER::read_samples(int client_id, void* target_buffer, long int samples)
{
  node_control_entry();

  DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "read1");
  sample_t* ptr = static_cast<sample_t*>(target_buffer);
  jack_node_t* node = jacknodemap_rep[client_id];
  for(int n = node->first_in_port; n < node->first_in_port + node->in_ports; n++) {
    DBC_CHECK(n < static_cast<int>(in_cb_buffers_rep.size()));
    memcpy(ptr, in_cb_buffers_rep[n], cb_nframes_rep * sizeof(sample_t)); // FIXME: was frame_size()
    ptr += cb_nframes_rep;
  }
  DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "read2");

  node_control_exit();

  return(cb_nframes_rep);
}

void AUDIO_IO_JACK_MANAGER::write_samples(int client_id, void* target_buffer, long int samples)
{
  node_control_entry();

  DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "write1");
  sample_t* ptr = static_cast<sample_t*>(target_buffer);
  jack_node_t* node = jacknodemap_rep[client_id];
  for(int n = node->first_out_port; n < node->first_out_port + node->out_ports; n++) {
    DBC_CHECK(n < static_cast<int>(out_cb_buffers_rep.size()));
    memcpy(out_cb_buffers_rep[n], ptr, cb_nframes_rep * sizeof(sample_t)); // FIXME: was frame_size()
    ptr += cb_nframes_rep;
  }
  DEBUG_CFLOW_STATEMENT(std::cerr << std::endl << "write2");

  node_control_exit();
}

void AUDIO_IO_JACK_MANAGER::start(int client_id)
{
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) start for client " + kvu_numtostr(client_id));

  if (active_rep != true) {
    if (jack_activate (client_repp)) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) Error! Cannot active client!");
    }
    active_rep = true;
  }

  jack_node_t* node = jacknodemap_rep[client_id];
  connect_node(node);
}

void AUDIO_IO_JACK_MANAGER::stop(int client_id)
{
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack-manager) stop for client " + kvu_numtostr(client_id));

  jack_node_t* node = jacknodemap_rep[client_id];
  disconnect_node(node);

  if (active_rep != false) {
    if (jack_deactivate (client_repp)) {
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) Error! Cannot deactive client!");
    }
    active_rep = false;
  }
}

void AUDIO_IO_JACK_MANAGER::connect_node(jack_node_t* node) { 
  if (node->aobj->io_mode() == AUDIO_IO::io_read) {
    for(int n = node->first_in_port; n < node->first_in_port + node->in_ports; n++) {
      std::string tport = in_port_autoconnect_rep[n];
      if (tport.size() == 0) {
	tport = "ALSA I/O:Input " + kvu_numtostr(n + 1);
      }
      if (jack_port_connect (client_repp, tport.c_str(), jack_port_name(in_ports_rep[n]))) {
	ecadebug->msg(ECA_DEBUG::info, 
		      "(audioio-jack-manager) Error! Cannot connect to input " + tport);
      }
    }
  }
  else {
    for(int n = node->first_out_port; n < node->first_out_port + node->out_ports; n++) {
      std::string tport = out_port_autoconnect_rep[n];
      if (tport.size() == 0) {
	tport = "ALSA I/O:Output " + kvu_numtostr(n + 1);
      }
      if (jack_port_connect (client_repp, jack_port_name(out_ports_rep[n]), tport.c_str())) {
	ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) Error! Cannot connect to output " + tport);
      }
    }
  }
  
  ++active_nodes_rep;
}

void AUDIO_IO_JACK_MANAGER::disconnect_node(jack_node_t* node)
{
  --active_nodes_rep;
}


/**
 * Signals that callback token is received.
 */
void AUDIO_IO_JACK_MANAGER::signal_token(void) {
  pthread_mutex_lock(&token_mutex_rep);
  token_rep = true;
  pthread_cond_broadcast(&token_cond_rep);
  pthread_mutex_unlock(&token_mutex_rep);
}

/**
 * Waits until token is received from 
 * the callback.
 * 
 * @return true if token was succesfully received
 */
bool AUDIO_IO_JACK_MANAGER::wait_for_token(void) {
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

  if (ret < 0) {
    if (ret == -ETIMEDOUT)
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) wait_for_token failed; timeout");
    else
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) wait_for_token failed");

    return(false);
  }

  return(true);
}

/**
 * Signals that processing of callback token 
 * has been completed.
 */
void AUDIO_IO_JACK_MANAGER::signal_completion(void) {
  pthread_mutex_lock(&completion_mutex_rep);
  completion_rep = true;
  pthread_cond_broadcast(&completion_cond_rep);
  pthread_mutex_unlock(&completion_mutex_rep);
}

/**
 * Waits until token processing is completed.
 * 
 * @return true if completion event was succesfully received
 */
bool AUDIO_IO_JACK_MANAGER::wait_for_completion(void) {
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

  if (ret < 0) {
    if (ret == -ETIMEDOUT)
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) wait_for_completion failed; timeout");
    else
      ecadebug->msg(ECA_DEBUG::info, "(audioio-jack-manager) wait_for_completion failed");
    
    return(false);
  }
  
  return(true);
}
