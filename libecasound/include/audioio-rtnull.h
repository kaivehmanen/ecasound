#ifndef _AUDIOIO_RTNULL_H
#define _AUDIOIO_RTNULL_H

#include <sys/time.h>
#include "audioio-types.h"

/**
 * Null audio object with realtime behaviour
 */
class REALTIME_NULL : public AUDIO_IO_DEVICE {
 public:

  string name(void) const { return("Realtime null device"); }

  void open(void);
  void close(void);

  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  void stop(void);
  void start(void);
  void prepare(void) { }

  long int latency(void) const;

  long position_in_samples(void) const;

  REALTIME_NULL(const string& name = "realtime null");
  ~REALTIME_NULL(void);
  REALTIME_NULL* clone(void) { return new REALTIME_NULL(*this); }
  REALTIME_NULL* new_expr(void) { return new REALTIME_NULL(); }

 private:

  bool is_triggered;
  struct timeval start_time;
  struct timeval access_time;
  struct timeval buffer_delay;
  struct timeval buffer_fill;
};

#endif
