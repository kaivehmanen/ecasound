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
  bool istatus_rep; /* whether we have found an option match? */
  enum Mix_mode mixmode_rep;
  AUDIO_IO* last_audio_object;
  AUDIO_STAMP_SERVER stamp_server_rep;
  bool interpret_result_rep; /* whether we found an option match with correct format? */
  string interpret_result_verbose_rep;

  std::vector<std::string> options;
  std::string options_general;

  void update_option_strings(void);
  std::string general_options_to_string(void) const;

  void set_defaults (void);
  void preprocess_options(std::vector<std::string>& opts);

  void interpret_entry(void);
  void interpret_exit(const std::string& arg);
  void interpret_set_result(bool result, const std::string& verbose) { interpret_result_rep = result; interpret_result_verbose_rep = verbose; }

  void interpret_general_option (const std::string& arg);
  void interpret_processing_control (const std::string& arg);
  void interpret_audio_format (const std::string& arg);
  void interpret_chains (const std::string& arg);
  void interpret_chain_operator (const std::string& arg);
  void interpret_controller (const std::string& arg);
  void interpret_effect_preset (const std::string& arg);
  void interpret_audioio_device (const std::string& argu);
  void interpret_midi_device (const std::string& arg);

  bool interpret_match_found(void) const { return(istatus_rep); }

 public:

  void set_name(const std::string& str) { setup_name_rep = str; }
  void set_filename(const std::string& str) { setup_filename_rep = str; }

  void enable(void) throw(ECA_ERROR&);
  void disable(void);

  void change_position_exact(double seconds);
  void set_position_exact(double seconds);

  CHAIN_OPERATOR* create_chain_operator (const std::string& arg);
  CHAIN_OPERATOR* create_ladspa_plugin (const std::string& arg);
  CHAIN_OPERATOR* create_vst_plugin (const std::string& arg);
  GENERIC_CONTROLLER* create_controller (const std::string& arg);

  /**
   * Returns the result of last call to interpret_option(), interpret_global_option() 
   * or interpret_object_option().
   *
   * @result true if options interpreted succesfully, otherwise false
   */
  bool interpret_result(void) const { return(interpret_result_rep); }
  const std::string& interpret_result_verbose(void) const { return(interpret_result_verbose_rep); }

  void interpret_option(const std::string& arg);
  void interpret_global_option(const std::string& arg);
  void interpret_object_option(const std::string& arg);
  void interpret_options(vector<string>& opts);

  /**
   * Checks whethers chainsetup is enabled (device ready for use).
   */
  bool is_enabled(void) const { return(is_enabled_rep); }
  bool is_valid(void) const;

  void add_chain_operator(CHAIN_OPERATOR* cotmp);
  void add_controller(GENERIC_CONTROLLER* csrc);
  void add_default_output(void);
  void set_target_to_controller(void);

  // ---
  // Setup load/save functions
  // ---
  void load_from_file(const std::string& filename) throw(ECA_ERROR&);
  void save(void) throw(ECA_ERROR&);
  void save_to_file(const std::string& filename) throw(ECA_ERROR&);

  // ---
  // Chainsetup info
  // --

  void set_mixmode(enum Mix_mode value) { mixmode_rep = value; }  

  const std::string& name(void) const { return(setup_name_rep); }
  const std::string& filename(void) const { return(setup_filename_rep); }
  enum Mix_mode mixmode(void) const { return(mixmode_rep); }

  ECA_CHAINSETUP(void);
  ECA_CHAINSETUP(const std::vector<std::string>& options);
  ECA_CHAINSETUP(const std::string& setup_file);
  virtual ~ECA_CHAINSETUP(void);
};

#endif
