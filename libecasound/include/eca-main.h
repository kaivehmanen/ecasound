#ifndef _ECA_MAIN_H
#define _ECA_MAIN_H

#include <vector>
#include <string>
#include <ctime>
#include <pthread.h>

#include <kvutils/kvutils.h>

#include "samplebuffer.h"

class AUDIO_IO;
class ECA_SESSION;
class CHAIN;
class CHAIN_OPERATOR;
class ECA_CHAINSETUP;

extern VALUE_QUEUE ecasound_queue;

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

  ECA_SESSION* eparams;
  pthread_t chain_thread;

  bool finished_result;
  bool rt_infiles, rt_outfiles;
  bool nonrt_infiles;
  bool nonrt_outfiles;

  bool was_running;
  bool end_request;
  bool trigger_outputs_request;

  size_t active_chain_index;
  size_t active_chainop_index;
  size_t active_chainop_param_index;

  // ---
  // Pointers to connected chainsetup
  // ---
  ECA_CHAINSETUP* csetup;
  vector<AUDIO_IO*>* inputs;
  vector<AUDIO_IO*>* outputs;
  vector<CHAIN*>* chains;
  vector<long int> input_start_pos;
  vector<long int> output_start_pos;

  SAMPLE_BUFFER mixslot;
  long int buffersize_rep;

  vector<int> input_chain_count;
  vector<int> output_chain_count;

  int input_count, output_count, chain_count;

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

  void init_connection_to_chainsetup(void) throw(ECA_ERROR*);
  void init_status_variables(void);
  void init_mix_method(void);

  void inputs_to_chains(void);
  void mix_to_chains(void);
  void mix_to_outputs(void);

  void chain_processing(void);
  void chain_muting(void);
  
  void exec_normal_iactive(void);
  void exec_normal_passive(void);
  void exec_simple_iactive(void);
  void exec_simple_passive(void);
  void exec_mthreaded_iactive(void) throw(ECA_ERROR*);
  void exec_mthreaded_passive(void) throw(ECA_ERROR*);
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

void *mthread_process_chains(void* ecaparams);

#endif






