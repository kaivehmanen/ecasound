#ifndef INCLUDED_FILEIO_H
#define INCLUDED_FILEIO_H

#include <string>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef USE_CXX_STD_NAMESPACE
using std::string;
#endif

/**
 * Interface for blocking file input/output with buffering
 */
class ECA_FILE_IO {
 public:

  // -----
  // Open/close routines

  virtual void open_file(const string& fname,
			 const string& fmode) = 0;
  virtual void open_stdin(void) = 0;
  virtual void open_stdout(void) = 0;
  virtual void close_file(void) = 0;

  // ----
  // Normal file operations

  virtual void read_to_buffer(void* obuf, long int bytes) = 0;
  virtual void write_from_buffer(void* obuf, long int bytes) = 0;

  virtual void set_file_position(long int newpos) = 0;
  virtual void set_file_position_advance(long int fw) = 0;
  virtual void set_file_position_end(void) = 0;
  virtual long int get_file_position(void) const = 0;
  virtual long int get_file_length(void) const = 0;

  // -----
  // Status

  virtual bool is_file_ready(void) const = 0;
  virtual bool is_file_error(void) const = 0;
  virtual long int file_bytes_processed(void) const = 0;
  virtual const string& file_mode(void) const = 0;

  virtual ~ECA_FILE_IO(void) { }
};

#endif
