#ifndef _ECA_CHAINSETUP_H
#define _ECA_CHAINSETUP_H

#include <vector>
#include <string>
#include <map>

#include <kvutils/kvutils.h>

#include "eca-control-position.h"
/*  #include "eca-chainop-map.h" */
/*  #include "eca-controller-map.h" */
#include "eca-audio-objects.h"

#include "eca-error.h"

class CONTROLLER_SOURCE;
class CHAIN_OPERATOR;
class GENERIC_CONTROLLER;

/**
 * Class that represents a single chainsetup.
 * @author Kai Vehmanen
 */
class ECA_CHAINSETUP : public ECA_CONTROL_POSITION,
		       public ECA_AUDIO_OBJECTS {

 public:

  enum EP_MM_MODE { ep_mm_auto, ep_mm_simple, ep_mm_normal, ep_mm_mthreaded };

 private:

  string setup_name;
  string setup_filename;
  bool is_enabled_rep;
  enum EP_MM_MODE mixmode_rep;

  vector<string> options;
  string options_general;

  // ---
  // Setup to strings
  // ---
  void update_option_strings(void);
  string general_options_to_string(void) const;

  /**
   * Make sure that all option tokes start with a '-' sign
   */
  static vector<string> combine_options(const vector<string>& opts);

 public:

  /**
   * Set default values.
   */
  void set_defaults (void);

  void set_name(const string& str) { setup_name = str; }
  void set_filename(const string& str) { setup_filename = str; }

  /**
   * Handle options. 
   */
  void interpret_options(const vector<string>& opts);

  /**
   * Handle general options. 
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  void interpret_general_option (const string& argu);

  /**
   * Handle processing control
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  void interpret_processing_control (const string& argu);

  /**
   * Handle chainsetup options.
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  void interpret_audio_format (const string& argu);

  /**
   * Handle chain options.
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  void interpret_chains (const string& argu);

  /**
   * Handle chain operator options.
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  void interpret_chain_operator (const string& argu);

  /**
   * Create a new chain operator
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  CHAIN_OPERATOR* create_chain_operator (const string& argu);


  /**
   * Create a new LADSPA-plugin
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  CHAIN_OPERATOR* create_ladspa_plugin (const string& argu);

  /**
   * Handle controller sources and general controllers.
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  void interpret_controller (const string& argu);

  /**
   * Handle controller sources and general controllers.
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  GENERIC_CONTROLLER* create_controller (const string& argu);

  /**
   * Handle effect preset options.
   *
   * require:
   *  argu.size() > 0
   *  argu[0] == '-'
   */
  void interpret_effect_preset (const string& argu);

  /**
   * Enable chainsetup. Opens all devices and reinitializes all 
   * chain operators if necessary.
   * 
   * ensure:
   *   is_enabled() == false
   */
  void enable(void);

  /**
   * Disable chainsetup. Closes all devices. 
   * 
   * ensure:
   *   is_enabled() == false
   */
  void disable(void);

  /**
   * Checks whethers chainsetup is enabled (device ready for use).
   */
  bool is_enabled(void) const { return(is_enabled_rep); }

  /**
   * Add chain operator to selected chain.
   *
   * require:
   *   cotmp != 0
   *
   * ensure:
   *   selected_chains().size() == 1
   */
  void add_chain_operator(CHAIN_OPERATOR* cotmp);

  /**
   * Add general controller to selected chainop.
   *
   * require:
   *   csrc != 0
   */
  void add_controller(GENERIC_CONTROLLER* csrc);

  /**
   * Select controllers as targets for parameter control
   */
  void set_target_to_controller(void);

  // ---
  // Setup load/save functions
  // ---
  void load_from_file(const string& filename) throw(ECA_ERROR*);
  void save(void) throw(ECA_ERROR*);
  void save_to_file(const string& filename) throw(ECA_ERROR*);

  // ---
  // Chainsetup info
  // --

 public:

  void set_mixmode(enum EP_MM_MODE value) { mixmode_rep = value; }  

  const string& name(void) const { return(setup_name); }
  const string& filename(void) const { return(setup_filename); }
  enum EP_MM_MODE mixmode(void) const { return(mixmode_rep); }

  /**
   * Construct from a command line object.
   *
   * ensure:
   *   buffersize != 0
   */
  ECA_CHAINSETUP(COMMAND_LINE& cline);

  /**
   * Construct from a chainsetup file.
   *
   * ensure:
   *   buffersize != 0
   */
  ECA_CHAINSETUP(const string& setup_file, bool fromfile = true);
    
  /**
   * Destructor
   */
  virtual ~ECA_CHAINSETUP(void);
};

#endif
