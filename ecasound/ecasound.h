#ifndef INCLUDED_ECASOUND_H
#define INCLUDED_ECASOUND_H


#include <pthread.h>   /* POSIX: pthread_create() */
#include <signal.h>    /* POSIX: sigaction(), sigwait(), sig_atomic_t */

class ECA_CONSOLE;
class ECA_CONTROL;
class ECA_LOGGER_INTERFACE;
class ECA_NETECI_SERVER;
class ECA_SESSION;

/**
 * Type definitions
 */

struct ecasound_state {
  ECA_CONSOLE* console;
  ECA_CONTROL* control;
  ECA_LOGGER_INTERFACE* logger;
  ECA_NETECI_SERVER* eciserver;
  ECA_SESSION* session;
  pthread_t* daemon_thread;
  sig_atomic_t exit_request;
  int retval;
  bool daemon_mode;
  bool cerr_output_only_mode;
  bool interactive_mode;
  bool quiet_mode;
};

#endif /* INCLUDED_ECASOUND_H */
