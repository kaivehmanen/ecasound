#ifndef INCLUDED_AUDIOIO_FORKED_STREAM_H
#define INCLUDED_AUDIOIO_FORKED_STREAM_H

#include <string>
#include <kvu_temporary_file_directory.h>

/**
 * Helper class providing routines for forking new processes 
 * and creating read/write pipes between the child and the 
 * parent process.
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_FORKED_STREAM {

 private:

  int pid_of_parent_rep;
  int pid_of_child_rep;
  int fd_rep;
  bool last_fork_rep;
  std::string tmpfile_repp;
  bool tmp_file_created_rep;
  bool use_named_pipe_rep;
  std::string command_rep;
  std::string object_rep;
  TEMPORARY_FILE_DIRECTORY tempfile_dir_rep;

  void init_temp_directory(void);
  void fork_child_for_fifo_read(void);

 public:
  
  /**
   * Set the command std::string. This must be done before other set_* 
   * calls.
   */
  void set_fork_command(const std::string& cmd) { command_rep = cmd; }
  void set_fork_file_name(const std::string& filename);
  void set_fork_pipe_name(void);
  void set_fork_channels(int channels);
  void set_fork_sample_rate(long int sample_rate);
  void set_fork_bits(int bits);
  
  void fork_child_for_read(void);
  void fork_child_for_write(void);
  void clean_child(void);

  bool wait_for_child(void) const;
  bool child_fork_succeeded(void) const { return(last_fork_rep); }
  int pid_of_child(void) const { return(pid_of_child_rep); }
  int file_descriptor(void) const { return(fd_rep); }

  AUDIO_IO_FORKED_STREAM(void) : 
    pid_of_child_rep(0),
    fd_rep(0),
    last_fork_rep(false),
    tmp_file_created_rep(false),
    use_named_pipe_rep(false) { }
};

#endif
