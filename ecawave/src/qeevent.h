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
   *
   * require:
   *  is_valid() == true
   */
  virtual void start(void) = 0;

  /**
   * Tests whether processing has started
   */
  bool is_triggered(void) const { return(triggered_rep); }

  /**
   * Tests whether event ready for processing
   */
  bool is_valid(void) const { if (ectrl == 0) return(false); else return(ectrl->is_selected() && ectrl->is_valid()); }

  /**
   * Current position in samples
   */
  virtual long int position(void) const { return(ectrl->position_in_samples()); }

  /**
   * Returns processing length in samples
   */
  virtual long int length(void) const { return(ectrl->length_in_samples()); }

  /**
   * Returns name of current input as a formatted string
   */
  const string& input_name(void) const { return(input_object->label()); }

  /**
   * Returns name of current output as a formatted string
   */
  const string& output_name(void) const { return(output_object->label()); }

  bool class_invariant(void) { return(ectrl != 0); }
  QEEvent(ECA_CONTROLLER* ctrl);
  virtual ~QEEvent(void);

 protected:

  /**
   * Starts processing. If processing takes long, a graphical progressbar 
   * is shown.
   *
   * require:
   *  ectrl->is_valid() == true
   *  ectrl->is_selected() == true
   *  is_triggered() == false
   *
   * ensure:
   *  is_triggered() == false
   */
  void blocking_start(void);

  /**
   * Starts processing and returns immediately without blocking.
   * is shown.
   *
   * require:
   *  ectrl->is_valid() == true
   *  ectrl->is_selected() == true
   *  is_triggered() == false
   *
   * ensure:
   *  is_triggered() == true || ectrl->is_running() == false
   */
  void nonblocking_start(void);

  /**
   * Initializes chainsetup for processing
   *
   * require:
   *  ectrl != 0
   * ensure:
   *  ectrl->selected_chainsetup() == chainsetup
   */
  void init(const string& chainsetup, const string& chain = "");

  /**
   * Toggles whether event has been started
   */
  void toggle_triggered_state(bool v) { triggered_rep = v; }

  /**
   * Use audio format of file/device 'name' as the default.
   */
  void set_default_audio_format(const string& name);

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

 public:

  /**
   * Sets a status info string shown to the user while processing
   */
  void status_info(const string& info_string) { info_string_rep = info_string; }

  /**
   * Get the current status string
   */
  const string& status_info(void) const { return(info_string_rep); }

 private:

  string info_string_rep;
  bool triggered_rep;
  ECA_CONTROLLER* ectrl;
  string initialized_cs_rep;

 protected:

  AUDIO_IO* input_object;
  AUDIO_IO* output_object;
};

#endif
