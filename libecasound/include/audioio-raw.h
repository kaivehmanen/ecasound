#ifndef _AUDIOIO_RAW_H
#define _AUDIOIO_RAW_H

#include <string>

#include "eca-fileio.h"
#include "eca-fileio-mmap.h"
#include "eca-fileio-stream.h"

class SAMPLE_BUFFER;

/**
 * Class for handling raw/headerless audio files
 *
 * - multiple channels are interleaved, left channel first
 *
 * @author Kai Vehmanen
 */
class RAWFILE : public AUDIO_IO_BUFFERED {

  ECA_FILE_IO* fio;
  bool double_buffering_rep;

  //  RAWFILE(const RAWFILE& x) { }
  RAWFILE& operator=(const RAWFILE& x) { return *this; }

  void set_length_in_bytes(void);

  /**
   * Do a info query prior to actually opening the device.
   *
   * require:
   *  !is_open()
   *
   * ensure:
   *  !is_open()
   */
  void format_query(void);

 public:

  string name(void) const { return("Raw audio file"); }

  void open(void);
  void close(void);

  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  bool finished(void) const;
  void seek_position(void);

  RAWFILE (const string& name = "", bool double_buffering = false);
  ~RAWFILE(void);
    
  RAWFILE* clone(void) { return new RAWFILE(*this); }    
  RAWFILE* new_expr(void) { return new RAWFILE(); }    
};

#endif









