#ifndef INCLUDED_ECA_CONTROL_OBJECTS_H
#define INCLUDED_ECA_CONTROL_OBJECTS_H

#include "eca-control-base.h"
#include "audioio.h"

class CHAIN_OPERATOR;

/**
 * Class for configuring ecasound library objects
 * @author Kai Vehmanen
 */
class ECA_CONTROL_OBJECTS : public ECA_CONTROL_BASE {

  AUDIO_IO* selected_audio_object_repp;

 protected:

  AUDIO_IO* selected_audio_input_repp;
  AUDIO_IO* selected_audio_output_repp;

  void audio_input_as_selected(void);
  void audio_output_as_selected(void);
  void rewind_audio_object(double seconds);
  void forward_audio_object(double seconds);
  void set_audio_object_position(double seconds);
  void set_audio_object_position_samples(long int samples);
  void wave_edit_audio_object(void);

  void send_chain_commands_to_engine(int command, double value);

 public:

  // -------------------------------------------------------------------
  // Chainsetups  (if not specified, selected chainsetup is used)
  // -------------------------------------------------------------------

  void add_chainsetup(const std::string& name);
  void remove_chainsetup(void);
  void load_chainsetup(const std::string& filename);
  void save_chainsetup(const std::string& filename);
  void select_chainsetup(const std::string& name);
  void select_chainsetup_by_index(int index);
  void edit_chainsetup(void);
  void connect_chainsetup(void);
  void disconnect_chainsetup(void);

  std::string selected_chainsetup(void) const;
  std::string connected_chainsetup(void) const;

  void change_chainsetup_position(double seconds);
  void set_chainsetup_position(double seconds);

  double chainsetup_position(double seconds) const;
  const ECA_CHAINSETUP* get_chainsetup(void) const;
  const ECA_CHAINSETUP* get_chainsetup_filename(const std::string& filename) const;
  std::vector<std::string> chainsetup_names(void) const;
  const std::string& chainsetup_filename(void) const;
  long int chainsetup_buffersize(void) const;

  void set_chainsetup_filename(const std::string& name);
  void set_chainsetup_parameter(const std::string& name);
  void set_chainsetup_sample_format(const std::string& name);
  void set_chainsetup_processing_length_in_seconds(double value);
  void set_chainsetup_processing_length_in_samples(long int value);
  void set_chainsetup_output_mode(int output_mode);
  void toggle_chainsetup_looping(void);

  // -------------------------------------------------------------------
  // Chains (if not specified, active chainsetup is used)
  // -------------------------------------------------------------------

  void add_chain(const std::string& names);
  void add_chains(const std::string& names);
  void add_chains(const std::vector<std::string>& names);
  void remove_chains(void);
  void select_chains_by_index(const std::vector<int>& index_numbers);
  void select_chain(const std::string& chain);
  void select_chains(const std::vector<std::string>& chains);
  void deselect_chains(const std::vector<std::string>& chains);
  void select_all_chains(void);

  const std::vector<std::string>& selected_chains(void) const;
  std::vector<std::string> chain_names(void) const;
  const CHAIN* get_chain(void) const;

  void clear_chains(void);
  void rename_chain(const std::string& name);
  void toggle_chain_muting(void);
  void toggle_chain_bypass(void);
  void rewind_chains(double pos_in_seconds);
  void forward_chains(double pos_in_seconds);
  void set_position_chains(double pos_in_seconds);
  void change_position_chains(double change_in_seconds);

  // -------------------------------------------------------------------
  // Audio-devices  (active chainsetup is edited)
  // -------------------------------------------------------------------

  void add_audio_input(const std::string& filename);
  void remove_audio_input(void);
  void attach_audio_input(void);
  void select_audio_input(const std::string& name);
  void select_audio_input_by_index(int index);

  void add_audio_output(const std::string& filename);
  void add_default_output(void);
  void remove_audio_output(void);
  void attach_audio_output(void);
  void select_audio_output(const std::string& name);
  void select_audio_output_by_index(int index);
  void set_default_audio_format(const std::string& sfrm, int channels, long int srate, bool interleaving);
  void set_default_audio_format(const ECA_AUDIO_FORMAT& format);
  void set_default_audio_format_to_selected_input(void);
  void set_default_audio_format_to_selected_output(void);

  const AUDIO_IO* get_audio_input(void);
  std::vector<std::string> audio_input_names(void) const;

  const AUDIO_IO* get_audio_output(void);
  std::vector<std::string> audio_output_names(void) const;

  const ECA_AUDIO_FORMAT& default_audio_format(void) const;
  ECA_AUDIO_FORMAT get_audio_format(AUDIO_IO* aobj) const;

  // -------------------------------------------------------------------
  // Chain operators (currently selected chainsetup and chains)
  // -------------------------------------------------------------------

  void add_chain_operator(const std::string& chainop_params);
  void add_chain_operator(CHAIN_OPERATOR* cotmp);
  void remove_chain_operator(void);
  void select_chain_operator(int chainop_id);
  void select_chain_operator_parameter(int param);
  void set_chain_operator_parameter(CHAIN_OPERATOR::parameter_type value);
  void add_controller(const std::string& gcontrol_params);
  void select_controller(int ctrl_id);
  void remove_controller(void);

  int selected_chain_operator(void) const;
  int selected_chain_operator_parameter(void) const;
  int selected_controller(void) const;

  const CHAIN_OPERATOR* get_chain_operator(void) const;
  CHAIN_OPERATOR::parameter_type get_chain_operator_parameter(void) const;
  const GENERIC_CONTROLLER* get_controller(void) const;

  std::vector<std::string> chain_operator_names(void) const;
  std::vector<std::string> chain_operator_parameter_names(void) const;
  std::vector<std::string> controller_names(void) const;

  // -------------------------------------------------------------------
  // Constructors and destructors
  // -------------------------------------------------------------------

  ECA_CONTROL_OBJECTS (ECA_SESSION* psession);
  ~ECA_CONTROL_OBJECTS (void) { }
};

#endif
