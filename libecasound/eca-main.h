#ifndef INCLUDED_ECA_MAIN_H
#define INCLUDED_ECA_MAIN_H

#include <vector>
#include <string>
#include <ctime>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "samplebuffer.h"
#include "eca-chainsetup.h"
#include "audioio-buffered-proxy.h"

class AUDIO_IO;
class AUDIO_IO_DEVICE;
class ECA_SESSION;
class CHAIN;
class CHAIN_OPERATOR;

/**
 * Main processing engine
 */
class ECA_PROCESSOR {

 public:

  enum COMMANDS {
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

private:

  ECA_SESSION* eparams_repp;

  bool was_running_rep;
  bool rt_running_rep;
  bool end_request_rep;
  bool continue_request_rep;
  bool trigger_outputs_request_rep;
  bool input_not_finished_rep;
  bool processing_range_set_rep;
  bool use_double_buffering_rep;
  
  int trigger_counter_rep;
  struct timeval multitrack_input_stamp_rep;

  // ---
  // Pointers to connected chainsetup
  // ---
  ECA_CHAINSETUP* csetup_repp;
  vector<CHAIN*>* chains_repp;

  // -> pointers to input objects
  //    (when proxies are used, inputsr_inputs != inputs)
  vector<AUDIO_IO*>* inputs_repp;
  vector<AUDIO_IO*>* csetup_inputs_repp;
  
  // -> pointers to input objects in csetup
  // -> r_outputs used for runtime-i/o calls
  //    (only when proxies are used, r_outputs != outputs)
  vector<AUDIO_IO*>* outputs_repp;
  vector<AUDIO_IO*>* csetup_outputs_repp;

  mutable map<AUDIO_IO*,AUDIO_IO*> csetup_orig_ptr_map_rep;

  // ---
  // Various audio objects groupings
  // ---
  // - pointers to all realtime inputs
  vector<AUDIO_IO_DEVICE*> realtime_inputs_rep;
  // - pointers to all realtime outputs
  vector<AUDIO_IO_DEVICE*> realtime_outputs_rep;
  // - pointers to all realtime inputs and outputs
  vector<AUDIO_IO_DEVICE*> realtime_objects_rep;
  // - pointers to all non_realtime inputs
  vector<AUDIO_IO*> non_realtime_inputs_rep;
  // - pointers to all non_realtime outputs
  vector<AUDIO_IO*> non_realtime_outputs_rep;
  // - pointers to all non_realtime inputs and outputs
  vector<AUDIO_IO*> non_realtime_objects_rep;
  // - pointers to proxy input objects (if used, assigned to r_inputs)
  vector<AUDIO_IO*> proxy_inputs_rep;
  // - pointers to proxy output objects (if used, assigned to r_inputs)
  vector<AUDIO_IO*> proxy_outputs_rep;
  vector<AUDIO_IO_BUFFERED_PROXY*> proxies_rep;

  // ---
  // Data objects
  // ---
  vector<int> input_start_pos_rep;
  vector<int> output_start_pos_rep;
  vector<int> input_chain_count_rep;
  vector<int> output_chain_count_rep;

  AUDIO_IO_PROXY_SERVER pserver_rep;
  SAMPLE_BUFFER mixslot_rep;
  vector<SAMPLE_BUFFER> cslots_rep;

  long int buffersize_rep;
  ECA_CHAINSETUP::Mix_mode mixmode_rep;

  int input_count_rep, output_count_rep, chain_count_rep, max_channels_rep;

  /**
   * Start processing if it was conditionally stopped
   */
  void conditional_start(void);

  /**
   * Stop processing (see conditional_start())
   */
  void conditional_stop(void);

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

  /**
   * Start processing. If in multitrack-mode, performs the initial 
   * multitrack-sync phase.
   */
  void start(void);

  /**
   * Stop processing and notifies all devices.
   */
  void stop(void);
    
  /**
   * Interprets the command queue for interactive commands and
   * acts accordingly.
   */
  void interpret_queue(void);

  void interactive_loop(void);

  /**
   * Performs one processing loop skipping all realtime inputs
   * and outputs connected to them. The idea is to fill all 
   * the output buffers before starting to record from realtime 
   * inputs.
   */
  void multitrack_sync(void);

  /**
   * Trigger all output devices if requested by start()
   */
  void trigger_outputs(void);

  void init_variables(void);
  void init_connection_to_chainsetup(void);
  void init_multitrack_mode(void);
  void init_mix_method(void);
  void init_pserver(void);
  void init_inputs(void);
  void init_outputs(void);
  void init_chains(void);
  void create_sorted_input_map(void);
  void create_sorted_output_map(void);

  bool is_slave_output(AUDIO_IO* aiod) const;
  bool has_realtime_objects(void) const;

  void inputs_to_chains(void);
  void mix_to_chains(void);
  void mix_to_outputs(void);

  void chain_processing(void);
  void chain_muting(void);
  
  void exec_normal_iactive(void);
  void exec_simple_iactive(void);
  bool finished(void);

  typedef vector<AUDIO_IO*>::const_iterator audio_ci;
  typedef vector<SAMPLE_BUFFER>::const_iterator audioslot_ci;
  typedef vector<CHAIN*>::const_iterator chain_ci;
  typedef vector<CHAIN*>::iterator chain_i;
  typedef vector<CHAIN_OPERATOR*>::const_iterator chainop_ci;

 public:

  void exec(void);
  void init(void);
  void init(ECA_SESSION* eparam);

  ECA_PROCESSOR(void);
  ECA_PROCESSOR(ECA_SESSION* eparam);
  ~ECA_PROCESSOR(void);

 private:

  ECA_PROCESSOR& operator=(const ECA_PROCESSOR& x) { return *this; }
};

#endif
