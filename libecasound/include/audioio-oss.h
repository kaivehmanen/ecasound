#ifndef _AUDIOIO_OSS_H
#define _AUDIOIO_OSS_H

#include <config.h>
#ifdef COMPILE_OSS

#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#include "samplebuffer.h"

/**
 * Class for handling Open Sound System -devices (OSS/Linux 
 * and OSS/Lite).
 * @author Kai Vehmanen
 */
class OSSDEVICE : public AUDIO_IO_DEVICE {

  int audio_fd;
  
  audio_buf_info audiobuf;          // soundcard.h
  count_info audioinfo;             // soundcard.h
  fd_set fds;

  int fragment_size;
  long int bytes_read;
  int oss_caps;
  struct timeval start_time;
  
  bool is_triggered;
  bool precise_srate_mode;
  
 public:

  virtual string name(void) const { return("OSS soundcard device"); }
  virtual string description(void) const { return("Open Sound System -devices (OSS/Linux and OSS/Free)."); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }

  virtual void open(void) throw(ECA_ERROR*);
  virtual void close(void) throw(ECA_ERROR*);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void) throw(ECA_ERROR*);
  virtual void prepare(void) { }

  virtual long position_in_samples(void) const;

  OSSDEVICE (const string& name = "/dev/dsp", bool precise_sample_rates = false);
  ~OSSDEVICE(void);
  OSSDEVICE* clone(void) { cerr << "Not implemented!" << endl; return this; }
  OSSDEVICE* new_expr(void) { return new OSSDEVICE(); }

 private:
  
  OSSDEVICE(const OSSDEVICE& x) { }
  OSSDEVICE& operator=(const OSSDEVICE& x) { return *this; }    

};

#endif // COMPILE_OSS
#endif
