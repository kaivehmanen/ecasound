#ifndef INCLUDED_AUDIOIO_PROXY_SERVER_IMPL_H
#define INCLUDED_AUDIOIO_PROXY_SERVER_IMPL_H

#include <pthread.h>

class AUDIO_IO_PROXY_SERVER_impl {

 public:

  friend AUDIO_IO_PROXY_SERVER;

 private:

  pthread_t io_thread_rep;
  pthread_cond_t full_cond_repp;
  pthread_mutex_t full_mutex_repp;
  pthread_cond_t stop_cond_repp;
  pthread_mutex_t stop_mutex_repp;
  pthread_cond_t flush_cond_repp;
  pthread_mutex_t flush_mutex_repp;

  size_t profile_sleep_rep;
  size_t profile_no_processing_rep;
  size_t profile_not_full_anymore_rep;
  size_t profile_processing_rep;
  size_t profile_read_xrun_danger_rep;
  size_t profile_write_xrun_danger_rep;
  size_t profile_rounds_total_rep;

};

#endif /* INCLUDED_AUDIOIO_PROXY_SERVER_IMPL_H */
