// ------------------------------------------------------------------------
// eca-neteci-server.c: NetECI server implementation.
// Copyright (C) 2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <iostream>
#include <string>

#include <fcntl.h>        /* POSIX: fcntl() */
#include <unistd.h>       /* POSIX: fcntl() */
// #include <sys/poll.h> 

#include <kvu_dbc.h>
#include <kvu_numtostr.h>

#include "ecasound.h"
#include "eca-neteci-server.h"

using namespace std;

ECA_NETECI_SERVER::ECA_NETECI_SERVER(void)
  : srvfd_rep(-1),
    server_listening_rep(false),
    unix_sockets_rep(true)
{
}

ECA_NETECI_SERVER::~ECA_NETECI_SERVER(void)
{
  if (server_listening_rep == true) {
    close_server_socket();
  }
}

void* ECA_NETECI_SERVER::launch_server_thread(void* arg)
{
  cerr << "eca-neteci-server: launch_server_thread()" << endl;

  struct ecasound_state* state = 
    reinterpret_cast<struct ecasound_state*>(arg);

  ECA_NETECI_SERVER* self = state->eciserver;
  self->run(state);
  return(0);
}

void ECA_NETECI_SERVER::run(struct ecasound_state* state)
{
  cerr << "eca-neteci-server: run()" << endl;

  create_server_socket();
  open_server_socket();
  
  sleep(30);

  close_server_socket();

  cerr << "eca-neteci-server: run() exit" << endl;
}

/**
 * Creates a server socket. Depending on 
 * object configuration either UNIX or IP
 * socket is created.
 */
void ECA_NETECI_SERVER::create_server_socket(void)
{

  if (unix_sockets_rep == true) {
    srvfd_rep = socket(AF_UNIX, SOCK_STREAM, 0);
    if (srvfd_rep >= 0) {
      /* create a temporary filename for the socket in a secure way */
      socketpath_rep = "/tmp/neteci_server_1";
      addr_un_rep.sun_family = AF_UNIX;
      memcpy(addr_un_rep.sun_path, socketpath_rep.c_str(), socketpath_rep.size() + 1);
      addr_repp = reinterpret_cast<struct sockaddr*>(&addr_un_rep);
    }
  }
  else {
    srvfd_rep = socket(PF_INET, SOCK_STREAM, 0);
    if (srvfd_rep >= 0) {
      addr_in_rep.sin_family = AF_INET;
      addr_in_rep.sin_port = htons(2868);
      addr_in_rep.sin_addr.s_addr = INADDR_LOOPBACK;
      
      addr_repp = reinterpret_cast<struct sockaddr*>(&addr_in_rep);
    }
  }
}

/**
 * Opens the server socket and starts listening
 * for incoming connetions.
 */
void ECA_NETECI_SERVER::open_server_socket(void)
{
  if (srvfd_rep >= 0) {
    int val = 1;
    int ret = setsockopt(srvfd_rep, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (ret < 0) 
      std::cerr << "(eca-neteci_server) setsockopt() failed." << endl;

    // int res = bind(srvfd_rep, (struct sockaddr*)addr_repp, sizeof(*addr_repp));
    
    int res = 0;
    if (unix_sockets_rep == true) 
      res = bind(srvfd_rep, (struct sockaddr*)&addr_un_rep, sizeof(addr_un_rep));
    else
      res = bind(srvfd_rep, (struct sockaddr*)&addr_in_rep, sizeof(addr_in_rep));

    if (res == 0) {
      res = listen(srvfd_rep, 5);
      if (res == 0) {
	int res = fcntl(srvfd_rep, F_SETFL, O_NONBLOCK);
	if (res == -1) 
	  std::cerr << "(eca-neteci_server) fcntl() failed." << endl;
	
	std::cout << "(eca-neteci_server) server socket created." << endl;
	server_listening_rep = true;
      }
      else 
	std::cerr << "(eca-neteci_server) listen() failed." << endl;
    }
    else {
      if (unix_sockets_rep == true) {
	unlink(socketpath_rep.c_str());
      }
      socketpath_rep.resize(0);
      std::cerr << "(eca-neteci_server) bind() failed." << endl;
    }
  }
  else {
    socketpath_rep.resize(0);
    std::cerr << "(eca-neteci_server) socket() failed." << endl;
  }

  DBC_ENSURE((unix_sockets_rep == true) && 
	     ((server_listening_rep == true && socketpath_rep.size() > 0 ||
	       server_listening_rep != true && socketpath_rep.size() == 0)) ||
	     (unix_sockets_rep != true));
}

/**
 * Closes the server socket.
 */
void ECA_NETECI_SERVER::close_server_socket(void)
{
  DBC_REQUIRE(srvfd_rep > 0);
  DBC_REQUIRE(server_listening_rep == true);

  std::cout << "(eca-neteci-server) closing socket " << kvu_numtostr(srvfd_rep) << "." << endl;
  close(srvfd_rep);
  srvfd_rep = -1;
  server_listening_rep = false;

  DBC_ENSURE(srvfd_rep == -1);
  DBC_ENSURE(server_listening_rep != true);
}
