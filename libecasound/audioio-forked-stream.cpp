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
#include <iostream>
#include <cstring>
#include <cstdio>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <kvutils/kvu_numtostr.h>
#include <kvutils/kvutils.h>

#include "eca-debug.h"
#include "audioio-forked-stream.h"

/**
 * If found, replaces the string '%f' with 'filename'. This is
 * the file used by the forked child for input/output.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_file_name(const std::string& filename) {
  object_rep = filename;
}

/**
 * If found, replaces the string '%F' with a path name to a 
 * temporary named pipe. This pipe will be used for communicating
 * with the forked child instead of standard input and output pipes.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_pipe_name(void) {
  if (command_rep.find("%F") != std::string::npos) {
    init_temp_directory();
    if (tempfile_dir_rep.is_valid() == true) {
      tmpfile_repp = tempfile_dir_rep.create_filename("fork-pipe", ".raw");
      ::mkfifo(tmpfile_repp.c_str(), 0755);
      command_rep.replace(command_rep.find("%F"), 2, tmpfile_repp);
      tmp_file_created_rep = true;
    }
  }
}

void AUDIO_IO_FORKED_STREAM::init_temp_directory(void) {
  std::string tmpdir ("ecasound-");
  char* tmp_p = getenv("USER");
  if (tmp_p != NULL) {
    tmpdir += std::string(tmp_p);
    tempfile_dir_rep.reserve_directory(tmpdir);
  }
  if (tempfile_dir_rep.is_valid() != true) {
    ecadebug->msg("(audioio-forked-stream) Warning! Unable to create temporary directory \"" + tmpdir + "\".");
  }
}

/**
 * If found, replaces the string '%c' with value of parameter
 * 'channels'.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_channels(int channels) {
  if (command_rep.find("%c") != std::string::npos) {
    command_rep.replace(command_rep.find("%c"), 2, kvu_numtostr(channels));
  }
}

/**
 * If found, replaces the string '%s' with value of parameter
 * 'sample_rate'.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_sample_rate(long int sample_rate) {
  if (command_rep.find("%s") != std::string::npos) {
    command_rep.replace(command_rep.find("%s"), 2, kvu_numtostr(sample_rate));
  }
}

/**
 * If found, replaces the string '%b' with value of parameter
 * 'bits'.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_bits(int bits) {
  if (command_rep.find("%b") != std::string::npos) {
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
	std::vector<std::string> temp = string_to_words(command_rep);
	if (temp.size() > 1024) temp.resize(1024);
	const char* args[1024];
	// = new char* [temp.size() + 1];
	std::vector<std::string>::size_type p = 0;
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
	std::cerr << "You shouldn't see this!\n";
      }
      else if (pid_of_child_rep > 0) { 
	// ---
	// parent
	// ---
	pid_of_parent_rep = ::getpid();
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
    std::vector<std::string> temp = string_to_words(command_rep);
    if (temp.size() > 1024) temp.resize(1024);
    const char* args[1024];
    std::vector<std::string>::size_type p = 0;
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
      std::cerr << "(audioio-forked-stream) execvp() failed!\n";
      int fd = open(tmpfile_repp.c_str(), O_WRONLY);
      close(fd);
    }
    
    exit(res);
    std::cerr << "You shouldn't see this!\n";
  }
  else if (pid_of_child_rep > 0) { 
    // ---
    // parent
    // ---
    pid_of_parent_rep = ::getpid();
    fd_rep = 0;
    if (wait_for_child() == true)
      fd_rep = ::open(tmpfile_repp.c_str(), O_RDONLY);
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
      std::vector<std::string> temp = string_to_words(command_rep);
      if (temp.size() > 1024) temp.resize(1024);
      const char* args[1024];
      // = new char* [temp.size() + 1];
      std::vector<std::string>::size_type p = 0;
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
      std::cerr << "You shouln't see this!\n";
    }
    else if (pid_of_child_rep > 0) { 
      // ---
      // parent
      // ---
      pid_of_parent_rep = ::getpid();
      ::close(fpipes[0]);
      fd_rep = fpipes[1];
      if (wait_for_child() == true)
	last_fork_rep = true;
      else
	last_fork_rep = false;
    }
  }
}

/**
 * Cleans (waits for) the forked child process. Note! This
 * function must be called from the same thread as 
 * fork_child_for_read/write() was called.
 */
void AUDIO_IO_FORKED_STREAM::clean_child(void) {
  if (pid_of_child_rep > 0) {
    kill(pid_of_child_rep, SIGTERM);
    if (::getpid() == pid_of_parent_rep) {
      waitpid(pid_of_child_rep, 0, 0);
      pid_of_child_rep = 0;
    }
    else {
      ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-forked-stream) Warning! Parent-pid changed!");
    }
  }
  if (fd_rep > 0) ::close(fd_rep);
  if (tmp_file_created_rep == true) {
    ::remove(tmpfile_repp.c_str());
  }
}

/**
 * Checks whether child is still active. Returns false 
 * if child has exited, otherwise true.
 */
bool AUDIO_IO_FORKED_STREAM::wait_for_child(void) const {
  if (pid_of_child_rep > 0) {
    int pid = waitpid(pid_of_child_rep, 0, WNOHANG);
    if (pid > 0) {
      return false;
    }
    if (pid == 0) return true;
  }
  return false;
}
