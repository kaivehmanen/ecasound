#ifndef _ECA_AUDIO_OBJECTS_H
#define _ECA_AUDIO_OBJECTS_H

#include <vector>
#include <map>
#include <string>

#include <kvutils/definition_by_contract.h>

#include "eca-chain.h"
#include "audioio.h"
#include "audioio-loop.h"

#include "eca-error.h"

/**
 * A specialized container class for representing a group of inputs, 
 * outputs and chains. Not meant for general use.
 * @author Kai Vehmanen
 */
class ECA_AUDIO_OBJECTS : public DEFINITION_BY_CONTRACT {

 private:

  bool double_buffering_rep;
  bool precise_sample_rates_rep;
  int output_openmode_rep;
  long int buffersize_rep;
  ECA_AUDIO_FORMAT default_audio_format_rep;

  AUDIO_IO* last_audio_object;
  map<int,LOOP_DEVICE*> loop_map;

  vector<double> input_start_pos;
  vector<double> output_start_pos;

  string options_inputs;
  string options_outputs;
  string options_chains;

  // ---
  // Setup data
  // ---
  vector<string> selected_chainids;

 protected:

  // ---
  // Setup to strings
  // ---
  string inputs_to_string(void) const;
  string outputs_to_string(void) const;
  string chains_to_string(void) const;
  string audioio_to_string(const AUDIO_IO* aiod, const string& direction) const;


  // ---
  // Setup helper functions
  // ---

 public:

  /**
   * Create a new audio object based on the formatted argument string
   *
   * require:
   *  argu.empty() != true
   */
  static AUDIO_IO* create_audio_object(const string& tname);

  /**
   * Create a new loop input object
   *
   * require:
   *  argu.empty() != true
   */
  AUDIO_IO* create_loop_input(const string& tname);

  /**
   * Create a new loop output object
   *
   * require:
   *  argu.empty() != true
   */
  AUDIO_IO* create_loop_output(const string& tname);

  /**
   * Print format and id information
   *
   * require:
   *   aio != 0
   */
  string audio_object_info(const AUDIO_IO* aio) const;

  /**
   * Adds a "default" chain to the setup.
   *
   * require:
   *   buffersize >= 0 && chains.size() == 0
   *
   * ensure:
   *   chains.back()->name() == "default" && 
   *   active_chainids.back() == "default"
   */
  void add_default_chain(void);

  /**
   * Handle audio-IO-devices and files.
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  void interpret_audioio_device (const string& argu) throw(ECA_ERROR*);

  /**
   * Add a new input object and attach it to selected chains.
   *
   * require:
   *   aiod != 0 && chains.size() > 0
   *
   * ensure:
   *   inputs.size() > 0
   */
  void add_input(AUDIO_IO* aiod);

  /**
   * Add a new output object and attach it to selected chains.
   *
   * require:
   *   aiod != 0 && chains.size() > 0
   *
   * ensure:
   *   outputs.size() > 0
   */
  void add_output(AUDIO_IO* aiod);
  void remove_audio_input(const string& label);
  void remove_audio_output(const string& label);
  void attach_input_to_selected_chains(const string& filename);
  void attach_output_to_selected_chains(const string& filename);

  void add_new_chains(const vector<string>& newchains);
  void remove_chains(void);
  void select_chains(const vector<string>& chains) { selected_chainids = chains; }
  void select_all_chains(void);
  const vector<string>& selected_chains(void) const { return(selected_chainids); }
  void clear_chains(void);
  void rename_chain(const string& name);
  void toggle_chain_muting(void);
  void toggle_chain_bypass(void);

  // ---
  // Status/info functions
  // ---
  vector<string> get_connected_chains_to_input(AUDIO_IO* aiod) const;
  vector<string> get_connected_chains_to_output(AUDIO_IO* aiod) const;
  vector<string> get_connected_chains_to_iodev(const string& filename) const;
  int number_of_connected_chains_to_input(AUDIO_IO* aiod) const;
  int number_of_connected_chains_to_output(AUDIO_IO* aiod) const;
  const CHAIN* get_chain_with_name(const string& name) const;

  // ---
  // Public data objects
  // ---
  vector<AUDIO_IO*> inputs;
  vector<AUDIO_IO*> outputs;
  vector<CHAIN*> chains;

  // ---
  // Protected data objects
  // ---

 public:

  void toggle_double_buffering(bool value) { double_buffering_rep = value; }
  void toggle_precise_sample_rates(bool value) { precise_sample_rates_rep = value; }
  void set_output_openmode(int value) { output_openmode_rep = value; }
  void set_buffersize(long int value) { buffersize_rep = value; }
  void set_default_audio_format(ECA_AUDIO_FORMAT& value) { default_audio_format_rep = value; }

  bool double_buffering(void) const { return(double_buffering_rep); }
  bool precise_sample_rates(void) const { return(precise_sample_rates_rep); }
  long int buffersize(void) const { return(buffersize_rep); }
  const ECA_AUDIO_FORMAT& default_audio_format(void) const { return(default_audio_format_rep); }
  int output_openmode(void) const { return(output_openmode_rep); }
  bool is_valid(void) const;

  /**
   * Constructor
   */
  ECA_AUDIO_OBJECTS(void);

  /**
   * Destructor
   */
  virtual ~ECA_AUDIO_OBJECTS(void);
};

#endif
