#ifndef _ECA_AUDIO_OBJECTS_H
#define _ECA_AUDIO_OBJECTS_H

#include <vector>
#include <string>

#include "eca-chain.h"
#include "audioio.h"

#include "eca-error.h"

/**
 * A specialized container class for representing a group of inputs, 
 * outputs and chains. Not meant for general use.
 * @author Kai Vehmanen
 */
class ECA_AUDIO_OBJECTS {

 public:

  enum {
    TYPE_NOT_OPENED =   -1,
    TYPE_UNKNOWN =       0,
    TYPE_WAVE =          1,
    TYPE_DART =          2,
    TYPE_CDR =           3,
    TYPE_OSS =           4,
    TYPE_EWF =           5,
    TYPE_ESS =           6,
    TYPE_OSSDMA =        7,
    TYPE_MP3 =           8,
    TYPE_ALSA =          9,
    TYPE_ALSAFILE =     10,
    TYPE_ALSALOOPBACK = 16,
    TYPE_AUDIOFILE =    11,
    TYPE_RAWFILE =      12,
    TYPE_STDIN =        13,
    TYPE_STDOUT =       14,
    TYPE_NULL =         15,
    TYPE_RTNULL =       18,
    TYPE_MIKMOD =       17
  };

 private:

  bool double_buffering_rep;
  bool precise_sample_rates_rep;
  SIMODE output_openmode_rep;
  long int buffersize_rep;
  ECA_AUDIO_FORMAT default_audio_format_rep;

  AUDIO_IO* last_audio_object;
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
   * Recognise file format from the file name.
   */
  static int get_type_from_extension (const string& teksti);

  /**
   * Create a new audio object based on given arguments.
   *
   * require:
   *  argu.empty() != true &&
   *  buffersize > 0
   */
  AUDIO_IO* create_audio_object(const string& tname, const SIMODE mode, const ECA_AUDIO_FORMAT& format, long int buffersize_rep) const throw(ECA_ERROR*);

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
   */
  void interpret_audioio_device (const string& argu, const string& argu_param);

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
  void set_output_openmode(SIMODE value) { output_openmode_rep = value; }
  void set_buffersize(long int value) { buffersize_rep = value; }
  void set_default_audio_format(ECA_AUDIO_FORMAT& value) { default_audio_format_rep = value; }

  bool double_buffering(void) const { return(double_buffering_rep); }
  bool precise_sample_rates(void) const { return(precise_sample_rates_rep); }
  long int buffersize(void) const { return(buffersize_rep); }
  const ECA_AUDIO_FORMAT& default_audio_format(void) const { return(default_audio_format_rep); }
  const SIMODE output_openmode(void) const { return(output_openmode_rep); }
  bool is_valid(void) const;

  /**
   * Constructor
   */
  ECA_AUDIO_OBJECTS(void);

  /**
   * Destructor
   */
  ~ECA_AUDIO_OBJECTS(void);
};

#endif



