#ifndef INCLUDED_CHAIN_H
#define INCLUDED_CHAIN_H

#include <string>
#include <vector>

#include "eca-chainop.h"
#include "eca-audio-position.h"

class GENERIC_CONTROLLER;
class OPERATOR;
class SAMPLE_BUFFER;

/**
 * Class representing an abstract audio signal chain.
 */
class CHAIN : public ECA_AUDIO_POSITION {
 
 public:

  CHAIN (void);
  virtual ~CHAIN (void);

  bool is_initialized(void) const { return initialized_rep; }

  /**
   * Is chain muted? If muted, audio buffers are zeroed during
   * processing.
   */ 
  bool is_muted(void) const { return muted_rep; }

  /**
   * Is processing enabled? If disabled, all chain operators
   * will be skipped during processing. 
   */
  bool is_processing(void) const { return sfx_rep; }

  void toggle_muting(bool v) { muted_rep = v; }
  void toggle_processing(bool v) { sfx_rep = v; }

  std::string name(void) const { return chainname_rep; }
  void name(const std::string& c) { chainname_rep = c; }

  bool is_valid(void) const;

  void connect_input(int input);
  void disconnect_input(void);
  void connect_output(int output);
  void disconnect_output(void);
  void disconnect_buffer(void);

  /**
   * Returns an id number to input connected to this chain. If no input
   * is connected, -1 is returned.
   */
  int connected_input(void) const { return input_id_rep; }

  /**
   * Returns an id number to output connected to this chain. If no input
   * is connected, -1 is returned.
   */
  int connected_output(void) const { return output_id_rep; }

  void clear(void);
  void add_chain_operator(CHAIN_OPERATOR* chainop);
  void remove_chain_operator(void);
  void select_chain_operator(int index);
  void select_chain_operator_parameter(int index);
  void set_parameter(CHAIN_OPERATOR::parameter_t value);

  /**
   * Index of selected chain operator
   */
  int selected_chain_operator(void) const { return selected_chainop_number_rep; }
  int selected_chain_operator_parameter(void) const { return selected_chainop_parameter_rep; }
  int number_of_chain_operators(void) const { return chainops_rep.size(); }
  int number_of_chain_operator_parameters(void) const;
  CHAIN_OPERATOR::parameter_t get_parameter(void) const;
  std::string chain_operator_name(void) const;
  std::string chain_operator_parameter_name(void) const;
  const CHAIN_OPERATOR* get_chain_operator(int index) const { return chainops_rep[index]; }
  const CHAIN_OPERATOR* get_selected_chain_operator(void) const { return selected_chainop_repp; }

  void add_controller(GENERIC_CONTROLLER* gcontroller);
  void remove_controller(void);
  void select_controller(int index);
  void select_controller_parameter(int index);
  void set_controller_parameter(CHAIN_OPERATOR::parameter_t value);
  const GENERIC_CONTROLLER* get_controller(int index) const { return gcontrollers_rep[index]; }
  const GENERIC_CONTROLLER* get_selected_controller(void) const { return selected_controller_repp; }
  int number_of_controller_parameters(void) const;
  std::string controller_parameter_name(void) const;
  CHAIN_OPERATOR::parameter_t get_controller_parameter(void) const;

  /**
   * Index of selected controller
   */
  int selected_controller(void) const { return selected_controller_number_rep; }
  int selected_controller_parameter(void) const { return selected_controller_parameter_rep; }
  int number_of_controllers(void) const { return gcontrollers_rep.size(); }
  std::string controller_name(void) const;

  void selected_chain_operator_as_target(void);

  void selected_controller_as_target(void);

  /**
   * Returns the object that is the current target for 
   * parameter control, or 0 if none selected.
   */
  OPERATOR* selected_target(void) const { return selected_dynobj_repp; }

  void init(SAMPLE_BUFFER* sbuf = 0, int in_channels = 0, int out_channels = 0);
  void release(void);
  void process(void);
  void controller_update(void);
  void refresh_parameters(void);

  std::string to_string(void) const;

  /** @name Functions implemented from ECA_SAMPLERATE_AWARE */
  /*@{*/

  virtual void set_samples_per_second(SAMPLE_SPECS::sample_rate_t v);

  /*@}*/

  /** @name Functions implemented from ECA_AUDIO_POSITION */
  /*@{*/

  virtual void seek_position(void);

  /*@}*/

 private:

  bool initialized_rep;
  std::string chainname_rep;
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

  int input_id_rep;
  int output_id_rep;

  SAMPLE_BUFFER* audioslot_repp;

};

#endif
