#ifndef INCLUDED_ECA_ENGINE_H
#define INCLUDED_ECA_ENGINE_H

#include <vector>
#include <string>

#include "samplebuffer.h"

class AUDIO_IO;
class AUDIO_IO_DEVICE;
class ECA_SESSION;
class ECA_CHAINSETUP;
class CHAIN;
class CHAIN_OPERATOR;
class AUDIO_IO_BUFFERED_PROXY;
class ECA_ENGINE_impl;

/**
 * Main processing engine
 *
 * Notes: This class is closely tied to 
 *        the ECA_SESSION and ECA_CHAINSETUP, 
 *        which all allow friend-access to all 
 *        their private data and members to 
 *        ECA_ENGINE.
 */
class ECA_ENGINE {

 public:

  /** @name Public type definitions and constants */
  /*@{*/

  /** 
   * Engine operation states
   */
  enum Engine_status { engine_status_running,
		       engine_status_stopped, 
		       engine_status_finished,
		       engine_status_error,
		       engine_status_notready };
  typedef enum Engine_status Engine_status_t;
  
  /**
   * Commands used in ECA_ENGINE<->ECA_CONTROL communication.
   */
  enum Engine_command {
    ep_start,
    ep_stop,
    ep_debug,
    ep_exit,
    // --
    //      ep_aio_forward,
    //      ep_aio_rewind,
    //      ep_aio_setpos,
    //      ep_ao_select,
    //      ep_ai_select,
    // --
    ep_c_mute,
    ep_c_bypass,
    ep_c_forward,
    ep_c_rewind,
    ep_c_setpos,
    ep_c_select,
    // --
    ep_cop_select,
    ep_copp_select,
    ep_copp_value,
    // --
    ep_sfx,
    ep_rewind,
    ep_forward,
    ep_setpos
  };
  typedef enum Engine_command Engine_command_t;

  /*@}*/

  /** @name Public functions */
  /*@{*/

  ECA_ENGINE(ECA_SESSION* eparam);
  ~ECA_ENGINE(void);
  void exec(void);
  void command(Engine_command_t cmd, double arg);
  void wait_for_stop(int timeout);

  /** @name Public functions for observing engine status information */
  /*@{*/

  Engine_status_t status(void) const;
  const ECA_SESSION* session(void) const { return(session_repp); }

  /*@}*/

private:

  /** @name Private data and functions */
  /*@{*/

  /**
   * Number of sample-frames of data is prefilled to 
   * rt-outputs before starting processing.
   */
  static const long int prefill_threshold_constant = 4096;
  static const int prefill_blocks_constant = 3;

  ECA_ENGINE_impl* impl_repp;

  long int prefill_threshold_rep;

  bool was_running_rep;
  bool rt_running_rep;
  bool end_request_rep;
  bool continue_request_rep;
  bool trigger_outputs_request_rep;
  bool processing_range_set_rep;
  bool use_double_buffering_rep;
  bool use_midi_rep;
  int outputs_finished_rep;
  int inputs_not_finished_rep;

  int trigger_counter_rep;

  /*@}*/

  /** @name Pointers to connected chainsetup  */
  /*@{*/

  ECA_SESSION* session_repp;
  ECA_CHAINSETUP* csetup_repp;
  std::vector<CHAIN*>* chains_repp;
  std::vector<AUDIO_IO*>* inputs_repp;
  std::vector<AUDIO_IO*>* outputs_repp;

  /*@}*/

  /** @name Audio data buffers */
  /*@{*/

  SAMPLE_BUFFER mixslot_rep;
  std::vector<SAMPLE_BUFFER> cslots_rep;

  /*@}*/

  /** 
   * @name Various audio object maps.
   * 
   * The main purpose of these maps is to make 
   * it easier to iterate audio objects with 
   * certain attributes.
   */
  /*@{*/

  std::vector<AUDIO_IO_DEVICE*> realtime_inputs_rep;
  std::vector<AUDIO_IO_DEVICE*> realtime_outputs_rep;
  std::vector<AUDIO_IO_DEVICE*> realtime_objects_rep;
  std::vector<AUDIO_IO*> non_realtime_inputs_rep;
  std::vector<AUDIO_IO*> non_realtime_outputs_rep;
  std::vector<AUDIO_IO*> non_realtime_objects_rep;

