#ifndef INCLUDED_FILEIO_MMAP_H
#define INCLUDED_FILEIO_MMAP_H

#include <sys/types.h>

#include "eca-fileio.h"

#ifndef MAP_FAILED
#define MAP_FAILED	((__ptr_t) -1)
#endif

#if !defined _CADDR_T && !defined __USE_BSD
#define _CADDR_T
typedef char* caddr_t;
#endif

/**
 * File-io and buffering using mmap for data transfers.
 */
class ECA_FILE_IO_MMAP : public ECA_FILE_IO {

 private:

  int fd_rep;
  caddr_t buffer_repp;
  long int bytes_rep;
  long int fposition_rep;
  long int flength_rep;

  bool file_ready_rep;
  bool file_ended_rep;
  std::string mode_rep;
  std::string fname_rep;
   
 public:


  // --
  // Open/close routines
  // ---
  void open_file(const std::string& fname, 
		 const std::string& fmode);
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
  const std::string& file_mode(void) const { return(mode_rep); }

  ECA_FILE_IO_MMAP(void);
  ~ECA_FILE_IO_MMAP(void);
};

#endif
