// ------------------------------------------------------------------------
// audioio-forked-streams.cpp: Helper class providing routines for
//                             forking for piped input/output.
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <vector>
#include <string>
#include <cstring>

#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <kvutils/kvu_numtostr.h>
#include <kvutils/kvutils.h>

#include "audioio-forked-stream.h"

/**
 * If found, replaces the string '%f' with 'filename'. This is
 * the file used by the forked child for input/output.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_file_name(const string& filename) {
  object_rep = filename;
}

/**
 * If found, replaces the string '%F' with a path name to a 
 * temporary named pipe. This pipe will be used for communicating
 * with the forked child instead of standard input and output pipes.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_pipe_name(void) {
  if (command_rep.find("%F") != string::npos) {
    ::tmpnam(tmpfile_repp);
    ::strcat(tmpfile_repp, ".raw");
    ::mkfifo(tmpfile_repp, 0755);
    command_rep.replace(command_rep.find("%F"), 2, string(tmpfile_repp));
    tmp_file_created_rep = true;
  }
}

/**
 * If found, replaces the string '%c' with value of parameter
 * 'channels'.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_channels(int channels) {
  if (command_rep.find("%c") != string::npos) {
    command_rep.replace(command_rep.find("%c"), 2, kvu_numtostr(channels));
  }
}

/**
 * If found, replaces the string '%s' with value of parameter
 * 'sample_rate'.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_sample_rate(long int sample_rate) {
  if (command_rep.find("%s") != string::npos) {
    command_rep.replace(command_rep.find("%s"), 2, kvu_numtostr(sample_rate));
  }
}

/**
 * If found, replaces the string '%b' with value of parameter
 * 'bits'.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_bits(int bits) {
  if (command_rep.find("%b") != string::npos) {
    command_rep.replace(command_rep.find("%b"), 2, kvu_numtostr(bits));
  }
}

void AUDIO_IO_FORKED_STREAM::fork_child_for_read(void) {
  last_fork_rep = false;
  fd_rep = 0;

  if (tmp_file_created_rep == true) {
    fork_child_for_fifo_read();
  }
  else {
    int fpipes[2];
    if (pipe(fpipes) == 0) {
      pid_of_child_rep = fork();
      if (pid_of_child_rep == 0) { 
	// ---
	// child 
	// ---
	::close(1);
	dup2(fpipes[1], 1);
	::close(fpipes[0]);
	::close(fpipes[1]);
	freopen("/dev/null", "w", stderr);
	vector<string> temp = string_to_words(command_rep);
	if (temp.size() > 1024) temp.resize(1024);
	const char* args[1024];
	// = new char* [temp.size() + 1];
	vector<string>::size_type p = 0;
	while(p < temp.size()) {
	  if (temp[p] == "%f") 
	    args[p] = object_rep.c_str();
	  else
	    args[p] = temp[p].c_str();
	  ++p;
	}
	args[p] = 0;
	int res = execvp(temp[0].c_str(), const_cast<char**>(args));
	::close(1);
	exit(res);
	cerr << "You shouldn't see this!\n";
      }
      else if (pid_of_child_rep > 0) { 
	// ---
	// parent
	// ---
	::close(fpipes[1]);
	fd_rep = fpipes[0];
	if (wait_for_child() == true)
	  last_fork_rep = true;
	else
	  last_fork_rep = false;
      }
    }
  }
}

void AUDIO_IO_FORKED_STREAM::fork_child_for_fifo_read(void) {
  last_fork_rep = false;
  fd_rep = 0;

  pid_of_child_rep = fork();
  if (pid_of_child_rep == 0) { 
    // ---
    // child 
    // ---
    freopen("/dev/null", "w", stderr);
    vector<string> temp = string_to_words(command_rep);
    if (temp.size() > 1024) temp.resize(1024);
    const char* args[1024];
    vector<string>::size_type p = 0;
    while(p < temp.size()) {
      if (temp[p] == "%f") 
	args[p] = object_rep.c_str();
      else
	args[p] = temp[p].c_str();
      ++p;
    }
    args[p] = 0;
    int res = execvp(temp[0].c_str(), const_cast<char**>(args));
    if (res < 0) {
      /**
       * If execvp failed, make sure that the other end of 
       * the pipe doesn't block forever.
       */
      int fd = ::open(tmpfile_repp, O_WRONLY);
      ::close(fd);
    }
    
    exit(res);
    cerr << "You shouldn't see this!\n";
  }
  else if (pid_of_child_rep > 0) { 
    // ---
    // parent
    // ---
    fd_rep = 0;
    if (wait_for_child() == true)
      fd_rep = ::open(tmpfile_repp, O_RDONLY);
    if (fd_rep > 0)
      last_fork_rep = true;
  }
}

void AUDIO_IO_FORKED_STREAM::fork_child_for_write(void) {
  last_fork_rep = false;
  fd_rep = 0;
  
  int fpipes[2];
  if (pipe(fpipes) == 0) {
    pid_of_child_rep = fork();
    if (pid_of_child_rep == 0) { 
      // ---
      // child 
      // ---
      ::close(0);
      ::dup2(fpipes[0],0);
      ::close(fpipes[0]);
      ::close(fpipes[1]);
      freopen("/dev/null", "w", stderr);
      vector<string> temp = string_to_words(command_rep);
      if (temp.size() > 1024) temp.resize(1024);
      const char* args[1024];
      // = new char* [temp.size() + 1];
      vector<string>::size_type p = 0;
      while(p < temp.size()) {
	if (temp[p] == "%f") 
	  args[p] = object_rep.c_str();
	else
	  args[p] = temp[p].c_str();

	++p;
      }
      args[p] = 0;
      int res = execvp(temp[0].c_str(), const_cast<char**>(args));
      ::close(0);
      exit(res);
      cerr << "You shouln't see this!\n";
    }
    else if (pid_of_child_rep > 0) { 
      // ---
      // parent
      // ---
      ::close(fpipes[0]);
      fd_rep = fpipes[1];
      if (wait_for_child() == true)
	last_fork_rep = true;
      else
	last_fork_rep = false;
    }
  }
}

void AUDIO_IO_FORKED_STREAM::clean_child(void) {
  if (pid_of_child_rep > 0) {
    kill(pid_of_child_rep, SIGTERM);
    waitpid(pid_of_child_rep, 0, 0);
  }
  ::close(fd_rep);
  if (tmp_file_created_rep == true) {
    ::remove(tmpfile_repp);
  }
}

bool AUDIO_IO_FORKED_STREAM::wait_for_child(void) const {
  if (waitpid(pid_of_child_rep, 0, WNOHANG) != 0)
    return false;

  return true;
}
