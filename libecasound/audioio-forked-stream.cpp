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
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include <kvutils/kvu_numtostr.h>
#include <kvutils/kvutils.h>

#include "audioio-forked-stream.h"

void AUDIO_IO_FORKED_STREAM::fork_child_for_read(const string& cmd,
						 const string& object) {
  last_fork_rep = false;
  fd_rep = 0;
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
      vector<string> temp = string_to_words(cmd);
      if (temp.size() > 1024) temp.resize(1024);
      const char* args[1024];
      // = new char* [temp.size() + 1];
      vector<string>::size_type p = 0;
      while(p < temp.size()) {
	if (temp[p] == "%f") 
	  args[p] = object.c_str();
	else
	  args[p] = temp[p].c_str();
	++p;
      }
      args[p] = 0;
      execvp(temp[0].c_str(), const_cast<char**>(args));
      ::close(1);
      exit(0);
      cerr << "You shouldn't see this!\n";
    }
    else if (pid_of_child_rep > 0) { 
      // ---
      // parent
      // ---
      ::close(fpipes[1]);
      fd_rep = fpipes[0];
      last_fork_rep = true;
    }
  }
}

void AUDIO_IO_FORKED_STREAM::fork_child_for_write(const string& param_cmd,
						  const string& object) {
  last_fork_rep = false;
  fd_rep = 0;
  
  string cmd = param_cmd;
  if (cmd.find("%f") != string::npos) {
    cmd.replace(cmd.find("%f"), 2, object);
  }
  
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
      vector<string> temp = string_to_words(cmd);
      if (temp.size() > 1024) temp.resize(1024);
      const char* args[1024];
      // = new char* [temp.size() + 1];
      vector<string>::size_type p = 0;
      while(p < temp.size()) {
	args[p] = temp[p].c_str();
	++p;
      }
      args[p] = 0;
      execvp(temp[0].c_str(), const_cast<char**>(args));
      ::close(0);
      exit(0);
      cerr << "You shouln't see this!\n";
    }
    else if (pid_of_child_rep > 0) { 
      // ---
      // parent
      // ---
      ::close(fpipes[0]);
      fd_rep = fpipes[1];
      last_fork_rep = true;
    }
  }
}

void AUDIO_IO_FORKED_STREAM::clean_child(void) {
  kill(pid_of_child_rep, SIGKILL);
  waitpid(pid_of_child_rep, 0, 0);
  ::close(fd_rep);
}

bool AUDIO_IO_FORKED_STREAM::wait_for_child(void) const {
  if (waitpid(pid_of_child_rep, 0, WNOHANG) < 0)
    return false;

  return true;
}
