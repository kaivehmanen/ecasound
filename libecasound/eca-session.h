#ifndef INCLUDED_ECA_SESSION_H
#define INCLUDED_ECA_SESSION_H

#include <vector>
#include <string>
#include <map>
#include <pthread.h>

#include <kvutils/kvutils.h>
#include <kvutils/com_line.h>

#include "samplebuffer.h"

#include "eca-resources.h"
#include "eca-error.h"

class AUDIO_IO;
class ECA_CHAIN;

#include "eca-chainsetup.h"

/**
 * Ecasound runtime setup and parameters.
 */
class ECA_SESSION {

 public:

  enum Engine_status { ep_status_running,
		       ep_status_stopped, 
		       ep_status_finished,
		       ep_status_notready };

  friend class ECA_CONTROL_BASE;
  friend class ECA_CONTROL_OBJECTS;
  friend class ECA_CONTROL;
  friend class ECA_PROCESSOR;

 private:

  ECA_RESOURCES ecaresources;

  // ---
  // Status data
  // ---
  Engine_status ep_status_rep;
  vector<ECA_CHAINSETUP*> chainsetups_rep;

  ECA_CHAINSETUP* connected_chainsetup_repp;
  ECA_CHAINSETUP* selected_chainsetup_repp;

  // ---
  // Setup interpretation
  // ---
  void set_defaults(void);

  void interpret_general_options(COMMAND_LINE& cline);
  void interpret_general_option (const string& argu);
  void interpret_chainsetup (const string& argu);
  void create_chainsetup_options(COMMAND_LINE& cline, vector<string>* options);
  bool is_session_option(const string& arg) const;

  // ---
  // Function for handling chainsetups
  // ---

  /**
   * Add a new chainsetup
   *
   * require:
   *  comline_setup != 0
   *
   * ensure:
   *  selected_chainsetup == comline_setup
   */
  void add_chainsetup(ECA_CHAINSETUP* comline_setup) throw(ECA_ERROR*);

  /**
   * Remove selected chainsetup
   *
   * require:
   *  connected_chainsetup != selected_chainsetup
   *
   * ensure:
   *  selected_chainsetup == 0
   */
  void remove_chainsetup(void);

  /**
   * Add a new chainsetup
   *
   * require:
   *  name.empty() != true
   *
   * ensure:
   *  selected_chainsetup->name() == name
   */
  void add_chainsetup(const string& name);

  /**
   * Select chainsetup with name 'name'
   *
   * require:
   *  name.empty() != true &&
   *
   * ensure:
   *  (selected_chainsetup->name() == name) ||
   *  (selected_chainsetup == 0)
   */
  void select_chainsetup(const string& name);

  /**
   * Save selected chainsetup
   *
   * require:
   *  selected_chainsetup != 0
   */
  void save_chainsetup(void) throw(ECA_ERROR*);

  /**
   * Save selected chainsetup to file 'filename'
   *
   * require:
   *  selected_chainsetup != 0 &&
   *  filename.empty() != true
   */
  void save_chainsetup(const string& filename) throw(ECA_ERROR*);

  /**
   * Load chainsetup from file "filename"
   *
   * require:
   *  filename.empty() != true
   *
   * ensure:
   *  selected_chainsetup->filename() == filename
   */
  void load_chainsetup(const string& filename) throw(ECA_ERROR*);

  /**
   * Connect selected chainsetup
   *
   * require:
   *  selected_chainsetup != 0 &&
   *  selected_chainsetup->is_valid()
   *
   * ensure:
   *  selected_chainsetup == connected_chainsetup
   */
  void connect_chainsetup(void);

  /**
   * Disconnect connected chainsetup
   *
   * require:
   *  connected_chainsetup != 0
   *
   * ensure:
   *  connected_chainsetup == 0
   */
  void disconnect_chainsetup(void);

  /**
   * Gets a vector of all chainsetup names.
   */
  vector<string> chainsetup_names(void) const;

 private:

  void update_controller_sources(void);
  void status(Engine_status status);

  // ---
  // Status/info functions
  // ---
  Engine_status status(void) const;

  vector<string> get_attached_chains_to_input(AUDIO_IO* aiod) const { return(selected_chainsetup_repp->get_attached_chains_to_input(aiod)); }
  vector<string> get_attached_chains_to_output(AUDIO_IO* aiod) const { return(selected_chainsetup_repp->get_attached_chains_to_output(aiod)); }
  int number_of_connected_chains_to_input(AUDIO_IO* aiod) const {
    return(connected_chainsetup_repp->number_of_attached_chains_to_input(aiod)); }
  int number_of_connected_chains_to_output(AUDIO_IO* aiod) const {
    return(connected_chainsetup_repp->number_of_attached_chains_to_output(aiod)); }

 private:

  // ---
  // Status data
  // ---
  bool iactive_rep;          // Should engine use 'cqueue'?
  bool multitrack_mode_rep;
  enum ECA_CHAINSETUP::Mix_mode mixmode_rep;
  bool raisepriority_rep;

  // --
  // Public/const routines
  // --
 public:

  const vector<ECA_CHAINSETUP*>& get_chainsetups(void) const { return chainsetups_rep; }
  const ECA_CHAINSETUP* get_selected_chainsetup(void) const { return selected_chainsetup_repp; }
  const ECA_CHAINSETUP* get_connected_chainsetup(void) const { return connected_chainsetup_repp; }
  const ECA_CHAINSETUP* get_chainsetup_with_name(const string& name) const;
  bool is_selected_chainsetup_connected(void) const { return(selected_chainsetup_repp == connected_chainsetup_repp); }

  bool is_interactive(void) const { return iactive_rep; }

  bool raised_priority(void) const { return(raisepriority_rep); }
  void toggle_raised_priority(bool value) { raisepriority_rep = value; }

 public:

  // --
  // Constructors and destructors
  // --
  ECA_SESSION(void);
  ECA_SESSION(COMMAND_LINE& cline) throw(ECA_ERROR*);
  ~ECA_SESSION(void);

 private:

  // --
  // Make sure that objects of this class aren't copy constucted/assigned
  // --
  ECA_SESSION (const ECA_SESSION& x) { }
  ECA_SESSION& operator=(const ECA_SESSION& x) { return *this; }
};

#endif
