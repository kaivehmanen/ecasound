#ifndef _AUDIOIO_WAVE_H
#define _AUDIOIO_WAVE_H

#include <string>
#include <inttypes.h>
#include <sys/types.h>

#include "audioio-types.h"
#include "samplebuffer.h"
#include "eca-fileio.h"

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

/**
 * Represents a RIFF WAVE -file (wav).
 *
 * This class currently supports only a limited set of features:
 *
 * Format 1 sample data: Pulse Code Modulation (PCM) Format
 *
 * - multiple channels are interleaved
 *
 * - 8, 16, 24 and 32 bit data supported
 * 
 * - if more than 8 bits, least significant byte first as specified
 *   in the stantard
 */
class WAVEFILE : public AUDIO_IO_BUFFERED {

  ECA_FILE_IO* fio;

  RH riff_header;
  RF riff_format;

  long int data_start_position;
  bool double_buffering_rep;

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
  void format_query(void) throw(ECA_ERROR*);
    
 public:

  virtual string name(void) const { return("RIFF wave file"); }
  virtual bool locked_audio_format(void) const { return(true); }

  virtual void open(void) throw(ECA_ERROR*);
  virtual void close(void);

  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual bool finished(void) const;
  virtual void seek_position(void);

  WAVEFILE (const string& name = "", bool double_buffering = false);
  ~WAVEFILE(void);
  WAVEFILE* clone(void) { cerr << "Not implemented!" << endl; return this; }
  WAVEFILE* new_expr(void) { return new WAVEFILE(); }

 private:

  WAVEFILE(const WAVEFILE& x) { cerr << "WAVE construct"; }
  WAVEFILE& operator=(const WAVEFILE& x) {  return(*this); }

  void update(void);        
  void set_length_in_bytes(void);
  void read_riff_header (void) throw(ECA_ERROR*);
  bool next_riff_block(RB *t, unsigned long int *offtmp);
  void read_riff_fmt(void) throw(ECA_ERROR*);
  void write_riff_header (void) throw(ECA_ERROR*);
  void write_riff_fmt(void);
  void write_riff_datablock(void);
  void update_riff_datablock(void);
  void find_riff_datablock (void) throw(ECA_ERROR*);
  signed long int find_block(const char* fblock);
};

#endif
