#ifndef INCLUDED_AUDIOIO_JACK_H
#define INCLUDED_AUDIOIO_JACK_H

#include <string>
#include <vector>

#include <pthread.h>

#include "audioio-types.h"
#include "sample-specs.h"

/**
 * Interface to JACK audio framework.
 *
 * @author Kai Vehmanen
 */
class JACK_INTERFACE : public AUDIO_IO_DEVICE {

 public:

  friend int eca_jack_process(nframes_t nframes, void *arg);
  friend int eca_jack_bufsize (nframes_t nframes, void *arg);
  friend int eca_jack_srate (nframes_t nframes, void *arg);
  friend void eca_jack_shutdown (void *arg);

 public:

  virtual string name(void) const { return("JACK interface"); }
  virtual string description(void) const { return(name()); }
  virtual string parameter_names(void) const { return("label,jackname"); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual bool locked_audio_format(void) const { return(true); }

  JACK_INTERFACE (void);
  ~JACK_INTERFACE(void);
  
  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);
  virtual void prepare(void);

  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const;

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;
    
  JACK_INTERFACE* clone(void) const { return new JACK_INTERFACE(*this); }
  JACK_INTERFACE* new_expr(void) const { return new JACK_INTERFACE(); }  

 private:

  pthread_cond_t token_cond_rep;
  pthread_mutex_t token_mutex_rep;
  pthread_cond_t completion_cond_rep;
  pthread_mutex_t completion_mutex_rep;
  bool token_rep;
  bool completion_rep;

  jack_client_t *client_repp;
  std::vector<jack_port_t*> ports_rep;
  std::vector<sample_t*> portbufs_rep;
  std::vector<void*> cb_buffers_rep;
  long int cb_nframes_rep;
  std::string jackname_rep;
  SAMPLE_SPECS::sample_pos_t curpos_rep;

 private:

  void connect_ports(void);
  void disconnect_ports(void);

  void wait_for_token(void);
  void signal_token(void);
  void wait_for_completion(void);
  void signal_completion(void);

  JACK_INTERFACE (const JACK_INTERFACE& x) { }
  JACK_INTERFACE& operator=(const JACK_INTERFACE& x) {  return *this; }
};

extern "C" {
AUDIO_IO* audio_io_descriptor(void) { return(new JACK_INTERFACE()); }
int audio_io_interface_version(void);
const char* audio_io_keyword(void);
const char* audio_io_keyword_regex(void);
};

#endif
