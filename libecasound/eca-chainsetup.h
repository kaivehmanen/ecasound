#ifndef INCLUDED_ECA_CHAINSETUP_H
#define INCLUDED_ECA_CHAINSETUP_H

#include <vector>
#include <string>
#include <map>

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
 *        ECA_CHAINSETUP_PARSER and ECA_PROCESSOR.
 *        In addition, to ease implementation, 
 *        ECA_CONTROL classes have direct access
 *        to ECA_CHAINSETUP's implementation.
 *
 * @author Kai Vehmanen
 */
class ECA_CHAINSETUP : public ECA_CHAINSETUP_POSITION {

 public:

  friend class ECA_PROCESSOR;
  friend class ECA_CONTROL;
  friend class ECA_CONTROL_BASE;
  friend class ECA_CONTROL_OBJECTS;
  friend class ECA_CHAINSETUP_PARSER;

  // --
  // type definitions and constants

  enum Mix_mode { ep_mm_auto, ep_mm_simple, ep_mm_normal, ep_mm_mthreaded };
  typedef enum Mix_mode Mix_mode_t;

  // --
  // functions for init and cleanup

  ECA_CHAINSETUP(void);
  ECA_CHAINSETUP(const std::vector<std::string>& options);
  ECA_CHAINSETUP(const std::string& setup_file);
  virtual ~ECA_CHAINSETUP(void);

  // --
  // functions for handling audio objects

  void add_input(AUDIO_IO* aiod);
  void add_output(AUDIO_IO* aiod);
  void add_default_output(void);
  void remove_audio_input(const std::string& label);
  void remove_audio_output(const std::string& label);

  void attach_input_to_selected_chains(const AUDIO_IO* obj);
  void attach_output_to_selected_chains(const AUDIO_IO* obj);
  void audio_object_info(const AUDIO_IO* aio) const;
  bool is_slave_output(AUDIO_IO* aiod) const;
  std::vector<std::string> audio_input_names(void) const;
  std::vector<std::string> audio_output_names(void) const;

  // --
  // functions for handling chains

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
  std::vector<std::string> get_attached_chains_to_input(AUDIO_IO* aiod) const;
  std::vector<std::string> get_attached_chains_to_output(AUDIO_IO* aiod) const;
  std::vector<std::string> get_attached_chains_to_iodev(const std::string& filename) const;
  int number_of_attached_chains_to_input(AUDIO_IO* aiod) const;
  int number_of_attached_chains_to_output(AUDIO_IO* aiod) const;
  const CHAIN* get_chain_with_name(const std::string& name) const;

  // --
  // functions for handling MIDI-objects

  void add_midi_device(MIDI_IO* mididev);
  void remove_midi_device(const std::string& name);

  // --
  // functions for chain operators

  void add_chain_operator(CHAIN_OPERATOR* cotmp);
  void add_controller(GENERIC_CONTROLLER* csrc);
  void set_target_to_controller(void);

  // --
  // functions for configuration (default values, settings)

  void toggle_double_buffering(bool value) { double_buffering_rep = value; }
  void set_double_buffer_size(long int v) { double_buffer_size_rep = v; }
  void toggle_precise_sample_rates(bool value) { precise_sample_rates_rep = value; }
  void toggle_ignore_xruns(bool v) { ignore_xruns_rep = v; }
  void toggle_max_buffers(bool v) { max_buffers_rep = v; }
  void set_output_openmode(int value) { output_openmode_rep = value; }
  void set_buffersize(long int value) { buffersize_rep = value; }
  void set_default_audio_format(ECA_AUDIO_FORMAT& value) { default_audio_format_rep = value; }
  void set_default_midi_device(const std::string& name) { default_midi_device_rep = name; }
  void set_mixmode(Mix_mode_t value) { mixmode_rep = value; }  

  bool double_buffering(void) const { return(double_buffering_rep); }
  long int double_buffer_size(void) const { return(double_buffer_size_rep); }
  bool precise_sample_rates(void) const { return(precise_sample_rates_rep); }
  bool ignore_xruns(void) const { return(ignore_xruns_rep); }
  bool max_buffers(void) const { return(max_buffers_rep); }
  long int buffersize(void) const { return(buffersize_rep); }
  const ECA_AUDIO_FORMAT& default_audio_format(void) const { return(default_audio_format_rep); }
  const std::string& default_midi_device(void) const { return(default_midi_device_rep); }
  int output_openmode(void) const { return(output_openmode_rep); }
  bool is_valid_for_connection(void) const;
  Mix_mode_t mixmode(void) const { return(mixmode_rep); }

  // --
  // functions that modify current state

  void set_name(const std::string& str) { setup_name_rep = str; }
  void set_filename(const std::string& str) { setup_filename_rep = str; }
  void enable(void) throw(ECA_ERROR&);
  void disable(void);
  void change_position_exact(double seconds);
  void set_position_exact(double seconds);

  const std::string& name(void) const { return(setup_name_rep); }
  const std::string& filename(void) const { return(setup_filename_rep); }

  // --
  // functions for observing current state

  /**
   * Checks whethers chainsetup is enabled (device ready for use).
   */
  bool is_enabled(void) const { return(is_enabled_rep); }
  bool is_valid(void) const;
  bool has_realtime_objects(void) const;

  // --
  // functions for string->state conversions

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
  void interpret_options(vector<string>& opts);

  // --
  // functions for string<->state conversions

  void save(void) throw(ECA_ERROR&);
  void save_to_file(const std::string& filename) throw(ECA_ERROR&);

 private:

  // --
  // configuration data (settings and values)

  bool double_buffering_rep;
  bool precise_sample_rates_rep;
  bool ignore_xruns_rep;
  bool max_buffers_rep;
  int output_openmode_rep;
  long int double_buffer_size_rep;
  long int buffersize_rep;
  std::string default_midi_device_rep;
  ECA_AUDIO_FORMAT default_audio_format_rep;


  // --
  // current setup data (internal state, objects)

  bool is_enabled_rep;
  std::string setup_name_rep;
  std::string setup_filename_rep;
  std::vector<std::string> selected_chainids;
  std::map<int,LOOP_DEVICE*> loop_map;
  std::vector<double> input_start_pos;
  std::vector<double> output_start_pos;
  Mix_mode_t mixmode_rep;
  std::vector<AUDIO_IO*> inputs;
  std::vector<AUDIO_IO*> outputs;
  std::vector<CHAIN*> chains;
  std::vector<MIDI_IO*> midi_devices;
  int proxy_clients_rep;

  // --
  // aggregate objects

  AUDIO_STAMP_SERVER stamp_server_rep;
  ECA_CHAINSETUP_PARSER cparser_rep;
  AUDIO_IO_PROXY_SERVER pserver_rep;
  MIDI_SERVER midi_server_rep;

  // --
  // functions for handling audio objects

  AUDIO_IO* add_audio_object_helper(AUDIO_IO* aio);
  AUDIO_IO* remove_audio_object_helper(AUDIO_IO* aio);

  // --
  // functions for state<->string conversions

  void load_from_file(const std::string& filename, std::vector<std::string>& opts) const throw(ECA_ERROR&);

  // --
  // functions for init and cleanup

  void set_defaults (void);

};

#endif
