#ifndef INCLUDED_ECA_CONTROL_DUMP_H
#define INCLUDED_ECA_CONTROL_DUMP_H

#include <fstream>

class ECA_CONTROL;

/**
 * Class for dumping status information to a standard output stream.
 * @author Kai Vehmanen
 */
class ECA_CONTROL_DUMP {

 public:

  // --
  // Global info
  // --

  /**
   * Dumps engine status - 'running', 'stopped', 'finished' or 'notready'.
   */
  void dump_status(void);

  /**
   * Dumps the global position. Printed in seconds using a floating-point 
   * representation.
   */
  void dump_position(void);

  /**
   * Dumps the overall processing length. Printed in seconds using a floating-point 
   * representation.
   */
  void dump_length(void);

  // --
  // Chainsetups and chains
  // --

  /**
   * Dumps status string for the currently selected chainsetup - 'connected', 
   * 'selected' or an empty string.
   */
  void dump_chainsetup_status(void);

  /**
   * Dumps the name of currently selected chain.
   */
  void dump_selected_chain(void);

  // --
  // Audio objects
  // --

  /**
   * Dumps label of currently selected audio object. If no
   * object is selected, dumps an empty string.
   */
  void dump_selected_audio_object(void);

  /**
   * Dumps position of currently selected audio objects. 
   * Printed in seconds, using a floating-point representation.
   */
  void dump_audio_object_position(void);

  /**
   * Dumps length of currently selected audio object. 
   * Printed in seconds, using a floating-point representation.
   */
  void dump_audio_object_length(void);

  /**
   * Dumps audio object state info. Either 'open' or 'closed'.
   */
  void dump_audio_object_open_state(void);

  // --
  // Chain operators
  // --

  /**
   * Dumps chain operator parameter value
   *
   * @param chainop operator index 1...n
   * @param param parameter index 1...n
   */
  void dump_chain_operator_value(int chainop, int param);

  /** 
   * Set target stream for dumping.
   */
  void set_dump_target(ostream* target) { dostream_repp = target; internal_rep = false; }

  /** 
   * Set target stream for dumping.
   */
  void set_dump_target(const string& filename) { dostream_repp = new ofstream(filename.c_str()); internal_rep = true; }

  /**
   * Class constructor
   */ 
  ECA_CONTROL_DUMP (ECA_CONTROL* ctrl) : ctrl_repp(ctrl), dostream_repp(&cout), internal_rep(false) { }

  /**
   * Virtual destructor
   */
  ~ECA_CONTROL_DUMP (void) { if (internal_rep == true) delete dostream_repp; }

 private:

  ECA_CONTROL* ctrl_repp;
  ostream* dostream_repp;
  bool internal_rep;

  void dump(const string& key, const string& value) { *dostream_repp << key << " " << value << endl; }
};

#endif
