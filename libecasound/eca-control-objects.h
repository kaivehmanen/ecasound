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
  void wave_edit_audio_object(void);

  void send_chain_commands_to_engine(int command, double value);

 public:

  // -------------------------------------------------------------------
  // Chainsetups  (if not specified, selected chainsetup is used)
  // -------------------------------------------------------------------

  void add_chainsetup(const string& name);
  void remove_chainsetup(void);
  void load_chainsetup(const string& filename);
  void save_chainsetup(const string& filename);
  void select_chainsetup(const string& name);
  void select_chainsetup_by_index(int index);
  void edit_chainsetup(void);
  void connect_chainsetup(void);
  void disconnect_chainsetup(void);

  string selected_chainsetup(void) const;
  string connected_chainsetup(void) const;

  void change_chainsetup_position(double seconds);
  void set_chainsetup_position(double seconds);

  double chainsetup_position(double seconds) const;
  ECA_CHAINSETUP* get_chainsetup(void) const;
  ECA_CHAINSETUP* get_chainsetup_filename(const string& filename) const;
  vector<string> chainsetup_names(void) const;
  const string& chainsetup_filename(void) const;

  void set_chainsetup_filename(const string& name);
  void set_chainsetup_parameter(const string& name);
  void set_chainsetup_sample_format(const string& name);
  void set_chainsetup_processing_length_in_seconds(double value);
  void set_chainsetup_processing_length_in_samples(long int value);
  void set_chainsetup_output_mode(int output_mode);
  void toggle_chainsetup_looping(void);

  // -------------------------------------------------------------------
  // Chains (if not specified, active chainsetup is used)
  // -------------------------------------------------------------------

  void add_chain(const string& names);
  void add_chains(const string& names);
  void add_chains(const vector<string>& names);
  void remove_chains(void);
  void select_chains_by_index(const vector<int>& index_numbers);
  void select_chain(const string& chain);
  void select_chains(const vector<string>& chains);
  void deselect_chains(const vector<string>& chains);
  void select_all_chains(void);

  const vector<string>& selected_chains(void) const;
  vector<string> chain_names(void) const;
  CHAIN* get_chain(void) const;

  void clear_chains(void);
  void rename_chain(const string& name);
  void toggle_chain_muting(void);
  void toggle_chain_bypass(void);
  void rewind_chains(double pos_in_seconds);
  void forward_chains(double pos_in_seconds);
  void set_position_chains(double pos_in_seconds);
  void change_position_chains(double change_in_seconds);

  // -------------------------------------------------------------------
  // Audio-devices  (active chainsetup is edited)
  // -------------------------------------------------------------------

  void add_audio_input(const string& filename);
  void remove_audio_input(void);
  void attach_audio_input(void);
  void select_audio_input(const string& name);
  void select_audio_input_by_index(int index);

  void add_audio_output(const string& filename);
  void add_default_output(void);
  void remove_audio_output(void);
  void attach_audio_output(void);
  void select_audio_output(const string& name);
  void select_audio_output_by_index(int index);
  void set_default_audio_format(const string& sfrm, int channels, long int srate, bool interleaving);
  void set_default_audio_format(const ECA_AUDIO_FORMAT& format);

  AUDIO_IO* get_audio_input(void) const;
  vector<string> audio_input_names(void) const;

  AUDIO_IO* get_audio_output(void) const;
  vector<string> audio_output_names(void) const;

  ECA_AUDIO_FORMAT get_audio_format(AUDIO_IO* aobj) const;

  // -------------------------------------------------------------------
  // Chain operators (currently selected chainsetup and chains)
  // -------------------------------------------------------------------

  void add_chain_operator(const string& chainop_params);
  void add_chain_operator(CHAIN_OPERATOR* cotmp);
  void remove_chain_operator(void);
  void select_chain_operator(int chainop_id);
  void select_chain_operator_parameter(int param);
  void set_chain_operator_parameter(CHAIN_OPERATOR::parameter_type value);
  void add_controller(const string& gcontrol_params);
  void select_controller(int ctrl_id);
  void remove_controller(void);

  CHAIN_OPERATOR* get_chain_operator(void) const;
  CHAIN_OPERATOR::parameter_type get_chain_operator_parameter(void) const;
  GENERIC_CONTROLLER* get_controller(void) const;

  // -------------------------------------------------------------------
  // Constructors and destructors
  // -------------------------------------------------------------------

  ECA_CONTROL_OBJECTS (ECA_SESSION* psession);
  ~ECA_CONTROL_OBJECTS (void) { }
};

#endif
