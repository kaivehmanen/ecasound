#ifndef _FILEIO_MMAP_H
#define _FILEIO_MMAP_H

#include "eca-fileio.h"

/**
 * File-io and buffering using  mmap for data transfers.
 */
class ECA_FILE_IO_MMAP : public ECA_FILE_IO {

 private:

  int f1;
  unsigned char* internal_buffer;
  long int internal_bsize;
  long int bytes_rep;
  long int fposition, flength;
  long int fmaxbsize;

  bool file_ready;
  bool file_ended;
  string mode_rep;
   
 public:


  // --
  // Open/close routines
  // ---
  void open_file(const string& fname, 
		 const string& fmode, 
		 bool handle_errors = true);
  void open_stdin(void) { }
  void open_stdout(void) { }
  void close_file(void);

  // --
  // Normal file operations
  // ---
  void read_to_buffer(void* obuf, long int bytes);
  void write_from_buffer(void* obuf, long int bytes);

  void set_file_position(long int newpos) { set_file_position(newpos,true); }
  void set_file_position(long int newpos, bool seek);
  void set_file_position_advance(long int fw);
  void set_file_position_end(void);
  long int get_file_position(void) const;
  long int get_file_length(void) const;

  // --
  // Status
  // ---
  bool is_file_ready(void) const;
  bool is_file_error(void) const;
  bool is_file_ended(void) const;
  long int file_bytes_processed(void) const;
  const string& file_mode(void) const { return(mode_rep); }

  ECA_FILE_IO_MMAP(void) {
    internal_bsize = 0;
  }
  ~ECA_FILE_IO_MMAP(void);
};

#endif
