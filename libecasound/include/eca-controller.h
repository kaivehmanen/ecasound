#ifndef _ECA_CONTROLLER_H
#define _ECA_CONTROLLER_H

#include <pthread.h>

#include <kvutils/definition_by_contract.h>

#include "audioio.h"
#include "eca-chain.h"
#include "eca-session.h"
#include "dynamic-parameters.h"
#include "eca-iamode-parser.h"

class CHAIN_OPERATOR;
class ECA_CHAINSETUP;

extern string ecasound_lockfile;

enum { ECA_QUIT = 1 };

/**
 * Class for controlling the whole ecasound library.
 * @author Kai Vehmanen
 */
class ECA_CONTROLLER : public ECA_IAMODE_PARSER,
                       public DEFINITION_BY_CONTRACT {

  ECA_SESSION* session_rep;
  ECA_CHAINSETUP* selected_chainsetup_rep;
  CHAIN_OPERATOR* selected_chainop_rep;
  AUDIO_IO* selected_audio_object_rep;

  int retcode;
  pthread_t th_cqueue;
  bool engine_started;

  string selected_audio_object;

  /**
   * Start the processing engine
   *
   * require:
   *  is_connected() == true
   *
   * ensure:
   *  is_engine_started() == true
   */
  void start_engine(bool ignore_lock = false);

  /**
   * Close the processing engine
   *
   * ensure:
   *  is_engine_started() == false
   */
  void close_engine(void);

  void send_chain_commands_to_engine(int command, double value);

 public:
  
  /** 
   * See ECA_IAMODE_PARSER
   */
  void action(int action_id, const vector<string>& args) throw(ECA_ERROR*);

  // -------------------------------------------------------------------
  // Runtime control
  // -------------------------------------------------------------------

  /**
   * Start the processing engine
   *
   * require:
   *  is_connected() == true
   *
   * ensure:
   *  is_engine_started() == true
   */
  void start(bool ignore_lock = false);

  /**
   * Stop the processing engine
   *
   * require:
   *  is_engine_started() == true
   *
   * ensure:
   *   is_running() == false
   */
  void stop(void);

  /**
   * Stop the processing engine and throw an ECA_QUIT exception.
   */
  void quit(void);

  // -------------------------------------------------------------------
  // Session info / global resources (~/.ecasoundrc)
  // -------------------------------------------------------------------

  /**
   * Get resource values from ~/.ecasoundrc
   */
  const string& resource_value(const string& key) { return session_rep->ecaresources.resource(key); }

  // -------------------------------------------------------------------
  // Session info / output to ecadebug
  // -------------------------------------------------------------------
  
  void print_general_status(void);

  // -------------------------------------------------------------------
  // Session info / functions
  // -------------------------------------------------------------------
  
  /**
   * Get a string containing a comma separated list of all chains 
   * attached to input with index 'aiod'. 
   */
  string connected_chains_input(AUDIO_IO* aiod) const;

  /**
   * Get a string containing a comma separated list of all chains 
   * attached to output with index 'aiod'. 
   */
  string connected_chains_output(AUDIO_IO* aiod) const;

  /**
   * Get a string containing a comma separated list of all chains 
   * attached to audio object with name 'filename'. 
   */
  vector<string> connected_chains(const string& name) const;

  // -------------------------------------------------------------------
  // Session info / master input
  // -------------------------------------------------------------------

  long length_in_samples(void) const { return session_rep->length_in_samples(); }
  double length_in_seconds_exact(void) const { return session_rep->length_in_seconds_exact(); }
  long position_in_samples(void) const { return session_rep->position_in_samples(); }
  double position_in_seconds_exact(void) const { return session_rep->position_in_seconds_exact(); }

  // -------------------------------------------------------------------
  // Modify session
  // -------------------------------------------------------------------

  void toggle_interactive_mode(bool v) { session_rep->iactive = v; } 
  void toggle_multitrack_mode(bool v) { session_rep->multitrack_mode = v; } 

  /**
   * Set the default buffersize (in samples).
   *
   * require:
   *   is_editable() == true
   */
  void set_buffersize(int bsize);

  /**
   * Toggle whether raised priority mode is enabled or not.
   *
   * require:
   *   is_editable() == true
   */
  void toggle_raise_priority(bool v);

  // -------------------------------------------------------------------
  // Session status
  // -------------------------------------------------------------------

  /**
   * Returns true if processing engine is running.
   */
  bool is_running(void) const;

  /**
   * Returns true if active chainsetup exists and is connected.
   */
  bool is_connected(void) const;

  /**
   * Returns true if some chainsetup is selected.
   */
  bool is_selected(void) const;

  /**
   * Is currently selected chainsetup valid?
   *
   * require:
   *  is_selected()
   */
  bool is_valid(void) const;

  /**
   * Returns true if engine has been started. 
   */
  bool is_engine_started(void) const { return(engine_started); }

  /**
   * Return info about engine status.
   */
  string engine_status(void) const;

  // -------------------------------------------------------------------
  // Chainsetups  (if not specified, selected chainsetup is used)
  // -------------------------------------------------------------------

  /**
   * Add a new chainsetup.
   *
   * @param name chainsetup name 
   *
   * require:
   *  name != ""
   *
   * ensure:
   *  selected_chainsetup() == name
   */
  void add_chainsetup(const string& name);

  /**
   * Remove chainsetup.
   *
   * @param name chainsetup name 
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *
   * ensure:
   *  selected_chainsetup.empty() == true
   */
  void remove_chainsetup(void);

  /**
   * Load chainsetup from file 'filename'.
   *
   * @param name chainsetup filename 
   *
   * require:
   *  filename != ""
   *
   * ensure:
   *  loaded chainsetup == selected_chainsetup()
   */
  void load_chainsetup(const string& filename);

  /**
   * Save selected chainsetup.
   *
   * @param filename chainsetup filename (if omitted, previously used filename will be used, if any)
   *
   * require:
   *  selected_chainsetup().empty() != true
   */
  void save_chainsetup(const string& filename);

  /**
   * Select chainsetup
   *
   * @param name chainsetup name 
   *
   * require:
   *   name != ""
   *
   * ensure:
   *  name == selected_chainsetup() ||
   *  selected_chainsetup_rep == 0
   */
  void select_chainsetup(const string& name);


  /**
   * Select chainsetup by index (see chainsetup_status())
   *
   * @param name chainsetup name 
   *
   * require:
   *  index.empty() != true
   *  index[0] == 'c'
   *
   * ensure:
   *  selected_chainsetup_rep == 0
   */
  void select_chainsetup_by_index(const string& index);

  /**
   * Name of currently active chainsetup.
   */
  string selected_chainsetup(void) const;

  /**
   * Spawns an external editor for editing selected chainsetup
   *
   * require:
   *  is_selected() 
   *  connected_chainsetup() != selected_chainsetup()
   */
  void edit_chainsetup(void);

  /**
   * Connect selected chainsetup.
   *
   * require:
   *  is_selected() == true
   *  is_valid() == true
   *
   * ensure:
   *  is_connected() == true
   */
  void connect_chainsetup(void);

  /**
   * Name of connected chainsetup.
   */
  string connected_chainsetup(void) const;

  /**
   * Disconnect activate chainsetup.
   *
   * require:
   *  is_connected() == true
   *
   * ensure:
   *  connected_chainsetup() == ""
   */
  void disconnect_chainsetup(void);

  /**
   * Get a const pointer to chainsetup 'name'.
   */
  const ECA_CHAINSETUP* get_chainsetup(const string& name) const;

  /**
   * Get a const pointer to chainsetup with filename 'filename'.
   */
  const ECA_CHAINSETUP* get_chainsetup_filename(const string& filename) const;

  /** 
   * Get chainsetup filename (used by save_chainsetup())
   *
   * require:
   *  is_selected() == true
   */
  const string& chainsetup_filename(void) const;

  /**
   * Set chainsetup filename (used by save_chainsetup())
   *
   * require:
   *  is_selected() == true && 
   *  name.empty() != true
   */
  void set_chainsetup_filename(const string& name);

  /**
   * Set general chainsetup chainsetup parameter
   *
   * require:
   *  is_selected() == true && 
   *  name.empty() != true
   */
  void set_chainsetup_parameter(const string& name);

  /**
   * Set general chainsetup chainsetup parameter
   *
   * require:
   *  is_selected() == true && 
   *  name.empty() != true
   */
  void set_chainsetup_sample_format(const string& name);

  /**
   * Return info about chainsetups
   */
  string chainsetup_status(void) const;

  /**
   * Set processing length in seconds
   *
   * require:
   *  is_selected() == true
   *  value > 0
   */
  void set_chainsetup_processing_length_in_seconds(double value);

  /**
   * Set processing length in samples
   *
   * require:
   *  is_selected() == true
   *  value > 0
   */
  void set_chainsetup_processing_length_in_samples(long int value);

  /**
   * Toggle chainsetup looping
   *
   * require:
   *  is_selected() == true
   */
  void toggle_chainsetup_looping(void);

  // -------------------------------------------------------------------
  // Chains (if not specified, active chainsetup is used)
  // -------------------------------------------------------------------

  /**
   * Add a new chain (selected chainsetup). Added chain is automatically
   * selected.
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  
   * ensure:
   *   selected_chains().size() > 0
   */
  void add_chain(const string& names);

  /**
   * Add new chains (selected chainsetup).  Added chains are automatically
   * selected.
   *
   * @param names comma separated list of chain names
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  
   * ensure:
   *   selected_chains().size() > 0
   */
  void add_chains(const string& names);

  /**
   * Add new chains (selected chainsetup). Added chains are automatically
   * selected.
   *
   * @param namess vector of chain names
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  
   * ensure:
   *   selected_chains().size() == names.size()
   */
  void add_chains(const vector<string>& names);

  /**
   * Remove currently selected chain (selected chainsetup)
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() > 0 &&
   *
   * ensure:
   *  selected_chains().size() == 0
   */
  void remove_chains(void);

  /**
   * Select chains (currently selected chainsetup)
   *
   * @param chains vector of chain names
   *
   * require:
   *   is_selected() == true
   *
   * ensure:
   *   selected_chains().size() > 0
   */
  void select_chains(const vector<string>& chains);

  /**
   * Select all chains (currently selected chainsetup)
   *
   * require:
   *   is_selected() == true
   */
  void select_all_chains(void);

  /**
   * Returns a list of selected chains (currently selected chainsetup)
   *
   * require:
   *  is_selected() == true
   */
  const vector<string>& selected_chains(void) const;

  /**
   * Clear all selected chains (all chain operators and controllers
   * are removed)
   *
   * @param name chain name 
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() > 0
   */
  void clear_chains(void);

  /**
   * Clear all selected chains (all chain operators and controllers
   * are removed)
   *
   * @param name chain name 
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() == 1
   */
  void rename_chain(const string& name);

  /**
   * Toggle whether chain is muted or not
   *
   * require:
   *  is_selected() == true
   *  selected_chains().size() > 0
   */
  void toggle_chain_muting(void);

  /**
   * Toggle whether chain operators are enabled or disabled
   *
   * require:
   *  is_selected() == true && is_connected() == true
   *  selected_chains().size() > 0
   */
  void toggle_chain_bypass(void);

  /**
   * Rewind selected chains by 'pos_in_seconds' seconds
   *
   * require:
   *  is_selected() == true && is_connected() == true
   *  selected_chains().size() > 0
   */
  void rewind_chains(double pos_in_seconds);

  /**
   * Forward selected chains by 'pos_in_seconds' seconds
   *
   * require:
   *  is_selected() == true && is_connected() == true
   *  selected_chains().size() > 0
   */
  void forward_chains(double pos_in_seconds);

  /**
   * Set position of selected chains to 'pos_in_seconds' seconds
   *
   * require:
   *  is_selected() == true && is_connected() == true
   *  selected_chains().size() > 0
   */
  void set_position_chains(double pos_in_seconds);

  /**
   * Return info about current chain status
   *
   * require:
   *  is_selected() == true
   *  selected_chains().size() > 0
   */
  string chain_status(void) const;

  // -------------------------------------------------------------------
  // Audio-devices  (active chainsetup is edited)
  // -------------------------------------------------------------------

  /** 
   * Add a new audio input (file, soundcard device, etc). Input 
   * is attached to currently selected chains (if any).
   *
   * require:
   *  filename.empty() == false
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *
   * ensure:
   *  get_audio_object(filename) == selected_audio_object_rep
   */
  void add_audio_input(const string& filename);

  /** 
   * Add a new audio output (file, soundcard device, etc). Output 
   * is attached to currently selected chains (if any).
   *
   * require:
   *  filename.empty() == false
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   */
  void add_audio_output(const string& filename);

  /** 
   * Add a default output (as defined in ~/.ecasoundrc) and attach
   * it to currently selected chains.
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   */
  void add_default_output(void);

  /** 
   * Get a pointer to a generic audio object with name 'name'.
   * Returns '0' if audio object doesn't exist.
   *
   * require:
   *  is_selected() == true
   */
  AUDIO_IO* get_audio_object(const string& name);

  /**
   * Remove selected audio input/output
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_audio_object_rep != 0
   *
   * ensure:
   *  selected_audio_object_rep = 0
   */
  void remove_audio_object(void);

  /**
   * Attach selected audio object to selected chains
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() > 0
   *  selected_audio_object_rep != 0
   */
  void attach_audio_object(void);

  /**
   * Rewind selected audio object by 'pos_in_seconds' seconds
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_audio_object_rep != 0
   */
  void rewind_audio_object(double seconds);

  /**
   * Forward selected audio object by 'pos_in_seconds' seconds
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_audio_object_rep != 0
   */
  void forward_audio_object(double seconds);

  /**
   * Set position of selected audio object
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_audio_object_rep != 0
   */
  void set_audio_object_position(double seconds);

  /**
   * Spawns an external wave editor for editing selected audio object.
   *
   * require:
   *  is_selected() 
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_audio_object_rep != 0
   */
  void wave_edit_audio_object(void);

  /**
   * Select an audio object
   *
   * require:
   *  is_selected() == true
   */
  void select_audio_object(const string& name);

  /**
   * Select an audio object by index (see aio_status())
   *
   * require:
   *  is_selected() == true
   *  index.empty() != true
   *  index[0] == 'i' || index[0] == 'o'
   */
  void select_audio_object_by_index(const string& name);

  /**
   * Get audio format of currently selected audio object
   *
   * require:
   *  selected_audio_object_rep != 0
   *  is_selected() == true
   */
  ECA_AUDIO_FORMAT get_audio_format(void) const;

  /**
   * Set default audio format. This format will be used, when
   * adding audio inputs and outputs.
   *
   * require:
   *  is_selected() == true
   */
  void set_default_audio_format(const string& sfrm, int channels, long int srate);

  /**
   * Set default audio format. This format will be used, when
   * adding audio inputs and outputs.
   *
   * require:
   *  is_selected() == true
   */
  void set_default_audio_format(const ECA_AUDIO_FORMAT* format);

  /**
   * Return info about inputs and outputs
   */
  string aio_status(void) const;

  // -------------------------------------------------------------------
  // Chain operators (currently selected chainsetup and chains)
  // -------------------------------------------------------------------

  /**
   * Add a new chain operator
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() == 1
   */
  void add_chain_operator(const string& chainop_params);

  /**
   * Add a new chain operator. Pointer given as argument 
   * will remain to be usable, but notice that it is
   * _NOT_ thread-safe to use assigned/registered objects 
   * from client programs. You must be sure that ecasound 
   * isn't using the same object as you are. The 
   * easiest way to assure this is to disconnect 
   * the chainsetup to which object is attached.
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() == 1
   *  cotmp != 0
   */
  void add_chain_operator(CHAIN_OPERATOR* cotmp);

  /** 
   * Get a const pointer to Nth chain operator. If chain 
   * operator is not valid, 0 is returned.
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() == 1
   *  chainop_id > 0
   */
  const CHAIN_OPERATOR* get_chain_operator(int chainop_id);

  /**
   * Remove Nth chain operator
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() == 1
   *  chainop_id > 0
   */
  void remove_chain_operator(int chainop_id);

  /**
   * Set chain operator parameter value
   *
   * require:
   *  is_selected() == true
   *  selected_chains().size() == 1
   *  chainop_id > 0
   *  param > 0
   */
  void set_chain_operator_parameter(int chainop_id,
				    int param,
				    DYNAMIC_PARAMETERS::parameter_type value);

  /**
   * Add a new controller
   *
   * require:
   *  is_selected() == true
   *  connected_chainsetup() != selected_chainsetup()
   *  selected_chains().size() > 0
   */
  void add_controller(const string& gcontrol_params);

  /**
   * Return info about chain operators (selected chainsetup)
   *
   * require:
   *  is_selected() == true
   */
  string chain_operator_status(void) const;


  ECA_CONTROLLER (ECA_SESSION* psession);
};

void start_normal_thread(ECA_SESSION* param, int retcode, pthread_t* th_cqueue, pthread_attr_t* th_attr);
void* start_normal(void* param);
void start_normal(ECA_SESSION* param);

#endif
