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

  std::vector<CHAIN_OPERATOR*> chainops_rep;
  std::vector<GENERIC_CONTROLLER*> gcontrollers_rep;

  CHAIN_OPERATOR* selected_chainop_repp;
  GENERIC_CONTROLLER* selected_controller_repp;
  OPERATOR* selected_dynobj_repp;

  int selected_chainop_number_rep;
  int selected_chainop_parameter_rep;
  int selected_controller_number_rep;
  int selected_controller_parameter_rep;

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
  void name(const std::string& c) { chainname_rep = c; }

  bool is_valid(void) const;

  void connect_input(AUDIO_IO* input);
  void disconnect_input(void);
  void connect_output(AUDIO_IO* output);
  void disconnect_output(void);
  void disconnect_buffer(void);

  /**
   * Returns a const pointer to input connected to this chain. If no input
   * is connected, 0 is returned.
   */
  const AUDIO_IO* connected_input(void) const { return(input_id_repp); }

  /**
   * Returns a const pointer to input connected to this chain. If no input
   * is connected, 0 is returned.
   */
  const AUDIO_IO* connected_output(void) const { return(output_id_repp); }

  void clear(void);
  void add_chain_operator(CHAIN_OPERATOR* chainop);
  void remove_chain_operator(void);
  void select_chain_operator(int index);
  void select_chain_operator_parameter(int index);
  void set_parameter(CHAIN_OPERATOR::parameter_type value);

  /**
   * Index of selected chain operator
   */
  int selected_chain_operator(void) const { return(selected_chainop_number_rep); }
  int selected_chain_operator_parameter(void) const { return(selected_chainop_parameter_rep); }
  int number_of_chain_operators(void) const { return(chainops_rep.size()); }
  int number_of_chain_operator_parameters(void) const;
  CHAIN_OPERATOR::parameter_type get_parameter(void) const;
  string chain_operator_name(void) const;
  string chain_operator_parameter_name(void) const;

  void add_controller(GENERIC_CONTROLLER* gcontroller);
  void remove_controller(void);
  void select_controller(int index);
  void select_controller_parameter(int index);

  /**
   * Index of selected chain operator
   */
  int selected_controller(void) const { return(selected_controller_number_rep); }
  int selected_controller_parameter(void) const { return(selected_controller_parameter_rep); }
  int number_of_controllers(void) const { return(gcontrollers_rep.size()); }
  std::string controller_name(void) const;
  void selected_chain_operator_as_target(void);

  void selected_controller_as_target(void);

  /**
   * Returns the object that is the current target for 
   * parameter control, or 0 if none selected.
   */
  OPERATOR* selected_target(void) const { return(selected_dynobj_repp); }

  void init(SAMPLE_BUFFER* sbuf, int in_channels = 0, int out_channels = 0);
  void process(void);
  void controller_update(void);
  void refresh_parameters(void);

  string to_string(void) const;
  string chain_operator_to_string(CHAIN_OPERATOR* chainop) const;
  string controller_to_string(GENERIC_CONTROLLER* gctrl) const;

  CHAIN (void);
  virtual ~CHAIN (void);
};

#endif
