#ifndef INCLUDED_ECA_CONTROL_BASE_H
#define INCLUDED_ECA_CONTROL_BASE_H

#include <pthread.h>

#include <kvu_locks.h>

#include "audioio.h"
#include "eca-chain.h"
#include "eca-session.h"
#include "dynamic-parameters.h"
#include "eca-iamode-parser.h"
#include "sample-specs.h"

class ECA_CHAINSETUP;
class ECA_ENGINE;

/**
 * Base class providing basic functionality for controlling the 
 * ecasound library.
 * @author Kai Vehmanen
 */
class ECA_CONTROL_BASE {

 private:

  bool req_batchmode_rep;
  pthread_t th_cqueue_rep;
  ATOMIC_INTEGER engine_exited_rep;

  static void* start_normal_thread(void *ptr);

  void start_engine_sub(bool batchmode);
  void run_engine(void);

 protected:

  void close_engine(void);

  ECA_ENGINE* engine_repp;
  ECA_SESSION* session_repp;
  ECA_CHAINSETUP* selected_chainsetup_repp;

 public:

  // -------------------------------------------------------------------
  // Runtime control
  // -------------------------------------------------------------------

  void engine_start(void);
  void start(void);
  void stop(void);
  void stop_on_condition(void);
  void run(void);
  void quit(void);

  // -------------------------------------------------------------------
  // ECI specific routines
  // -------------------------------------------------------------------

 private:

  std::vector<std::string> last_los_rep;
  std::string last_s_rep;
  long int last_li_rep;
  int last_i_rep;
  double last_f_rep;
  std::string last_error_rep;
  std::string last_type_rep;

 protected:

  void set_last_string_list(const std::vector<std::string>& s);
  void set_last_string(const std::string& s);
  void set_last_float(double v);
  void set_last_integer(int v);
  void set_last_long_integer(long int v);
  void set_last_error(const std::string& s);
  void clear_last_values(void);

 public:

  const std::vector<std::string>& last_string_list(void) const;
  const std::string& last_string(void) const;
  double last_float(void) const;
  int last_integer(void) const;
  long int last_long_integer(void) const;
  const std::string& last_error(void) const;
  const std::string& last_type(void) const;

  // -------------------------------------------------------------------
  // Session info / functions
  // -------------------------------------------------------------------
  
  std::string attached_chains_input(AUDIO_IO* aiod) const;
  std::string attached_chains_output(AUDIO_IO* aiod) const;
  std::vector<std::string> attached_chains(const std::string& name) const;

  // -------------------------------------------------------------------
  // Session info / position and length of selected chainsetup
  // -------------------------------------------------------------------

  SAMPLE_SPECS::sample_pos_t length_in_samples(void) const;
  double length_in_seconds_exact(void) const;
  SAMPLE_SPECS::sample_pos_t position_in_samples(void) const;
  double position_in_seconds_exact(void) const;

  // -------------------------------------------------------------------
  // Session info / global resources (~/.ecasoundrc)
  // -------------------------------------------------------------------

  /**
   * Get resource values from ~/.ecasoundrc
   */
  std::string resource_value(const std::string& key) const;

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
  bool is_engine_started(void) const { return(engine_repp != 0); }
  std::string engine_status(void) const;

  ECA_CONTROL_BASE (ECA_SESSION* psession);
  ~ECA_CONTROL_BASE (void);
};

#endif
