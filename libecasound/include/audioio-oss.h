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

  void open(void) throw(ECA_ERROR*);
  void close(void) throw(ECA_ERROR*);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  void stop(void);
  void start(void) throw(ECA_ERROR*);

  long position_in_samples(void) const;

  OSSDEVICE (const string& name, const SIMODE mode, const ECA_AUDIO_FORMAT& form, long int buffersize, bool precise_sample_rates = false);
  ~OSSDEVICE(void);
  OSSDEVICE* clone(void) { return new OSSDEVICE(*this); }

 private:
  
  //  OSSDEVICE(const OSSDEVICE& x) { }
  OSSDEVICE& operator=(const OSSDEVICE& x) { return *this; }    

};

#endif // COMPILE_OSS
#endif
