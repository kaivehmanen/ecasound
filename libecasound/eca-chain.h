#ifndef INCLUDED_CHAIN_H
#define INCLUDED_CHAIN_H

#include <string>
#include <vector>

#include "eca-chainop.h"
#include "samplebuffer.h"
#include "eca-debug.h"

class AUDIO_IO;
class GENERIC_CONTROLLER;
class OPERATOR;

/**
 * Class representing an abstract audio signal chain.
 */
class CHAIN {

  friend class ECA_PROCESSOR;
  friend class ECA_AUDIO_OBJECTS;
  friend class ECA_CONTROL;
  friend class ECA_CONTROL_OBJECTS;
  friend void *mthread_process_chains(void* params);

 private:

  bool initialized_rep;
  string chainname_rep;
  bool muted_rep;
  bool sfx_rep;
  int in_channels_rep;
  int out_channels_rep;

  vector<CHAIN_OPERATOR*> chainops_rep;
  vector<GENERIC_CONTROLLER*> gcontrollers_rep;

  CHAIN_OPERATOR* selected_chainop_repp;
  GENERIC_CONTROLLER* selected_controller_repp;
  OPERATOR* selected_dynobj_repp;

  int selected_chainop_number_rep;
  int selected_controller_number_rep;

  AUDIO_IO* input_id_repp;
  AUDIO_IO* output_id_repp;

  SAMPLE_BUFFER* audioslot_repp;
 
 public:

  bool is_initialized(void) const { return(initialized_rep); }
  bool is_muted(void) const { return(muted_rep); }
  bool is_processing(void) const { return(sfx_rep); }

  void toggle_muting(bool v) { muted_rep = v; }
  void toggle_processing(bool v) { sfx_rep = v; }

  string name(void) const { return(chainname_rep); }
  void name(const string& c) { chainname_rep = c; }

  /**
   * Whether chain is in a valid state (= ready for processing)?
   */
  bool is_valid(void) const;

  /**
   * Connects input to chain
   */
  void connect_input(AUDIO_IO* input);

  /**
   * Returns a const pointer to input connected to this chain. If no input
   * is connected, 0 is returned.
   */
  const AUDIO_IO* connected_input(void) const { return(input_id_repp); }

  /**
   * Disconnects input
   */
  void disconnect_input(void) { input_id_repp = 0; initialized_rep = false; }

  /**
   * Connects output to chain
   */
  void connect_output(AUDIO_IO* output);

  /**
   * Returns a const pointer to input connected to this chain. If no input
   * is connected, 0 is returned.
   */
  const AUDIO_IO* connected_output(void) const { return(output_id_repp); }

  /**
   * Disconnects output
   */
  void disconnect_output(void) { output_id_repp = 0; initialized_rep = false; }

  /**
   * Disconnects the sample buffer
   */
  void disconnect_buffer(void) { audioslot_repp = 0; initialized_rep = false; }

  /**
   * Clears chain (removes all chain operators and controllers)
   */
  void clear(void);

  /**
   * Adds the chain operator to the end of the chain
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
   * Removes the selected chain operator
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
   * Sets the parameter value (selected chain operator) 
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
   * Gets the parameter value (selected chain operator) 
   *
   * @param index parameter number
   *
   * require:
   *  index > 0 &&
   *  selected_chain_operator() != ""
   */
  CHAIN_OPERATOR::parameter_type get_parameter(int index) const;

  /**
   * Selects a chain operator
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
  int selected_chain_operator(void) const { return(selected_chainop_number_rep); }

  int number_of_chain_operators(void) const { return(chainops_rep.size()); }

  /**
   * Adds a generic controller and assign it to selected dynamic object
   *
   * require:
   *  gcontroller != 0
   *  selected_dynobj != 0
   */
  void add_controller(GENERIC_CONTROLLER* gcontroller);

  /**
   * Removes the selected controller
   *
   * require:
   *  selected_controller() <= number_of_controllers();
   *  selected_controller() > 0
   */
  void remove_controller(void);

  int number_of_controllers(void) const { return(gcontrollers_rep.size()); }

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
  int selected_controller(void) const { return(selected_controller_number_rep); }

  /**
   * Use current selected chain operator as 
   * target for parameters control.
   *
   * require:
   *   selected_chain_operator() != 0
   *
   * ensure:
   *   selected_target() == selected_chain_operator()
   */
  void selected_chain_operator_as_target(void);

  /**
   * Use current selected controller as 
   * target for parameter control.
   *
   * require:
   *   selected_controller() != 0
   *
   * ensure:
   *   selected_target() == selected_controller()
   */
  void selected_controller_as_target(void);

  /**
   * Returns the object that is the current target for 
   * parameter control, or 0 if none selected.
   */
  OPERATOR* selected_target(void) const { return(selected_dynobj_repp); }

  /**
   * Prepares chain for processing. All further processing
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
   * Processes chain data with all chain operators.
   *
   * require:
   *  is_initialized() == true
   */
  void process(void);

  /**
   * Calculates/fetches new values for all controllers.
   */
  void controller_update(void);

  /**
   * Re-initializes all effect parameters.
   */
  void refresh_parameters(void);

  /**
   * Converts chain to a formatted string.
   */
  string to_string(void) const;
  string chain_operator_to_string(CHAIN_OPERATOR* chainop) const;
  string controller_to_string(GENERIC_CONTROLLER* gctrl) const;

  CHAIN (void);
  virtual ~CHAIN (void);
};

#endif
