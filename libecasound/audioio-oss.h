#ifndef INCLUDED_AUDIOIO_OSS_H
#define INCLUDED_AUDIOIO_OSS_H

#include <string>
#include <iostream>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef COMPILE_OSS
#include <sys/soundcard.h>
#ifndef AFMT_S32_LE
#define AFMT_S32_LE              0x00001000
#endif
#ifndef AFMT_S32_BE
#define AFMT_S32_BE              0x00002000
#endif

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
  
  bool precise_srate_mode;
  
 public:

  virtual std::string name(void) const { return("OSS soundcard device"); }
  virtual std::string description(void) const { return("Open Sound System -devices (OSS/Linux and OSS/Free)."); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR &);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);

  virtual long position_in_samples(void) const;

  OSSDEVICE (const std::string& name = "/dev/dsp", bool precise_sample_rates = false);
  ~OSSDEVICE(void);
  OSSDEVICE* clone(void) const { std::cerr << "Not implemented!" << std::endl; return 0; }
  OSSDEVICE* new_expr(void) const { return new OSSDEVICE(); }

 private:
  
  OSSDEVICE(const OSSDEVICE& x) { }
  OSSDEVICE& operator=(const OSSDEVICE& x) { return *this; }    

};

#endif /* COMPILE_OSS */
#endif