  /*@}*/

  /** @name Cache objects for chainsetup and audio 
   * object information  */
  /*@{*/

  std::vector<int> input_start_pos_rep;
  std::vector<int> output_start_pos_rep;
  std::vector<int> input_chain_count_rep;
  std::vector<int> output_chain_count_rep;
  int input_count_rep;
  int output_count_rep;
  int chain_count_rep;
  int max_channels_rep;
  long int buffersize_rep;
  Engine_status_t engine_status_rep;

  /*@}*/

  /** @name Private functions for transport control  */
  /*@{*/

  /**
   * Start processing. If in multitrack-mode, performs the initial 
   * multitrack-sync phase.
   */
  void start(void);

  /**
   * Stop processing and notifies all devices.
   */
  void stop(void);

  void signal_stop(void);

  /**
   * Start processing if it was conditionally stopped
   */
  void conditional_start(void);

  /**
   * Stop processing (see conditional_start())
   */
  void conditional_stop(void);

  void start_servers(void);
  void stop_servers(void);

  void multitrack_sync(void);
  void multitrack_start(void);
  void reset_realtime_devices(void);

  /**
   * Trigger all output devices if requested by start()
   */
  void trigger_outputs(void);

  /*@}*/

  /** @name Private functions for observing and modifying position  */
  /*@{*/

  double current_position(void) const; // seconds, uses the master_input
  double current_position_chain(void) const; // seconds
  void set_position(double seconds);
  void set_position_chain(double seconds);
  void set_position(int seconds) { set_position((double)seconds); }
  void change_position(double seconds);
  void change_position_chain(double seconds);
  void rewind_to_start_position(void);

  /**
   * Calculates how much data we need to process and sets buffersize 
   * accordingly.
   */
  void prehandle_control_position(void);

  /**
   * If we've processed all the data that was request, stop or rewind. 
   * Also resets buffersize to its default value.
   */
  void posthandle_control_position(void);

  /*@}*/

  /** @name Private functions for command queue handling  */
  /*@{*/

  /**
   * Interprets the command queue for interactive commands and
   * acts accordingly.
   */
  void interpret_queue(void);

  void update_engine_state(void);

  /*@}*/

  /** @name Private functions for initial setup  */
  /*@{*/

  void init_variables(void);
  void init_connection_to_chainsetup(void);
  void init_multitrack_mode(void);
  void init_mix_method(void);
  void init_servers(void);
  void init_inputs(void);
  void init_outputs(void);
  void init_chains(void);
  void init_sorted_input_map(void);
  void init_sorted_output_map(void);

  /*@}*/

  /** @name Private functions for signal routing  */
  /*@{*/

  void inputs_to_chains(bool skip_realtime_inputs);
  void mix_to_chains(void);
  void process_chains(void);
  void mix_to_outputs(bool skip_realtime_target_outputs);

  /*@}*/

  /** @name Private functions for processing  */
  /*@{*/

  void exec_normal_iactive(void);
  void exec_simple_iactive(void);

  /*@}*/

  /** @name Private functions for toggling engine features */
  /*@{*/

  void chain_processing(void);
  void chain_muting(void);

  /*@}*/

  /** @name Private functions for modifying engine status information */
  /*@{*/

  /**
   * Set engine status to 'state'.
   *
   * @see status()
   */
  void set_status(Engine_status_t state);

  /*@}*/

  /** @name Private type definitions  */
  /*@{*/

  typedef std::vector<AUDIO_IO*>::const_iterator audio_ci;
  typedef std::vector<SAMPLE_BUFFER>::const_iterator audioslot_ci;
  typedef std::vector<CHAIN*>::const_iterator chain_ci;
  typedef std::vector<CHAIN*>::iterator chain_i;
  typedef std::vector<CHAIN_OPERATOR*>::const_iterator chainop_ci;

  /*@}*/

  /** @name Hidden/unimplemented functions */
  /*@{*/

  ECA_ENGINE& operator=(const ECA_ENGINE& x) { return *this; }

  /*@}*/
};

#endif
