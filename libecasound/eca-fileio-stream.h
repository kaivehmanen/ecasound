#ifndef INCLUDED_FILEIO_STREAM_H
#define INCLUDED_FILEIO_STREAM_H

#include <cstdio>
#include "eca-fileio.h"

/**
 * File-I/O and buffering routines using normal file streams.
 */
class ECA_FILE_IO_STREAM : public ECA_FILE_IO {

 private:

  FILE *f1;
  fpos_t bytes_rep;

  std::string mode_rep;
  bool standard_mode;
   
 public:


  // --
  // Open/close routines
  // ---
  void open_file(const std::string& fname, const std::string& fmode);
  void open_stdin(void);
  void open_stdout(void);
  void open_stderr(void);
  void close_file(void);

  // --
  // Normal file operations
  // ---
  void read_to_buffer(void* obuf, fpos_t bytes);
  void write_from_buffer(void* obuf, fpos_t bytes);

  void set_file_position(fpos_t newpos);
  void set_file_position_advance(fpos_t fw);
  void set_file_position_end(void);
  fpos_t get_file_position(void) const;
  fpos_t get_file_length(void) const;

  // --
  // Status
  // ---
  bool is_file_ready(void) const;
  bool is_file_error(void) const;
  fpos_t file_bytes_processed(void) const;
  const std::string& file_mode(void) const { return(mode_rep); }

  ECA_FILE_IO_STREAM (void) { }
  ~ECA_FILE_IO_STREAM(void) { if (mode_rep != "") close_file(); }
};

#endif
