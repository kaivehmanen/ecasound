#ifndef _FILEIO_STREAM_H
#define _FILEIO_STREAM_H

#include "eca-fileio.h"

/**
 * File-I/O and buffering routines using normal file streams.
 */
class ECA_FILE_IO_STREAM : public ECA_FILE_IO {

 private:

  FILE *f1;
  long int bytes_rep;

  string mode_rep;
  bool standard_mode;
   
 public:


  // --
  // Open/close routines
  // ---
  void open_file(const string& fname, 
		 const string& fmode, 
		 bool handle_errors = true) throw(ECA_ERROR*);
  void open_stdin(void);
  void open_stdout(void);
  void close_file(void);

  // --
  // Normal file operations
  // ---
  void read_to_buffer(void* obuf, long int bytes);
  void write_from_buffer(void* obuf, long int bytes);

  void set_file_position(long int newpos);
  void set_file_position_advance(long int fw);
  void set_file_position_end(void);
  long int get_file_position(void) const;
  long int get_file_length(void) const;

  // --
  // Status
  // ---
  bool is_file_ready(void) const;
  bool is_file_error(void) const;
  long int file_bytes_processed(void) const;
  const string& file_mode(void) const { return(mode_rep); }

  ECA_FILE_IO_STREAM (void) { }
  ~ECA_FILE_IO_STREAM(void) { if (mode_rep != "") close_file(); }
};

#endif
