#ifndef _ECA_CHAINSETUP_H
#define _ECA_CHAINSETUP_H

#include <vector>
#include <string>
#include <map>

#include <kvutils/kvutils.h>

#include "eca-control-position.h"
#include "eca-chainop-map.h"
#include "eca-controller-map.h"
#include "eca-audio-objects.h"

#include "eca-error.h"

class CONTROLLER_SOURCE;
class CHAIN_OPERATOR;
class ECA_RESOURCES;

/**
 * Class that represents a single chainsetup.
 * @author Kai Vehmanen
 */
class ECA_CHAINSETUP : public ECA_CONTROL_POSITION,
		       public ECA_AUDIO_OBJECTS,
                       public ECA_CONTROLLER_MAP {

 public:

  enum EP_MM_MODE { ep_mm_auto, ep_mm_simple, ep_mm_normal, ep_mm_mthreaded };

 private:

  string setup_name;
  string setup_filename;
  bool is_enabled_rep;
  enum EP_MM_MODE mixmode_rep;

  ECA_RESOURCES* ecaresources;

  vector<string> options;
  string options_general;

  // ---
  // Setup to strings
  // ---
  void update_option_strings(void);
  string general_options_to_string(void) const;

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
   */
  void interpret_general_option (const string& argu);

  /**
   * Handle processing control
   */
  void interpret_processing_control (const string& argu);

  /**
   * Handle chainsetup options.
   */
  void interpret_audio_format (const string& argu);

  /**
   * Handle chain options.
   */
  void interpret_chains (const string& argu);

  /**
   * Handle chain operator options.
   */
  void interpret_chain_operator (const string& argu);

  /**
   * Handle controller sources and general controllers.
   */
  void interpret_controller (const string& argu);

  /**
   * Handle effect preset options.
   */
  void interpret_effect_preset (const string& argu);

  /**
   * Process a singlechain effect preset.
   */
  void add_singlechain_preset(const string& preset_name) throw(ECA_ERROR*);

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
  bool is_enabled(void) { return(is_enabled_rep); }

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
   * require:
   *   ecarc != 0
   *
   * ensure:
   *   buffersize != 0
   */
  ECA_CHAINSETUP(ECA_RESOURCES* ecarc, COMMAND_LINE& cline);

  /**
   * Construct from a chainsetup file.
   *
   * require:
   *   ecarc != 0
   *
   * ensure:
   *   buffersize != 0
   */
  ECA_CHAINSETUP(ECA_RESOURCES* ecarc, const string& setup_file, bool fromfile = true);
    
  /**
   * Destructor
   */
  virtual ~ECA_CHAINSETUP(void);
};

#endif
