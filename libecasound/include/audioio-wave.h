#ifndef _AUDIOIO_WAVE_H
#define _AUDIOIO_WAVE_H

#include <string>
#include <sys/types.h>

#include "audioio-types.h"
#include "samplebuffer.h"
#include "eca-fileio.h"

typedef struct {
    u_int16_t format;
    u_int16_t channels;
    u_int32_t srate;
    u_int32_t byte_second;
    u_int16_t align;
    u_int16_t bits;
} RF;

typedef struct {
    u_int8_t sig[4];
    u_int32_t bsize;
} RB;

typedef struct {
    u_int8_t id[4];
    u_int32_t size;
    u_int8_t wname[4];
} RH;

/**
 *
 * Represents a RIFF WAVE -file (.wav)
 *
 * This class currently supports only a limited set of features:
 *
 * Format 1 sample data: Pulse Code Modulation (PCM) Format
 *
 * - multiple channels are interleaved
 *
 * - only 8 and 16 bit data supported
 * 
 * - if 16 bits, least significant byte first as specified
 *   in the stantard
 */
class WAVEFILE : public AUDIO_IO_FILE {

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
  void format_query(void);
    
 public:

  void open(void);
  void close(void);

  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  bool finished(void) const;
  void seek_position(void);

  WAVEFILE (const string& name,
	    const SIMODE mode,
	    const ECA_AUDIO_FORMAT& form,
	    bool double_buffering = false);
  WAVEFILE::~WAVEFILE(void);
  WAVEFILE* clone(void) { return new WAVEFILE(*this); }

 private:

  //  WAVEFILE(const WAVEFILE& x) { cerr << "WAVE construct"; }
  WAVEFILE& operator=(const WAVEFILE& x) {  return(*this); }

  void update(void);        
  void set_length_in_bytes(void);
  void read_riff_header (void);
  bool next_riff_block(RB *t, unsigned long int *offtmp);
  void read_riff_fmt(void);
  void write_riff_header (void);
  void write_riff_fmt(void);
  void write_riff_datablock(void);
  void update_riff_datablock(void);
  void find_riff_datablock (void);
  signed long int find_block(const char* fblock);
};

#endif
