#ifndef INCLUDED_ECA_CONTROL_BASE_H
#define INCLUDED_ECA_CONTROL_BASE_H

#include <pthread.h>
#include <kvutils/definition_by_contract.h>

#include "audioio.h"
#include "eca-chain.h"
#include "eca-session.h"
#include "dynamic-parameters.h"
#include "eca-iamode-parser.h"

extern string ecasound_lockfile;

enum { ECA_QUIT = 1 };

void start_normal_thread(ECA_SESSION* param, int retcode, pthread_t* th_cqueue, pthread_attr_t* th_attr);
void* start_normal(void* param);
void start_normal(ECA_SESSION* param);

class ECA_CHAINSETUP;

/**
 * Base class providing basic functionality for controlling the 
 * ecasound library.
 * @author Kai Vehmanen
 */
class ECA_CONTROL_BASE : public DEFINITION_BY_CONTRACT {

 private:

  int retcode_rep;
  pthread_t th_cqueue_rep;
  bool engine_started_rep;

 protected:

  ECA_SESSION* session_repp;
  ECA_CHAINSETUP* selected_chainsetup_repp;

 public:

  void start_engine(bool ignore_lock = false);
  void close_engine(void);

  // -------------------------------------------------------------------
  // Runtime control
  // -------------------------------------------------------------------

  void start(bool ignore_lock = false);
  void stop(void);
  void stop_on_condition(void);
  void run(void);
  void quit(void);

  // -------------------------------------------------------------------
  // Session info / functions
  // -------------------------------------------------------------------
  
  string attached_chains_input(AUDIO_IO* aiod) const;
  string attached_chains_output(AUDIO_IO* aiod) const;
  vector<string> attached_chains(const string& name) const;

  // -------------------------------------------------------------------
  // Session info / position and length of connected chainsetup
  // -------------------------------------------------------------------

  long length_in_samples(void) const;
  double length_in_seconds_exact(void) const;
  long position_in_samples(void) const;
  double position_in_seconds_exact(void) const;

  // -------------------------------------------------------------------
  // Session info / global resources (~/.ecasoundrc)
  // -------------------------------------------------------------------

  /**
   * Get resource values from ~/.ecasoundrc
   */
  string resource_value(const string& key) const { return session_repp->ecaresources.resource(key); }

  // -------------------------------------------------------------------
  // Modify session
  // -------------------------------------------------------------------

  void toggle_interactive_mode(bool v) { session_repp->iactive_rep = v; } 
  void toggle_multitrack_mode(bool v) { session_repp->multitrack_mode_rep = v; } 
  void toggle_raise_priority(bool v);
  void set_buffersize(int bsize);

  // -------------------------------------------------------------------
  // Session status
  // -------------------------------------------------------------------

  bool is_running(void) const;
  bool is_connected(void) const;
  bool is_selected(void) const;
  bool is_finished(void) const;
  bool is_valid(void) const;

  /**
   * Returns true if engine has been started. 
   */
  bool is_engine_started(void) const { return(engine_started_rep); }
  string engine_status(void) const;

  ECA_CONTROL_BASE (ECA_SESSION* psession);
  virtual ~ECA_CONTROL_BASE (void) { }
};

#endif
