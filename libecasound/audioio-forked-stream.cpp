// ------------------------------------------------------------------------
// audioio-forked-streams.cpp: Helper class providing routines for
//                             forking for piped input/output.
// Copyright (C) 2000-2004,2006 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

#include <kvu_dbc.h>
#include <kvu_numtostr.h>
#include <kvu_utils.h>

#include "eca-logger.h"
#include "audioio-forked-stream.h"

using namespace std;

/**
 * Maximum number of arguments passed to exec()
 */
const static int afs_max_exec_args = 1024;

/**
 * Runs exec() with the given parameters.
 * @return exec() return value
 */
static int afs_run_exec(const string& command, const string& filename)
{
  vector<string> temp = kvu_string_to_tokens_quoted(command);
  if (static_cast<int>(temp.size()) > afs_max_exec_args) {
    temp.resize(afs_max_exec_args);
    ECA_LOG_MSG(ECA_LOGGER::info, "WARNING: too many arguments for external application, truncating.");
  }
  const char* args[afs_max_exec_args];
  vector<string>::size_type p = 0;
  while(p < temp.size()) {
    if (temp[p].find("%f") != string::npos) {
      temp[p].replace(temp[p].find("%f"), 2, filename);
      args[p] = temp[p].c_str();
    }
    else
      args[p] = temp[p].c_str();
    ++p;
  }
  args[p] = 0;
  return execvp(temp[0].c_str(), const_cast<char**>(args));
}

/**
 * If found, replaces the string '%f' with 'filename'. This is
 * the file used by the forked child for input/output.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_file_name(const string& filename)
{
  object_rep = filename;
  /* do not yet replace %f yet as it would make it more
     difficult to tokenize the exec string */
}

/**
 * If found, replaces the string '%F' with a path name to a 
 * temporary named pipe. This pipe will be used for communicating
 * with the forked child instead of standard input and output pipes.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_pipe_name(void)
{
  if (command_rep.find("%F") != string::npos) {
    use_named_pipe_rep = true;
    init_temp_directory();
    if (tempfile_dir_rep.is_valid() == true) {
      tmpfile_repp = tempfile_dir_rep.create_filename("fork-pipe", ".raw");
      ::mkfifo(tmpfile_repp.c_str(), 0755);
      command_rep.replace(command_rep.find("%F"), 2, tmpfile_repp);
      tmp_file_created_rep = true;
    }
    else 
      tmp_file_created_rep = false;
  }
  else 
    use_named_pipe_rep = false;
}

void AUDIO_IO_FORKED_STREAM::init_temp_directory(void)
{
  string tmpdir ("ecasound-");
  char* tmp_p = getenv("USER");
  if (tmp_p != NULL) {
    tmpdir += string(tmp_p);
    tempfile_dir_rep.reserve_directory(tmpdir);
  }
  if (tempfile_dir_rep.is_valid() != true) {
    ECA_LOG_MSG(ECA_LOGGER::info, "WARNING: Unable to create temporary directory \"" + tmpdir + "\".");
  }
}

/**
 * If found, replaces the string '%c' with value of parameter
 * 'channels'.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_channels(int channels)
{
  if (command_rep.find("%c") != string::npos) {
    command_rep.replace(command_rep.find("%c"), 2, kvu_numtostr(channels));
  }
}

/**
 * If found, replaces the string '%s' with value of parameter
 * 'sample_rate', and '%S' with 'sample_rate/1000' (kHz).
 */
void AUDIO_IO_FORKED_STREAM::set_fork_sample_rate(long int sample_rate)
{
  if (command_rep.find("%s") != string::npos) {
    command_rep.replace(command_rep.find("%s"), 2, kvu_numtostr(sample_rate));
  }
  if (command_rep.find("%S") != string::npos) {
    command_rep.replace(command_rep.find("%S"), 2, kvu_numtostr(sample_rate/1000.0f));
  }
}

/**
 * If found, replaces the string '%b' with value of parameter
 * 'bits'.
 */
void AUDIO_IO_FORKED_STREAM::set_fork_bits(int bits)
{
  if (command_rep.find("%b") != string::npos) {
    command_rep.replace(command_rep.find("%b"), 2, kvu_numtostr(bits));
  }
}

