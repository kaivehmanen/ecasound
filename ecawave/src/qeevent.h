#ifndef QEEVENT_H
#define QEEVENT_H

#include <kvutils/definition_by_contract.h>
#include <ecasound/eca-controller.h>

/**
 * Virtual base for processing events
 */
class QEEvent : public DEFINITION_BY_CONTRACT {

 public:

  /**
   * Starts processing
   */
  virtual void start(void) = 0;

  /**
   * Tests whether processing has started
   */
  bool is_triggered(void) const { return(triggered_rep); }

  /**
   * Current position in samples
   */
  long int position(void) const = 0;

  /**
   * Returns processing length in samples
   */
  long int length(void) const = 0;

  QEEvent(void);
  virtual ~QEEvent(void);

 protected:

  const ECA_CONTROLLER* controller(void) const = 0;

  void process(bool blocking);
  void toggle_triggered_state(bool v) { triggered_rep = v; }

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

  /**
   * Returns name of current input as a formatted string
   */
  const string& input_name(void) const { return(input_object->label()); }

  /**
   * Returns name of current output as a formatted string
   */
  const string& output_name(void) const { return(output_object->label()); }

 private:

  bool triggered_rep;
  AUDIO_IO* input_object;
  AUDIO_IO* output_object;
};

#endif
