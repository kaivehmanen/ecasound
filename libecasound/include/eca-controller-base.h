#ifndef _ECA_CONTROLLER_BASE_H
#define _ECA_CONTROLLER_BASE_H

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
class ECA_CONTROLLER_BASE : public DEFINITION_BY_CONTRACT {

 private:

  int retcode;
  pthread_t th_cqueue;
  bool engine_started;

 protected:

  ECA_SESSION* session_rep;
  ECA_CHAINSETUP* selected_chainsetup_rep;

 public:

  /**
   * Start the processing engine
   *
   * require:
   *  is_connected() == true
   *
   * ensure:
   *  is_engine_started() == true
   */
  void start_engine(bool ignore_lock = false);

  /**
   * Close the processing engine
   *
   * ensure:
   *  is_engine_started() == false
   */
  void close_engine(void);

 public:
  
  // -------------------------------------------------------------------
  // Runtime control
  // -------------------------------------------------------------------

  /**
   * Start the processing engine
   *
   * require:
   *  is_connected() == true
   *
   * ensure:
   *  is_engine_started() == true
   */
  void start(bool ignore_lock = false);

  /**
   * Stop the processing engine
   *
   * require:
   *  is_engine_started() == true
   *
   * ensure:
   *   is_running() == false
   */
  void stop(void);

  /**
   * Start the processing engine and block until 
   * processing is finished.
   *
   * require:
   *  is_connected() == true
   *
   * ensure:
   *  is_finished() == true
   */
  void run(void);

  /**
   * Stop the processing engine and throw an ECA_QUIT exception.
   */
  void quit(void);

  // -------------------------------------------------------------------
  // Session info / functions
  // -------------------------------------------------------------------
  
  /**
   * Get a string containing a comma separated list of all chains 
   * attached to input with index 'aiod'. 
   */
  string connected_chains_input(AUDIO_IO* aiod) const;

  /**
   * Get a string containing a comma separated list of all chains 
   * attached to output with index 'aiod'. 
   */
  string connected_chains_output(AUDIO_IO* aiod) const;

  /**
   * Get a string containing a comma separated list of all chains 
   * attached to audio object with name 'filename'. 
   */
  vector<string> connected_chains(const string& name) const;

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
  const string& resource_value(const string& key) { return session_rep->ecaresources.resource(key); }

  // -------------------------------------------------------------------
  // Modify session
  // -------------------------------------------------------------------

  void toggle_interactive_mode(bool v) { session_rep->iactive = v; } 
  void toggle_multitrack_mode(bool v) { session_rep->multitrack_mode = v; } 

  /**
   * Set the default buffersize (in samples).
   *
   * require:
   *   is_editable() == true
   */
  void set_buffersize(int bsize);

  /**
   * Toggle whether raised priority mode is enabled or not.
   *
   * require:
   *   is_editable() == true
   */
  void toggle_raise_priority(bool v);

  // -------------------------------------------------------------------
  // Session status
  // -------------------------------------------------------------------

  /**
   * Returns true if processing engine is running.
   */
  bool is_running(void) const;

  /**
   * Returns true if active chainsetup exists and is connected.
   */
  bool is_connected(void) const;

  /**
   * Returns true if some chainsetup is selected.
   */
  bool is_selected(void) const;

  /**
   * Returns true if engine has finished processing.
   */
  bool is_finished(void) const;

  /**
   * Is currently selected chainsetup valid?
   *
   * require:
   *  is_selected()
   */
  bool is_valid(void) const;

  /**
   * Returns true if engine has been started. 
   */
  bool is_engine_started(void) const { return(engine_started); }

  /**
   * Return info about engine status.
   */
  string engine_status(void) const;

  ECA_CONTROLLER_BASE (ECA_SESSION* psession);
  virtual ~ECA_CONTROLLER_BASE (void) { }
};

#endif
