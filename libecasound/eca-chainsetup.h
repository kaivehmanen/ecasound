#ifndef INCLUDED_ECA_CHAINSETUP_H
#define INCLUDED_ECA_CHAINSETUP_H

#include <vector>
#include <string>
#include <map>
#include <list>

#include "eca-chainsetup-position.h"
#include "eca-chainsetup-parser.h"
#include "eca-error.h"

class CONTROLLER_SOURCE;
class CHAIN_OPERATOR;
class GENERIC_CONTROLLER;
class AUDIO_IO;
class AUDIO_IO_MANAGER;
class MIDI_IO;
class LOOP_DEVICE;
class CHAIN;
class AUDIO_IO_PROXY_SERVER;
class MIDI_SERVER;
class ECA_AUDIO_FORMAT;
class ECA_CHAINSETUP_BUFPARAMS;
class ECA_CHAINSETUP_impl;

/**
 * Class representing an ecasound chainsetup object.
 *
 * Chainsetup is the central data object. It contains 
 * audio inputs, outputs, chains, operators and also 
 * information about how they are connected.
 * 
 * Notes: ECA_CHAINSETUP is closely coupled to the 
 *        ECA_CHAINSETUP_PARSER and ECA_ENGINE.
 *        In addition, to ease implementation, 
 *        also ECA_CONTROL classes have direct access
 *        to ECA_CHAINSETUP's implementation.
 *
 * @author Kai Vehmanen
 */
class ECA_CHAINSETUP : public ECA_CHAINSETUP_POSITION {

 public:

  friend class ECA_ENGINE;
  friend class ECA_CONTROL;
  friend class ECA_CONTROL_BASE;
  friend class ECA_CONTROL_OBJECTS;
  friend class ECA_CHAINSETUP_PARSER;

  /** @name Public type definitions and constants */
  /*@{*/

  enum Mix_mode { ep_mm_auto, ep_mm_simple, ep_mm_normal, ep_mm_mthreaded };
  enum Buffering_mode { cs_bmode_auto, cs_bmode_nonrt, cs_bmode_rt, cs_bmode_rtlowlatency, cs_bmode_none };

  typedef enum Mix_mode Mix_mode_t;
  typedef enum Buffering_mode Buffering_mode_t;

  /*@}*/

  /** @name Functions for init and cleanup */
  /*@{*/

  ECA_CHAINSETUP(void);
  ECA_CHAINSETUP(const std::vector<std::string>& options);
  ECA_CHAINSETUP(const std::string& setup_file);
  virtual ~ECA_CHAINSETUP(void);

  /*@}*/

  /** @name Functions for handling audio objects */
  /*@{*/

  void add_input(AUDIO_IO* aiod);
  void add_output(AUDIO_IO* aiod);
  void add_default_output(void);
  void remove_audio_input(const std::string& label);
  void remove_audio_output(const std::string& label);
  void attach_input_to_selected_chains(const AUDIO_IO* obj);
  void attach_output_to_selected_chains(const AUDIO_IO* obj);
  bool ok_audio_object(const AUDIO_IO* aobj) const;
  bool is_realtime_target_output(int output_id) const;
  std::vector<std::string> audio_input_names(void) const;
  std::vector<std::string> audio_output_names(void) const;

  static void audio_object_info(const AUDIO_IO* aio);


  /*@}*/

  /** @name Functions for handling chains */
  /*@{*/

  void add_default_chain(void);
  void add_new_chains(const std::vector<std::string>& newchains);
  void remove_chains(void);
  void select_chains(const std::vector<std::string>& chains) { selected_chainids = chains; }
  void select_all_chains(void);
  void clear_chains(void);
  void rename_chain(const std::string& name);
  void toggle_chain_muting(void);
  void toggle_chain_bypass(void);

  const std::vector<std::string>& selected_chains(void) const { return(selected_chainids); }
  unsigned int first_selected_chain(void) const; 
  std::vector<std::string> chain_names(void) const;
  std::vector<std::string> get_attached_chains_to_iodev(const std::string& filename) const;
  const CHAIN* get_chain_with_name(const std::string& name) const;

  /*@}*/

  /** @name Functions for handling MIDI-objects */
  /*@{*/

  void add_midi_device(MIDI_IO* mididev);
  void remove_midi_device(const std::string& name);

  /*@}*/

  /** @name Functions for chain operators */
  /*@{*/

