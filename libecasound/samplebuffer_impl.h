#ifndef INCLUDED_SAMPLEBUFFER_IMPL_H
#define INCLUDED_SAMPLEBUFFER_IMPL_H

#include "samplebuffer.h" 

class SAMPLE_BUFFER_impl {

 public:

  friend class SAMPLE_BUFFER;

 private:

  /** @name Misc member variables */
  /*@{*/
  
  bool rt_lock_rep;
  int lockref_rep;

  SAMPLE_BUFFER::sample_t* old_buffer_repp; // for resampling
  std::vector<SAMPLE_BUFFER::sample_t> resample_memory_rep;

  /*@}*/
};

#endif
