#ifndef INCLUDED_ECA_AUDIO_OBJECTS_H
#define INCLUDED_ECA_AUDIO_OBJECTS_H

#include <vector>
#include <map>
#include <string>

#include <kvutils/definition_by_contract.h>
#include "eca-audio-format.h"
#include "midi-server.h"
#include "eca-error.h"

class MIDI_IO;
class AUDIO_IO;
class LOOP_DEVICE;
class CHAIN;

/**
 * A specialized container class for representing a group of inputs, 
 * outputs and chains. Not meant for general use.
 * @author Kai Vehmanen
 */
class ECA_AUDIO_OBJECTS : public DEFINITION_BY_CONTRACT {

 private:

  bool double_buffering_rep;
  bool precise_sample_rates_rep;
  bool ignore_xruns_rep;
  bool max_buffers_rep;
  long int double_buffer_size_rep;
  int output_openmode_rep;
  long int buffersize_rep;
  string default_midi_device_rep;

  map<int,LOOP_DEVICE*> loop_map;

  string options_inputs;
  string options_outputs;
  string options_chains;

  // ---
  // Setup data
  // ---
  vector<string> selected_chainids;

 protected:

  ECA_AUDIO_FORMAT default_audio_format_rep;
  vector<double> input_start_pos;
  vector<double> output_start_pos;

  // ---
  // Setup to strings
  // ---
  string inputs_to_string(void) const;
  string outputs_to_string(void) const;
  string chains_to_string(void) const;
  string midi_to_string(void) const;
  string audioio_to_string(const AUDIO_IO* aiod, const string& direction) const;

 public:

  // ---
  // Setup helper functions
  // ---

  AUDIO_IO* create_loop_input(const string& arg);
  AUDIO_IO* create_loop_output(const string& arg);

  void audio_object_info(const AUDIO_IO* aio) const;
  void add_default_chain(void);
  void add_input(AUDIO_IO* aiod);
  void add_output(AUDIO_IO* aiod);
  void remove_audio_input(const string& label);
  void remove_audio_output(const string& label);
  void attach_input_to_selected_chains(const AUDIO_IO* obj);
  void attach_output_to_selected_chains(const AUDIO_IO* obj);

  void add_new_chains(const vector<string>& newchains);
  void remove_chains(void);
  void select_chains(const vector<string>& chains) { selected_chainids = chains; }
  void select_all_chains(void);
  void clear_chains(void);
  void rename_chain(const string& name);
  void toggle_chain_muting(void);
  void toggle_chain_bypass(void);

  void add_midi_device(MIDI_IO* mididev);
  void remove_midi_device(const string& name);

  const vector<string>& selected_chains(void) const { return(selected_chainids); }
  unsigned int first_selected_chain(void) const; 
  vector<string> chain_names(void) const;

  // ---
  // Status/info functions
  // ---
  vector<string> get_attached_chains_to_input(AUDIO_IO* aiod) const;
  vector<string> get_attached_chains_to_output(AUDIO_IO* aiod) const;
  vector<string> get_attached_chains_to_iodev(const string& filename) const;
  int number_of_attached_chains_to_input(AUDIO_IO* aiod) const;
  int number_of_attached_chains_to_output(AUDIO_IO* aiod) const;
  const CHAIN* get_chain_with_name(const string& name) const;

  // ---
  // Public data objects
  // ---
  vector<AUDIO_IO*> inputs;
  vector<AUDIO_IO*> outputs;
  vector<CHAIN*> chains;
  vector<MIDI_IO*> midi_devices;
  MIDI_SERVER midi_server_rep;

  // ---
  // Protected data objects
  // ---

 public:

  void toggle_double_buffering(bool value) { double_buffering_rep = value; }
  void set_double_buffer_size(long int v) { double_buffer_size_rep = v; }
  void toggle_precise_sample_rates(bool value) { precise_sample_rates_rep = value; }
  void toggle_ignore_xruns(bool v) { ignore_xruns_rep = v; }
  void toggle_max_buffers(bool v) { max_buffers_rep = v; }
  void set_output_openmode(int value) { output_openmode_rep = value; }
  void set_buffersize(long int value) { buffersize_rep = value; }
  void set_default_audio_format(ECA_AUDIO_FORMAT& value) { default_audio_format_rep = value; }
  void set_default_midi_device(const string& name) { default_midi_device_rep = name; }

  bool double_buffering(void) const { return(double_buffering_rep); }
  long int double_buffer_size(void) const { return(double_buffer_size_rep); }
  bool precise_sample_rates(void) const { return(precise_sample_rates_rep); }
  bool ignore_xruns(void) const { return(ignore_xruns_rep); }
  bool max_buffers(void) const { return(max_buffers_rep); }
  long int buffersize(void) const { return(buffersize_rep); }
  const ECA_AUDIO_FORMAT& default_audio_format(void) const { return(default_audio_format_rep); }
  const string& default_midi_device(void) const { return(default_midi_device_rep); }
  int output_openmode(void) const { return(output_openmode_rep); }
  bool is_valid(void) const;

  ECA_AUDIO_OBJECTS(void);
  virtual ~ECA_AUDIO_OBJECTS(void);
};

#endif
