#ifndef INCLUDED_ECA_CHAINSETUP_H
#define INCLUDED_ECA_CHAINSETUP_H

#include <vector>
#include <string>
#include <map>

#include "eca-chainsetup-position.h"
#include "eca-audio-objects.h"
#include "eca-error.h"
#include "audio-stamp.h"

class CONTROLLER_SOURCE;
class CHAIN_OPERATOR;
class GENERIC_CONTROLLER;
class AUDIO_IO;

/**
 * Class that represents a single chainsetup.
 * @author Kai Vehmanen
 */
class ECA_CHAINSETUP : public ECA_CHAINSETUP_POSITION,
		       public ECA_AUDIO_OBJECTS {

 public:

  enum Mix_mode { ep_mm_auto, ep_mm_simple, ep_mm_normal, ep_mm_mthreaded };

 private:

  string setup_name_rep;
  string setup_filename_rep;
  bool is_enabled_rep;
  bool istatus_rep;
  enum Mix_mode mixmode_rep;
  AUDIO_IO* last_audio_object;
  AUDIO_STAMP_SERVER stamp_server_rep;

  vector<string> options;
  string options_general;

  void update_option_strings(void);
  string general_options_to_string(void) const;

  void set_defaults (void);
  void preprocess_options(vector<string>& opts);

  void interpret_general_option (const string& arg);
  void interpret_processing_control (const string& arg);
  void interpret_audio_format (const string& arg);
  void interpret_chains (const string& arg);
  void interpret_chain_operator (const string& arg);
  void interpret_controller (const string& arg);
  void interpret_effect_preset (const string& arg);
  void interpret_audioio_device (const string& argu) throw(ECA_ERROR&);

 public:

  void set_name(const string& str) { setup_name_rep = str; }
  void set_filename(const string& str) { setup_filename_rep = str; }

  void enable(void) throw(ECA_ERROR&);
  void disable(void);

  CHAIN_OPERATOR* create_chain_operator (const string& arg);
  CHAIN_OPERATOR* create_ladspa_plugin (const string& arg);
  CHAIN_OPERATOR* create_vst_plugin (const string& arg);
  GENERIC_CONTROLLER* create_controller (const string& arg);

  void interpret_option(const string& arg) throw(ECA_ERROR&);
  void interpret_global_option(const string& arg);
  void interpret_object_option(const string& arg) throw(ECA_ERROR&);
  void interpret_options(vector<string>& opts) throw(ECA_ERROR&);

  /**
   * Returns the result of last call to interpret_option(), interpret_global_option() 
   * or interpret_object_option().
   *
   * @result true if options interpreted succesfully, otherwise false
   */
  bool interpretation_status(void) const { return(istatus_rep); }

  /**
   * Checks whethers chainsetup is enabled (device ready for use).
   */
  bool is_enabled(void) const { return(is_enabled_rep); }

  void add_chain_operator(CHAIN_OPERATOR* cotmp);
  void add_controller(GENERIC_CONTROLLER* csrc);
  void add_default_output(void);
  void set_target_to_controller(void);

  // ---
  // Setup load/save functions
  // ---
  void load_from_file(const string& filename) throw(ECA_ERROR&);
  void save(void) throw(ECA_ERROR&);
  void save_to_file(const string& filename) throw(ECA_ERROR&);

  // ---
  // Chainsetup info
  // --

  void set_mixmode(enum Mix_mode value) { mixmode_rep = value; }  

  const string& name(void) const { return(setup_name_rep); }
  const string& filename(void) const { return(setup_filename_rep); }
  enum Mix_mode mixmode(void) const { return(mixmode_rep); }

  ECA_CHAINSETUP(void);
  ECA_CHAINSETUP(const vector<string>& options);
  ECA_CHAINSETUP(const string& setup_file);
  virtual ~ECA_CHAINSETUP(void);
};

#endif