void AUDIO_IO_FORKED_STREAM::fork_child_for_read(void)
{
  last_fork_rep = false;
  fd_rep = 0;

  ECA_LOG_MSG(ECA_LOGGER::user_objects, "Fork child-for-read: '" + fork_command() + "'");

  if (use_named_pipe_rep == true) {
    if (tmp_file_created_rep == true) {
      fork_child_for_fifo_read();
    }
    else {
      last_fork_rep = false;
    }
  }
  else {
    int fpipes[2];
    if (pipe(fpipes) == 0) {
#ifdef HAVE_SIGPROCMASK
      sigset_t oldset, newset;
      sigemptyset(&newset);
      sigaddset(&newset, SIGTERM);
      sigprocmask(SIG_UNBLOCK, &newset, &oldset);
#endif
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
	int res = afs_run_exec(command_rep, object_rep);
	::close(1);
	exit(res);
	cerr << "You shouldn't see this!\n";
      }
      else if (pid_of_child_rep > 0) { 
	// ---
	// parent
	// ---
#ifdef HAVE_SIGPROCMASK
	sigprocmask(SIG_SETMASK, &oldset, 0);
#endif
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

void AUDIO_IO_FORKED_STREAM::fork_child_for_fifo_read(void)
{
  last_fork_rep = false;
  fd_rep = 0;

  ECA_LOG_MSG(ECA_LOGGER::user_objects, "Fork child-for-fifo-read: '" + fork_command() + "'");

#ifdef HAVE_SIGPROCMASK
  sigset_t oldset, newset;
  sigemptyset(&newset);
  sigaddset(&newset, SIGTERM);
  sigprocmask(SIG_UNBLOCK, &newset, &oldset);
#endif
  pid_of_child_rep = fork();
  if (pid_of_child_rep == 0) { 
    // ---
    // child 
    // ---
    freopen("/dev/null", "w", stderr);
    int res = afs_run_exec(command_rep, object_rep);
    if (res < 0) {
      /**
       * If execvp failed, make sure that the other end of 
       * the pipe doesn't block forever.
       */
      cerr << "execvp() failed!\n";
      int fd = open(tmpfile_repp.c_str(), O_WRONLY);
      close(fd);
    }
    
    exit(res);
    cerr << "You shouldn't see this!\n";
  }
  else if (pid_of_child_rep > 0) { 
    // ---
    // parent
    // ---
#ifdef HAVE_SIGPROCMASK
    sigprocmask(SIG_SETMASK, &oldset, 0);
#endif
    pid_of_parent_rep = ::getpid();
    fd_rep = 0;
    if (wait_for_child() == true)
      fd_rep = ::open(tmpfile_repp.c_str(), O_RDONLY);
    if (fd_rep > 0)
      last_fork_rep = true;
  }
}

void AUDIO_IO_FORKED_STREAM::fork_child_for_write(void)
{
  last_fork_rep = false;
  fd_rep = 0;

  ECA_LOG_MSG(ECA_LOGGER::user_objects, "Fork child-for-write: '" + fork_command() + "'");
  
  int fpipes[2];
  if (pipe(fpipes) == 0) {
#ifdef HAVE_SIGPROCMASK
    sigset_t oldset, newset;
    sigemptyset(&newset);
    sigaddset(&newset, SIGTERM);
    sigprocmask(SIG_UNBLOCK, &newset, &oldset);
#endif
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
      exit(afs_run_exec(command_rep, object_rep));
      cerr << "You shouln't see this!\n";
    }
    else if (pid_of_child_rep > 0) { 
      // ---
      // parent
      // ---
#ifdef HAVE_SIGPROCMASK
      sigprocmask(SIG_SETMASK, &oldset, 0);
#endif
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
void AUDIO_IO_FORKED_STREAM::clean_child(bool force)
{
  if (fd_rep > 0) {
    ::close(fd_rep);
    fd_rep = -1;
  }

  if (pid_of_child_rep > 0) {
    int flags = 0;
#ifdef __WALL
    /* Linux-specific flag to waitpid() for to wait for
     * childs created by other threads */
    flags += __WALL;
#endif
    /* wait until child process has exited */
    int status = 0;
    int res = waitpid(pid_of_child_rep, &status, flags);
    if (WIFEXITED(status)) {
      ECA_LOG_MSG(ECA_LOGGER::system_objects, "Child process exit ok");
      pid_of_child_rep = 0;
    }
    else if (res < 0) {
      perror("waitpid");
      force = true;
    }
  }

  if (force == true && pid_of_child_rep > 0) {
    ECA_LOG_MSG(ECA_LOGGER::system_objects, "WARNING: sending SIGTERM to child process");
    /* close did not trigger exit, send SIGTERM */
    kill(pid_of_child_rep, SIGTERM);
    pid_of_child_rep = 0;
  }
  
  if (tmp_file_created_rep == true) {
    ::remove(tmpfile_repp.c_str());
    tmp_file_created_rep = false;
  }
}

/**
 * Checks whether child is still active. Returns false 
 * if child has exited, otherwise true.
 */
bool AUDIO_IO_FORKED_STREAM::wait_for_child(void) const
{
  if (pid_of_child_rep > 0) {
    int pid = waitpid(pid_of_child_rep, 0, WNOHANG);
    if (pid > 0) {
      return false;
    }
    if (pid == 0) return true;
  }
  return false;
}
