#ifndef INCLUDED_AUDIOIO_JACK_MANAGER_H
#define INCLUDED_AUDIOIO_JACK_MANAGER_H

#include <map>
#include <list>
#include <string>
#include <vector>

#include <pthread.h>
#include <jack/jack.h>

#include "audioio-manager.h"
#include "audioio_jack.h"

class AUDIO_IO;

/**
 * Manager class for JACK client objects.
 *
 * Related design patterns:
 *     - Mediator (GoF273)
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_JACK_MANAGER : public AUDIO_IO_MANAGER {

 public:

  friend int eca_jack_process(nframes_t nframes, void *arg);
  friend int eca_jack_bufsize (nframes_t nframes, void *arg);
  friend int eca_jack_srate (nframes_t nframes, void *arg);
  friend void eca_jack_shutdown (void *arg);

  static const int instance_limit;

public:

  typedef struct jack_node {
    AUDIO_IO_JACK* aobj;
    AUDIO_IO* origptr;
    int in_ports;
    int first_in_port;
    int out_ports;
    int first_out_port;
  } jack_node_t;

  typedef struct jack_port_data {
    jack_port_t* jackport;
    std::string autoconnect;
    bool autoconnect_addprefix;
    sample_t* cb_buffer;
  } jack_port_data_t;

 public:

  /** @name Constructors */
  /*@{*/

  AUDIO_IO_JACK_MANAGER(void);
  virtual ~AUDIO_IO_JACK_MANAGER(void);

  /** @name Public API */
  /*@{*/

  virtual std::string name(void) const { return("JACK object manager"); }
  virtual bool is_managed_type(const AUDIO_IO* aobj) const;
  virtual void register_object(AUDIO_IO* aobj);
  virtual int get_object_id(const AUDIO_IO* aobj) const;
  virtual const std::list<int>& get_object_list(void) const;
  virtual void unregister_object(int id);

  /*@}*/

  /** @name Public API for JACK clients */
  /*@{*/

  void register_jack_ports(int client_id, int ports, const std::string& portprefix);
  void unregister_jack_ports(int client_id);
  void auto_connect_jack_port(int client_id, int portnum, const string& portname);

  void open(int client_id, long int buffersize, long int samplerate);
  void close(int client_id);
  
  long int read_samples(int client_id, void* target_buffer, long int samples);
  void write_samples(int client_id, void* target_buffer, long int samples);

  void stop(int client_id);
  void start(int client_id);

  bool is_open(void) const { return(open_rep); }

  /*@}*/

private:

  void set_node_connection(jack_node_t* node, bool connect);
  void connect_node(jack_node_t* node);
  void disconnect_node(jack_node_t* node);

  void node_control_entry(void);
  void node_control_exit(void);

  bool wait_for_token(void);
  void signal_token(void);
  bool wait_for_completion(void);
  void signal_completion(void);

  pthread_cond_t token_cond_rep;
  pthread_mutex_t token_mutex_rep;
  pthread_cond_t completion_cond_rep;
  pthread_mutex_t completion_mutex_rep;
  bool token_rep;
  bool completion_rep;

  int total_nodes_rep;
  int active_nodes_rep;

  int last_in_port_rep;
  int last_out_port_rep;

  bool active_rep;
  bool open_rep;
  bool shutdown_request_rep;

  int node_callback_counter_rep;
 
  int last_id_rep;
  std::list<int> objlist_rep;
  std::map<int,jack_node_t*> jacknodemap_rep;

  jack_client_t *client_repp;
  std::string jackname_rep;

  std::vector<jack_port_data_t> inports_rep;
  std::vector<jack_port_data_t> outports_rep;

  long int samplerate_rep;
  long int cb_nframes_rep;
  long int cb_allocated_frames_rep;
};

#endif
