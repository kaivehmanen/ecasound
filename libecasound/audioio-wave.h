#ifndef INCLUDED_AUDIOIO_WAVE_H
#define INCLUDED_AUDIOIO_WAVE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#define HAVE_INTTYPES_H
#endif

#include <string>
#include <iostream>
/* ANSI/ISO type defs: */
#ifdef HAVE_STDINT_H
#include <stdint.h> /* uint32_t, etc types */
#else
  #ifdef HAVE_INTTYPES_H
  #include <inttypes.h> /* uint32_t, etc types */
  #endif
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* uint32_t, etc types */
#endif
#include "audioio-types.h"
#include "samplebuffer.h"
#include "eca-fileio.h"

/**
 * Represents a RIFF WAVE -file (wav).
 *
 * This class currently supports only a limited set of features:
 *
 * Format 1 sample data: Pulse Code Modulation (PCM) Format
 *
 * Format 3 sample data: IEEE754 floats, range [-1, +1)

 * - multiple channels are interleaved
 *
 * - 8, 16, 24 and 32 bit data supported
 * 
 * - if more than 8 bits, least significant byte first as specified
 *   in the stantard
 */
class WAVEFILE : public AUDIO_IO_BUFFERED {

 public:

  typedef struct {
    uint16_t format;
    uint16_t channels;
    uint32_t srate;
    uint32_t byte_second;
    uint16_t align;
    uint16_t bits;
  } RF;

  typedef struct {
    uint8_t sig[4];
    uint32_t bsize;
  } RB;

  typedef struct {
    uint8_t id[4];
    uint32_t size;
    uint8_t wname[4];
  } RH;

 private:

  ECA_FILE_IO* fio_repp;

  RH riff_header_rep;
  RF riff_format_rep;

  long int data_start_position_rep;
  std::string mmaptoggle_rep;

  /**
   * Do a info query prior to actually opening the device.
   *
   * require:
   *  !is_open()
   *
   * ensure:
   *  !is_open()
   *  fio == 0
   */
  void format_query(void) throw(AUDIO_IO::SETUP_ERROR&);

  enum Format_tags {
    unknown		= (0x0000),
    pcm			= (0x0001), 
    adpcm		= (0x0002),
    ieee_float          = (0x0003),
    alaw		= (0x0006),
    mulaw		= (0x0007),
    oki_adpcm		= (0x0010),
    ima_adpcm		= (0x0011),
    digistd		= (0x0015),
    digifix		= (0x0016),
    dolby_ac2           = (0x0030),
    gsm610              = (0x0031),
    rockwell_adpcm      = (0x003b),
    rockwell_digitalk   = (0x003c),
    g721_adpcm          = (0x0040),
    g728_celp           = (0x0041),
    mpeg                = (0x0050),
    mpeglayer3          = (0x0055),
    g726_adpcm          = (0x0064),
    g722_adpcm          = (0x0065)
  };
    
 public:

  virtual std::string name(void) const { return("RIFF wave file"); }
  virtual bool locked_audio_format(void) const { return(true); }
  virtual std::string parameter_names(void) const { return("label,toggle_mmap"); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR &);
  virtual void close(void);

  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual bool finished(void) const;
  virtual void seek_position(void);

  virtual void set_parameter(int param, std::string value);
  virtual std::string get_parameter(int param) const;

  WAVEFILE (const std::string& name = "");
  ~WAVEFILE(void);
  WAVEFILE* clone(void) const { std::cerr << "Not implemented!" << std::endl; return 0; }
  WAVEFILE* new_expr(void) const { return new WAVEFILE(); }

 private:

  WAVEFILE(const WAVEFILE& x) { std::cerr << "WAVE construct"; }
  WAVEFILE& operator=(const WAVEFILE& x) {  return(*this); }

  void update(void);        
  void set_length_in_bytes(void);
  void read_riff_header (void) throw(AUDIO_IO::SETUP_ERROR&);
  bool next_riff_block(RB *t, off_t *offtmp);
  void read_riff_fmt(void) throw(AUDIO_IO::SETUP_ERROR&);
  void write_riff_header (void) throw(AUDIO_IO::SETUP_ERROR&);
  void write_riff_fmt(void);
  void write_riff_datablock(void);
  void update_riff_datablock(void);
  void find_riff_datablock (void) throw(AUDIO_IO::SETUP_ERROR&);
  bool find_block(const char* fblock);
};

#endif