  void add_chain_operator(CHAIN_OPERATOR* cotmp);
  void add_controller(GENERIC_CONTROLLER* csrc);
  void set_target_to_controller(void);

  /*@}*/

  /** @name Functions for configuration (default values, settings) */
  /*@{*/

  void toggle_precise_sample_rates(bool value) { precise_sample_rates_rep = value; }
  void toggle_ignore_xruns(bool v) { ignore_xruns_rep = v; }
  void set_output_openmode(int value) { output_openmode_rep = value; }
  void set_default_audio_format(ECA_AUDIO_FORMAT& value);
  void set_default_midi_device(const std::string& name) { default_midi_device_rep = name; }
  void set_mixmode(Mix_mode_t value) { mixmode_rep = value; }
  void set_buffering_mode(Buffering_mode_t value);

  bool precise_sample_rates(void) const { return(precise_sample_rates_rep); }
  bool ignore_xruns(void) const { return(ignore_xruns_rep); }
  const ECA_AUDIO_FORMAT& default_audio_format(void) const;
  const std::string& default_midi_device(void) const { return(default_midi_device_rep); }
  int output_openmode(void) const { return(output_openmode_rep); }
  Mix_mode_t mixmode(void) const { return(mixmode_rep); }
  Buffering_mode_t buffering_mode(void) const { return(buffering_mode_rep); }
  bool is_valid_for_connection(void) const;

  /*@}*/

  /** @name Functions for overriding current buffering mode parameters */
  /*@{*/

  void set_buffersize(long int value);
  void toggle_raised_priority(bool value);
  void set_sched_priority(int value);
  void toggle_double_buffering(bool value);
  void set_double_buffer_size(long int v);
  void toggle_max_buffers(bool v);

  long int buffersize(void) const;
  bool raised_priority(void) const;
  int sched_priority(void) const;
  bool double_buffering(void) const;
  long int double_buffer_size(void) const;
  bool max_buffers(void) const;

  /*@}*/

  /** @name Functions that modify current state  */
  /*@{*/

  void set_name(const std::string& str) { setup_name_rep = str; }
  void set_filename(const std::string& str) { setup_filename_rep = str; }
  void enable(void) throw(ECA_ERROR&);
  void disable(void);

  const std::string& name(void) const { return(setup_name_rep); }
  const std::string& filename(void) const { return(setup_filename_rep); }

  /*@}*/

  /** @name Functions implemented from ECA_SAMPLERATE_AWARE */
  /*@{*/

  virtual void set_samples_per_second(SAMPLE_SPECS::sample_rate_t v);

  /*@}*/

  /** @name Functions implemented from ECA_AUDIO_POSITION */
  /*@{*/

  virtual void seek_position(void);

  /*@}*/

  /** @name Functions for observing current state */
  /*@{*/

  /**
   * Checks whethers chainsetup is enabled (device ready for use).
   */
  bool is_enabled(void) const { return(is_enabled_rep); }
  bool is_in_use(void) const { return(is_in_use_rep); }
  bool is_valid(void) const;
  bool has_realtime_objects(void) const;
  bool has_nonrealtime_objects(void) const;
  std::string options_to_string(void) const;

  /*@}*/

  /** @name Functions for std::string->state conversions */
  /*@{*/

  /**
   * Returns the result of last call to interpret_option(), interpret_global_option() 
   * or interpret_object_option().
   *
   * @result true if options interpreted succesfully, otherwise false
   */
  bool interpret_result(void) const { return(cparser_rep.interpret_result()); }
  const std::string& interpret_result_verbose(void) const { return(cparser_rep.interpret_result_verbose()); }

  void interpret_option(const std::string& arg);
  void interpret_global_option(const std::string& arg);
  void interpret_object_option(const std::string& arg);
  void interpret_options(std::vector<std::string>& opts);

  /*@}*/

  /** @name Functions for std::string<->state conversions */
  /*@{*/

  void save(void) throw(ECA_ERROR&);
  void save_to_file(const std::string& filename) throw(ECA_ERROR&);

 private:

  /*@}*/

  /** @name Configuration data (settings and values)  */
  /*@{*/
  
  ECA_CHAINSETUP_impl* impl_repp;
  ECA_CHAINSETUP_PARSER cparser_rep;

  bool precise_sample_rates_rep;
  bool ignore_xruns_rep;
  bool rtcaps_rep;
  int output_openmode_rep;
  long int double_buffer_size_rep;
  std::string default_midi_device_rep;

