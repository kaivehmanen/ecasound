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

  vector<AUDIO_IO_PROXY_BUFFER> buffers_rep;
  vector<AUDIO_IO*> clients_rep;
  map<AUDIO_IO*, int> client_map_rep;
  pthread_t io_thread_rep;
  bool running_rep;
  int buffercount_rep;
  long int buffersize_rep;

  AUDIO_IO_PROXY_SERVER& operator=(const AUDIO_IO_PROXY_SERVER& x) { return *this; }
  AUDIO_IO_PROXY_SERVER (const AUDIO_IO_PROXY_SERVER& x) { }

  void io_thread(void);

 public:

  bool is_running(void) const;

  void start(void);
  void stop(void);
  void seek(AUDIO_IO* aobject, long int position_in_samples);

  void set_buffer_defaults(int buffers, long int buffersize);
  void register_client(AUDIO_IO* abject);
  void unregister_client(AUDIO_IO* abject);
  AUDIO_IO_PROXY_BUFFER* get_client_buffer(AUDIO_IO* abject);

  AUDIO_IO_PROXY_SERVER (void); 
  ~AUDIO_IO_PROXY_SERVER(void);
};

#endif
