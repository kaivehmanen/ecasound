#ifndef INCLUDED_ECA_CONTROL_POSITION_H
#define INCLUDED_ECA_CONTROL_POSITION_H

/**
 * Virtual class implementing position and
 * length related chainsetup features.
 */
class ECA_CHAINSETUP_POSITION {

 public:

  // --
  // init and cleanup

  ECA_CHAINSETUP_POSITION(long int srate);

  // --
  // functions for setting and observing position

  inline void change_position(long int  samples) { curpos_rep += samples; }
  void change_position_exact(double seconds);
  void set_position(long int samples) { curpos_rep = samples; }
  void set_position_exact(double seconds);

  inline bool is_over(void) const { return((curpos_rep > length_rep && length_set_rep == true) ? true : false); }
  long int position_in_samples(void) const { return(curpos_rep); }
  long int position_in_seconds(void) const;
  double position_in_seconds_exact(void) const;

  // --
  // functions for seeking

  /**
   * Seek to current position.
   */
  virtual void seek_position(void) = 0;

  // --
  // functions for setting and observing length

  /**
   * Sets length in samples. If 'pos' is 0, length 
   * is unspecified.
   */
  void length_in_samples(long int pos);
  void length_in_seconds(double pos_in_seconds);

  bool length_set(void) const { return(length_set_rep); }
  long int length_in_samples(void) const { return(length_rep); }
  long int length_in_seconds(void) const;
  double length_in_seconds_exact(void) const;

  // --
  // functions for setting and observing sample rate and looping

  void set_sample_rate(long int srate) { srate_rep = srate; }
  long int sample_rate(void) const { return(srate_rep); }

  void toggle_looping(bool v) { looping_rep = v; }
  bool looping_enabled(void) const { return(looping_rep); }

 private:

  bool length_set_rep;
  bool looping_rep;
  long int srate_rep;

  long int length_rep;
  long int curpos_rep;

};

#endif
