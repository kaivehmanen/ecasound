#ifndef QEEVENT_H
#define QEEVENT_H

#include <kvutils/definition_by_contract.h>
#include <ecasound/eca-controller.h>

/**
 * Virtual base for representing libecasound processing events
 */
class QEEvent : public DEFINITION_BY_CONTRACT {

 public:

  /**
   * Start processing
   */
  void start(bool blocking = false);

  /**
   * Stop processing
   */
  void stop(void);

  /**
   * Test whether processing has ended
   */
  bool is_triggered(void) const { return(triggered_rep); }

  /**
   * Test whether event is ready for triggering
   */
  bool is_valid(void) const { return(valid_rep); }

  /**
   * Current position in samples
   */
  long int position(void) const { return(ectrl->position_in_samples()); }

  /**
   * Returns processing length in samples
   */
  long int length(void) const { return(ectrl->length_in_samples()); }

  /**
   * Returns name of current input as a formatted string
   */
  const string& input_name(void) const { return(input_object->label()); }

  /**
   * Returns name of current output as a formatted string
   */
  const string& output_name(void) const { return(output_object->label()); }

  /**
   * Restart event with new position parameters
   */
  virtual void restart(long int start_pos, long int length) = 0;

  /**
   * Initialize event data to its original state
   */
  void init(void);

  QEEvent(ECA_CONTROLLER* ctrl);
  virtual ~QEEvent(void);

 protected:

  void toggle_triggered_state(bool v) { triggered_rep = v; }
  void toggle_valid_state(bool v) { valid_rep = v; }

  /**
   * Use audio format of file/device 'name' as the default.
   */
  void get_default_audio_format(const string& name);

  /**
   * Set input source using a formatted string (refer to ecasound's documentation)
   */
  void set_input(const string& name);
  
  /**
   * Set input start position in samples
   */
  void set_input_position(long int pos);

  /**
   * Set output source using a formatted string (refer to ecasound's documentation)
   */
  void set_output(const string& name);

  /**
   * Set output start position in samples
   */
  void set_output_position(long int pos);

  /**
   * Set processing length in samples
   */
  void set_length(long int pos);

  ECA_CONTROLLER* ectrl;

  bool class_invariant(void) { return(ectrl != 0); }
  
 private:

  bool triggered_rep, valid_rep;
  AUDIO_IO* input_object;
  AUDIO_IO* output_object;
};

#endif
