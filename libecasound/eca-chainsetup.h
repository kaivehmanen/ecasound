#ifndef INCLUDED_ECA_CHAINSETUP_H
#define INCLUDED_ECA_CHAINSETUP_H

#include <vector>
#include <string>
#include <map>
#include <list>

#include "eca-chainsetup-bufparams.h"
#include "eca-chainsetup-position.h"
#include "eca-chainsetup-parser.h"
#include "eca-error.h"
#include "audio-stamp.h"
#include "midi-server.h"
#include "audioio-buffered-proxy.h"

class CONTROLLER_SOURCE;
class CHAIN_OPERATOR;
class GENERIC_CONTROLLER;
class AUDIO_IO;
class MIDI_IO;
class LOOP_DEVICE;
class CHAIN;

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
  ECA_CHAINSETUP(const vector<string>& options);
  ECA_CHAINSETUP(const string& setup_file);
  virtual ~ECA_CHAINSETUP(void);

  /*@}*/

  /** @name Functions for handling audio objects */
  /*@{*/

  void add_input(AUDIO_IO* aiod);
  void add_output(AUDIO_IO* aiod);
  void add_default_output(void);
  void remove_audio_input(const string& label);
  void remove_audio_output(const string& label);
  void attach_input_to_selected_chains(const AUDIO_IO* obj);
  void attach_output_to_selected_chains(const AUDIO_IO* obj);
  void audio_object_info(const AUDIO_IO* aio) const;
  bool ok_audio_object(const AUDIO_IO* aobj) const;
  bool is_realtime_target_output(int output_id) const;
  vector<string> audio_input_names(void) const;
  vector<string> audio_output_names(void) const;

  /*@}*/

  /** @name Functions for handling chains */
  /*@{*/

  void add_default_chain(void);
  void add_new_chains(const vector<string>& newchains);
  void remove_chains(void);
  void select_chains(const vector<string>& chains) { selected_chainids = chains; }
  void select_all_chains(void);
  void clear_chains(void);
  void rename_chain(const string& name);
  void toggle_chain_muting(void);
  void toggle_chain_bypass(void);

  const vector<string>& selected_chains(void) const { return(selected_chainids); }
  unsigned int first_selected_chain(void) const; 
  vector<string> chain_names(void) const;
  vector<string> get_attached_chains_to_iodev(const string& filename) const;
  const CHAIN* get_chain_with_name(const string& name) const;

  /*@}*/

  /** @name Functions for handling MIDI-objects */
  /*@{*/

  void add_midi_device(MIDI_IO* mididev);
  void remove_midi_device(const string& name);

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
  void set_default_audio_format(ECA_AUDIO_FORMAT& value) { default_audio_format_rep = value; }
  void set_default_midi_device(const string& name) { default_midi_device_rep = name; }
  void set_mixmode(Mix_mode_t value) { mixmode_rep = value; }
  void set_buffering_mode(Buffering_mode_t value);

  bool precise_sample_rates(void) const { return(precise_sample_rates_rep); }
  bool ignore_xruns(void) const { return(ignore_xruns_rep); }
  const ECA_AUDIO_FORMAT& default_audio_format(void) const { return(default_audio_format_rep); }
  const string& default_midi_device(void) const { return(default_midi_device_rep); }
  int output_openmode(void) const { return(output_openmode_rep); }
  Mix_mode_t mixmode(void) const { return(mixmode_rep); }
  Buffering_mode_t buffering_mode(void) const { return(buffering_mode_rep); }
  bool is_valid_for_connection(void) const;

  /*@}*/

  /** @name Functions for overriding current buffering mode parameters */
  /*@{*/

  void set_buffersize(long int value) { bmode_override_rep.set_buffersize(value); }
  void toggle_raised_priority(bool value) { bmode_override_rep.toggle_raised_priority(value); }
  void set_sched_priority(int value) { bmode_override_rep.set_sched_priority(value); }
  void toggle_double_buffering(bool value) { bmode_override_rep.toggle_double_buffering(value); }
  void set_double_buffer_size(long int v) { bmode_override_rep.set_double_buffer_size(v); }
  void toggle_max_buffers(bool v) { bmode_override_rep.toggle_max_buffers(v); }

  long int buffersize(void) const { return(bmode_active_rep.buffersize()); }
  bool raised_priority(void) const { return(bmode_active_rep.raised_priority()); }
  int sched_priority(void) const { return(bmode_active_rep.sched_priority()); }
  bool double_buffering(void) const { return(bmode_active_rep.double_buffering()); }
  long int double_buffer_size(void) const { return(bmode_active_rep.double_buffer_size()); }
  bool max_buffers(void) const { return(bmode_active_rep.max_buffers()); }

  /*@}*/

  /** @name Functions that modify current state  */
  /*@{*/

  void set_name(const string& str) { setup_name_rep = str; }
  void set_filename(const string& str) { setup_filename_rep = str; }
  void enable(void) throw(ECA_ERROR&);
  void disable(void);

  const string& name(void) const { return(setup_name_rep); }
  const string& filename(void) const { return(setup_filename_rep); }

  /*@}*/

  /** @name Functions implemented from ECA_CHAINSETUP_POSITION */
  /*@{*/

  virtual void seek_position(void);

  /*@}*/

  /** @name Functions for observing current state */
  /*@{*/

  /**
   * Checks whethers chainsetup is enabled (device ready for use).
   */
  bool is_enabled(void) const { return(is_enabled_rep); }
  bool is_valid(void) const;
  bool has_realtime_objects(void) const;

  /*@}*/

  /** @name Functions for string->state conversions */
  /*@{*/

  /**
   * Returns the result of last call to interpret_option(), interpret_global_option() 
   * or interpret_object_option().
   *
   * @result true if options interpreted succesfully, otherwise false
   */
  bool interpret_result(void) const { return(cparser_rep.interpret_result()); }
  const string& interpret_result_verbose(void) const { return(cparser_rep.interpret_result_verbose()); }

  void interpret_option(const string& arg);
  void interpret_global_option(const string& arg);
  void interpret_object_option(const string& arg);
  void interpret_options(vector<string>& opts);

  /*@}*/

  /** @name Functions for string<->state conversions */
  /*@{*/

  void save(void) throw(ECA_ERROR&);
  void save_to_file(const string& filename) throw(ECA_ERROR&);

 private:

  /*@}*/

  /** @name Configuration data (settings and values)  */
  /*@{*/

  bool precise_sample_rates_rep;
  bool ignore_xruns_rep;
  bool rtcaps_rep;
  int output_openmode_rep;
  long int double_buffer_size_rep;
  string default_midi_device_rep;
  ECA_AUDIO_FORMAT default_audio_format_rep;

  /*@}*/

  /** @name Current setup data (internal state, objects) */
  /*@{*/

  bool is_enabled_rep;
  bool multitrack_mode_rep;
  bool memory_locked_rep;
  int active_chain_index_rep;
  int active_chainop_index_rep;
  int active_chainop_param_index_rep;
  int proxy_clients_rep;
  string setup_name_rep;
  string setup_filename_rep;
  vector<string> selected_chainids;
  std::map<int,LOOP_DEVICE*> loop_map;
  vector<double> input_start_pos;
  vector<double> output_start_pos;
  Mix_mode_t mixmode_rep;
  Buffering_mode_t buffering_mode_rep;
  Buffering_mode_t active_buffering_mode_rep;

  vector<AUDIO_IO*> inputs;
  vector<AUDIO_IO*> inputs_direct_rep;
  vector<AUDIO_IO*> outputs;
  vector<AUDIO_IO*> outputs_direct_rep;
  vector<CHAIN*> chains;
  vector<MIDI_IO*> midi_devices;
  list<AUDIO_IO*> aobj_garbage_rep;

  /*@}*/

  /** @name Aggregate objects */
  /*@{*/

  AUDIO_STAMP_SERVER stamp_server_rep;
  ECA_CHAINSETUP_PARSER cparser_rep;
  AUDIO_IO_PROXY_SERVER pserver_rep;
  MIDI_SERVER midi_server_rep;

  ECA_CHAINSETUP_BUFPARAMS bmode_active_rep;
  ECA_CHAINSETUP_BUFPARAMS bmode_override_rep;
  ECA_CHAINSETUP_BUFPARAMS bmode_nonrt_rep;
  ECA_CHAINSETUP_BUFPARAMS bmode_rt_rep;
  ECA_CHAINSETUP_BUFPARAMS bmode_rtlowlatency_rep;

  /** @name Functions for handling audio objects */
  /*@{*/

  AUDIO_IO* add_audio_object_helper(AUDIO_IO* aio);
  void remove_audio_object_helper(AUDIO_IO* aio);

  /** @name Functions for state<->string conversions */
  /*@{*/

  void load_from_file(const string& filename, vector<string>& opts) const throw(ECA_ERROR&);

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
  int number_of_chain_operators(void) const;

  /*@}*/

  /** @name Private helper functions */
  /*@{*/

  vector<string> get_attached_chains_to_input(AUDIO_IO* aiod) const;
  vector<string> get_attached_chains_to_output(AUDIO_IO* aiod) const;
  int number_of_attached_chains_to_input(AUDIO_IO* aiod) const;
  int number_of_attached_chains_to_output(AUDIO_IO* aiod) const;
  bool ok_audio_object_helper(const AUDIO_IO* aobj, const vector<AUDIO_IO*>& aobjs) const;
  void add_chain_helper(const string& name);

  /*@}*/

};

#endif
