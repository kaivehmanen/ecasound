#ifndef INCLUDED_ECA_NETECI_SERVER_H
#define INCLUDED_ECA_NETECI_SERVER_H

#include <string>

#include <sys/socket.h>   /* Generic socket definitions */
#include <sys/un.h>       /* UNIX socket definitions */
#include <netinet/in.h>   /* IP socket definitions */

struct ecasound_state;

/**
 * NetECI server implementation.
 *
 * @author Kai Vehmanen
 */
class ECA_NETECI_SERVER {

 public:

  /**
   * Constructor.
   */
  ECA_NETECI_SERVER(void);

  /**
   * Virtual destructor.
   */
  ~ECA_NETECI_SERVER(void);

  static void* launch_server_thread(void* arg);

 private:

  void run(struct ecasound_state* state);

  void create_server_socket(void);
  void open_server_socket(void);
  void close_server_socket(void);

  struct sockaddr_un addr_un_rep;
  struct sockaddr_in addr_in_rep;
  struct sockaddr* addr_repp;

  std::string socketpath_rep;
  int srvfd_rep;
  bool server_listening_rep;
  bool unix_sockets_rep;

};

#endif /* INCLUDED_ECA_NETECI_SERVER_H */
