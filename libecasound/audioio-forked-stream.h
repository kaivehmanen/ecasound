#ifndef INCLUDED_AUDIOIO_FORKED_STREAM_H
#define INCLUDED_AUDIOIO_FORKED_STREAM_H

#include <string>

/**
 * Helper class providing routines for forking new processes 
 * and creating read/write pipes between the child and the 
 * parent process.
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_FORKED_STREAM {

 private:

  int pid_of_child_rep;
  int fd_rep;
  bool last_fork_rep;

 public:
  
  void fork_child_for_read(const string& cmd, const string& object);
  void fork_child_for_write(const string& cmd, const string& object);
  void clean_child(void);

  bool wait_for_child(void) const;
  bool child_fork_succeeded(void) const { return(last_fork_rep); }
  int pid_of_child(void) const { return(pid_of_child_rep); }
  int file_descriptor(void) const { return(fd_rep); }
};

#endif
