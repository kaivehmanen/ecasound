#ifndef INCLUDED_FILEIO_STREAM_H
#define INCLUDED_FILEIO_STREAM_H

#include <string>
#include <cstdio>
#include "eca-fileio.h"


/**
 * File-I/O and buffering routines using normal file streams.
 */
class ECA_FILE_IO_STREAM : public ECA_FILE_IO {

 private:

  FILE *f1;
  off_t curpos_rep;
  off_t last_rep;

  std::string mode_rep;
  std::string fname_rep;
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
  void read_to_buffer(void* obuf, off_t bytes);
  void write_from_buffer(void* obuf, off_t bytes);

  void set_file_position(off_t newpos);
  void set_file_position_advance(off_t fw);
  void set_file_position_end(void);
  off_t get_file_position(void) const;
  off_t get_file_length(void) const;

  // --
  // Status
  // ---
  bool is_file_ready(void) const;
  bool is_file_error(void) const;
  off_t file_bytes_processed(void) const;
  const std::string& file_mode(void) const { return(mode_rep); }

  ECA_FILE_IO_STREAM (void) { }
  ~ECA_FILE_IO_STREAM(void) { if (mode_rep != "") close_file(); }
};

#endif
