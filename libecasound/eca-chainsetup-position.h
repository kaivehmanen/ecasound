#ifndef INCLUDED_ECA_CONTROL_POSITION_H
#define INCLUDED_ECA_CONTROL_POSITION_H

/**
 * Global position controller
 */
class ECA_CHAINSETUP_POSITION {

 private:

  bool length_set_rep;
  bool looping_rep;
  long int srate_rep;

  long int length_rep;
  long int curpos_rep;

 public:

  /**
   * Sets length in samples. If 'pos' is 0, length 
   * is unspecified.
   */
  void length_in_samples(long int pos);
  void length_in_seconds(double pos_in_seconds);

  void set_sample_rate(long int srate) { srate_rep = srate; }
  inline void change_position(long int  samples) { curpos_rep += samples; }
  void change_position_exact(double seconds);
  void set_position(long int samples) { curpos_rep = samples; }
  void set_position_exact(double seconds);

  void toggle_looping(bool v) { looping_rep = v; }

  inline bool is_over(void) const { return((curpos_rep > length_rep && length_set_rep == true) ? true : false); }

  long int length_in_samples(void) const { return(length_rep); }
  long int length_in_seconds(void) const;
  double length_in_seconds_exact(void) const;
  long int position_in_samples(void) const { return(curpos_rep); }
  long int position_in_seconds(void) const;
  double position_in_seconds_exact(void) const;
  long int sample_rate(void) const { return(srate_rep); }
  bool length_set(void) const { return(length_set_rep); }
  bool looping_enabled(void) const { return(looping_rep); }

  ECA_CHAINSETUP_POSITION(long int srate);
};

#endif
