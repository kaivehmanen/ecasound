#ifndef INCLUDED_AUDIOIO_RTNULL_H
#define INCLUDED_AUDIOIO_RTNULL_H

#include <sys/time.h>
#include "audioio-types.h"

/**
 * Null audio object with realtime behaviour
 */
class REALTIME_NULL : public AUDIO_IO_DEVICE {
 public:

  virtual std::string name(void) const { return("Realtime null device"); }

  virtual void open(void) throw (AUDIO_IO::SETUP_ERROR &);
  virtual void close(void);

  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);
  virtual void prepare(void);

  virtual long int latency(void) const;

  virtual long position_in_samples(void) const;

  REALTIME_NULL(const std::string& name = "realtime null");
  virtual ~REALTIME_NULL(void);
  REALTIME_NULL* clone(void) { return new REALTIME_NULL(*this); }
  REALTIME_NULL* new_expr(void) { return new REALTIME_NULL(); }

 private:

  struct timeval start_time;
  struct timeval access_time;
  struct timeval buffer_delay;
  struct timeval buffer_fill;
};

#endif
