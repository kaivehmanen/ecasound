#ifndef _CHAIN_H
#define _CHAIN_H

#include <string>
#include <vector>

#include "eca-debug.h"
#include "eca-operator.h"
#include "eca-chainop.h"
#include "samplebuffer.h"

/*  #include "eca-chainop.h" */
/*  #include "eca-chainop-map.h" */
/*  #include "eca-controller-map.h" */

class GENERIC_CONTROLLER;
class AUDIO_IO;


/**
 * Class representing an abstract audio signal chain.
 */
class CHAIN {

  friend class ECA_PROCESSOR;
  friend class ECA_SESSION;
  friend class ECA_AUDIO_OBJECTS;
  friend class ECA_CONTROLLER;
  friend class ECA_CONTROLLER_OBJECTS;
  friend void *mthread_process_chains(void* params);

 private:

  bool initialized_rep;

  string chainname;

  bool muted;
  bool sfx;

  int in_channels_rep;
  int out_channels_rep;

  vector<CHAIN_OPERATOR*> chainops;
  vector<GENERIC_CONTROLLER*> gcontrollers;

  CHAIN_OPERATOR* selected_chainop;
  GENERIC_CONTROLLER* selected_controller_rep;
  OPERATOR* selected_dynobj;

  int selected_chainop_number;
  int selected_controller_number;

  vector<CHAIN_OPERATOR*>::const_iterator chainop_citer;

  AUDIO_IO* input_id;
  AUDIO_IO* output_id;

  SAMPLE_BUFFER* audioslot;
 
 public:

  bool is_initialized(void) const { return(initialized_rep); }
  bool is_muted(void) const { return(muted); }
  bool is_processing(void) const { return(sfx); }

  void toggle_muting(bool v) { muted = v; }
  void toggle_processing(bool v) { sfx = v; }

  string name(void) const { return(chainname); }
  void name(const string& c) { chainname = c; }

  /**
   * Whether chain is in a valid state (= ready for processing)?
   */
  bool is_valid(void) const;

  /**
   * Connect input to chain
   */
  void connect_input(AUDIO_IO* input);

  /**
   * Disconnect input
   */
  void disconnect_input(void) { input_id = 0; initialized_rep = false; }

  /**
   * Connect output to chain
   */
  void connect_output(AUDIO_IO* output);

  /**
   * Disconnect output
   */
  void disconnect_output(void) { output_id = 0; initialized_rep = false; }

  /**
   * Clear chain (removes all chain operators and controllers)
   */
  void clear(void);

  /**
   * Add chain operator to the end of the chain
   *
   * require:
   *  chainop != 0
   *
   * ensure:
   *  selected_chain_operator() == number_of_chain_operators()
   *  is_processing()
   */
  void add_chain_operator(CHAIN_OPERATOR* chainop);

  /**
   * Remove selected chain operator
   *
   * require:
   *  selected_chain_operator() <= number_of_chain_operators();
   *  selected_chain_operator() > 0
   *
   * ensure:
   *  (chainsops.size() == 0 && is_processing()) ||
   *  (chainsops.size() != 0 && !is_processing())
   */
  void remove_chain_operator(void);

  /**
   * Set parameter value (selected chain operator) 
   *
   * @param index parameter number
   * @param value new value
   *
   * require:
   *  selected_chainop_number > 0 && selected_chainop_number <= number_of_chain_operators()
   *  index > 0
   */
  void set_parameter(int index, CHAIN_OPERATOR::parameter_type value);

  /**
   * Get parameter value (selected chain operator) 
   *
   * @param index parameter number
   *
   * require:
   *  index > 0 &&
   *  selected_chain_operator() != ""
   */
  CHAIN_OPERATOR::parameter_type get_parameter(int index) const;

  /**
   * Select chain operator
   *
   * require:
   *  index > 0
   *
   * ensure:
   *  index == selected_chain_operator()
   */
  void select_chain_operator(int index);

  /**
   * Index of selected chain operator
   */
  int selected_chain_operator(void) const { return(selected_chainop_number); }

  int number_of_chain_operators(void) const { return(chainops.size()); }

  /**
   * Add a generic controller and assign it to selected dynamic object
   *
   * require:
   *  gcontroller != 0
   *  selected_dynobj != 0
   */
  void add_controller(GENERIC_CONTROLLER* gcontroller);

  int number_of_controllers(void) const { return(gcontrollers.size()); }

  /**
   * Select controller
   *
   * require:
   *  index > 0
   *
   * ensure:
   *  index == selected_controller()
   */
  void select_controller(int index);

  /**
   * Index of selected chain operator
   */
  int selected_controller(void) const { return(selected_controller_number); }

  /**
   * Use current selected chain operator as 
   * target for parameters control.
   *
   * require:
   *   selected_chainop != 0
   *
   * ensure:
   *   selected_dynobj == selected_chainop
   */
  void selected_chain_operator_as_target(void);

  /**
   * Use current selected controller as 
   * target for parameter control.
   *
   * require:
   *   selected_controller != 0
   *
   * ensure:
   *   selected_dynobj == selected_controller
   */
  void selected_controller_as_target(void);

  /**
   * Prepare chain for processing. All further processing
   * will be done using the buffer pointer by 'sbuf'.
   *
   * require:
   *  input_id != 0 || in_channels != 0
   *  output_id != 0 || out_channels != 0
   *  sbuf != 0
   *
   * ensure:
   *  is_initialized() == true
   */
  void init(SAMPLE_BUFFER* sbuf, int in_channels = 0, int out_channels = 0);

  /**
   * Process chain data with all chain operators.
   *
   * require:
   *  is_initialized() == true
   */
  void process(void);

  /**
   * Calculate/fetch new values for all controllers.
   */
  void controller_update(void);

  /**
   * Re-initializes all effect parameters.
   */
  void refresh_parameters(void);

  /**
   * Convert chain to a formatted string.
   */
  string to_string(void) const;
  string chain_operator_to_string(CHAIN_OPERATOR* chainop) const;
  string controller_to_string(GENERIC_CONTROLLER* gctrl) const;

  CHAIN (void);
  virtual ~CHAIN (void);
};

#endif