  /*@}*/

  /** @name Current setup data (internal state, objects) */
  /*@{*/

  bool is_in_use_rep;
  bool is_enabled_rep;
  bool multitrack_mode_rep;
  bool multitrack_mode_override_rep;
  bool memory_locked_rep;
  int active_chain_index_rep;
  int active_chainop_index_rep;
  int active_chainop_param_index_rep;
  int proxy_clients_rep;
  std::string setup_name_rep;
  std::string setup_filename_rep;
  std::vector<std::string> selected_chainids;
  std::map<int,LOOP_DEVICE*> loop_map;
  std::vector<double> input_start_pos;
  std::vector<double> output_start_pos;
  Mix_mode_t mixmode_rep;
  Buffering_mode_t buffering_mode_rep;
  Buffering_mode_t active_buffering_mode_rep;

  std::vector<AUDIO_IO*> inputs;
  std::vector<AUDIO_IO*> inputs_direct_rep;
  std::vector<AUDIO_IO*> outputs;
  std::vector<AUDIO_IO*> outputs_direct_rep;
  std::vector<AUDIO_IO_MANAGER*> aio_managers_rep;
  std::vector<CHAIN*> chains;
  std::vector<MIDI_IO*> midi_devices;
  std::list<AUDIO_IO*> aobj_garbage_rep;

  AUDIO_IO_PROXY_SERVER* pserver_repp;
  MIDI_SERVER* midi_server_repp;

  /*@}*/


  /** @name Functions for handling audio objects */
  /*@{*/

  AUDIO_IO_MANAGER* get_audio_object_manager(AUDIO_IO* aio) const;
  AUDIO_IO_MANAGER* get_audio_object_type_manager(AUDIO_IO* aio) const;
  void register_audio_object_to_manager(AUDIO_IO* aio);
  void unregister_audio_object_from_manager(AUDIO_IO* aio);
  AUDIO_IO* add_audio_object_helper(AUDIO_IO* aio);
  void remove_audio_object_helper(AUDIO_IO* aio);

  /** @name Functions for state<->string conversions */
  /*@{*/

  void load_from_file(const std::string& filename, std::vector<std::string>& opts) const throw(ECA_ERROR&);

  /*@}*/

  /** @name Functions for internal state changes */
  /*@{*/

  void select_active_buffering_mode(void);
  void enable_active_buffering_mode(void);
  void switch_to_direct_mode(void);
  void switch_to_direct_mode_helper(std::vector<AUDIO_IO*>* objs, const std::vector<AUDIO_IO*>& directobjs);
  void switch_to_proxy_mode(void);
  void switch_to_proxy_mode_helper(std::vector<AUDIO_IO*>* objs, const std::vector<AUDIO_IO*>& directobjs);
  void lock_all_memory(void);
  void unlock_all_memory(void);
  void set_defaults (void);
  int number_of_realtime_inputs(void) const;
  int number_of_realtime_outputs(void) const;
  int number_of_non_realtime_inputs(void) const;
  int number_of_non_realtime_outputs(void) const;
  int number_of_chain_operators(void) const;
  void toggle_is_in_use(bool value) { is_in_use_rep=value; }

  /*@}*/

  /** @name Private helper functions */
  /*@{*/

  const ECA_CHAINSETUP_BUFPARAMS& active_buffering_parameters(void) const;
  const ECA_CHAINSETUP_BUFPARAMS& override_buffering_parameters(void) const;
  std::vector<std::string> get_attached_chains_to_input(AUDIO_IO* aiod) const;
  std::vector<std::string> get_attached_chains_to_output(AUDIO_IO* aiod) const;
  int number_of_attached_chains_to_input(AUDIO_IO* aiod) const;
  int number_of_attached_chains_to_output(AUDIO_IO* aiod) const;
  void add_chain_helper(const std::string& name);
  void enable_audio_object_helper(AUDIO_IO* aobj) const;

  /*@}*/

  /** @name Static private helper functions */
  /*@{*/

  static bool ok_audio_object_helper(const AUDIO_IO* aobj, const std::vector<AUDIO_IO*>& aobjs);
  static void check_object_samplerate(const AUDIO_IO* obj,
				      SAMPLE_SPECS::sample_rate_t srate) throw(ECA_ERROR&);

  /*@}*/

};

#endif
