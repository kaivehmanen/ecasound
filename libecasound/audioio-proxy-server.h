#ifndef INCLUDED_AUDIOIO_PROXY_SERVER_H
#define INCLUDED_AUDIOIO_PROXY_SERVER_H

#include <map>
#include <pthread.h>
#include "audioio.h"
#include "audioio-proxy-buffer.h"

/**
 * Audio i/o engine. Meant for serving all proxied audio
 * objects (AUDIO_IO_BUFFERED_PROXY). 
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_PROXY_SERVER {

  friend void* start_proxy_server_io_thread(void *ptr);

 private:

  static const int buffercount_default;
  static const long int buffersize_default;

  std::vector<AUDIO_IO_PROXY_BUFFER*> buffers_rep;
  std::vector<AUDIO_IO*> clients_rep;
  std::map<AUDIO_IO*, int> client_map_rep;

  pthread_t io_thread_rep;
  pthread_cond_t full_cond_rep;
  pthread_mutex_t full_mutex_rep;
  pthread_cond_t stop_cond_rep;
  pthread_mutex_t stop_mutex_rep;
  pthread_cond_t flush_cond_rep;
  pthread_mutex_t flush_mutex_rep;
  bool thread_running_rep;

  ATOMIC_INTEGER exit_ok_rep;
  ATOMIC_INTEGER exit_request_rep;
  ATOMIC_INTEGER stop_request_rep;
  ATOMIC_INTEGER running_rep;
  ATOMIC_INTEGER full_rep;

  int buffercount_rep;
  long int buffersize_rep;
  long int samplerate_rep;
  int schedpriority_rep;

  size_t profile_not_full_set_rep;
  size_t profile_pauses_rep;
  size_t profile_full_and_active_rep;
  size_t profile_full_set_rep;
  size_t profile_not_full_rep;

  AUDIO_IO_PROXY_SERVER& operator=(const AUDIO_IO_PROXY_SERVER& x) { return *this; }
  AUDIO_IO_PROXY_SERVER (const AUDIO_IO_PROXY_SERVER& x) { }

  void io_thread(void);

  void signal_full(void);
  void signal_stop(void);
  void signal_flush(void);

  void dump_profile_counters(void);

 public:

  bool is_running(void) const;
  bool is_full(void) const;

  void start(void);
  void stop(void);
  void flush(void);

  void wait_for_full(void);
  void wait_for_stop(void);
  void wait_for_flush(void);

  void set_buffer_defaults(int buffers, long int buffersize, long int sample_rate);
  void set_schedpriority(int v) { schedpriority_rep = v; }

  void register_client(AUDIO_IO* abject);
  void unregister_client(AUDIO_IO* abject);
  AUDIO_IO_PROXY_BUFFER* get_client_buffer(AUDIO_IO* abject);

  AUDIO_IO_PROXY_SERVER (void); 
  ~AUDIO_IO_PROXY_SERVER(void);
};

#endif
