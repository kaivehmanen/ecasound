#ifndef _AUDIO_GATE_H
#define _AUDIO_GATE_H

#include "eca-chainop.h"

class SAMPLE_BUFFER;

/**
 * Interface to gate effects. Gate processes sample data, but
 * doesn't modify it. Gate is either open or closed.
 */
class GATE_BASE : public CHAIN_OPERATOR {

private:

  bool gate_open;
  SAMPLE_BUFFER* target;
  
protected:

  void close_gate(void) { gate_open = false; }
  void open_gate(void) { gate_open = true; }
 
public:

  inline bool is_open(void) const { return(gate_open); }
  void init(SAMPLE_BUFFER* sbuf);
  void process(void);
  virtual void analyze(SAMPLE_BUFFER* sbuf) = 0;

  virtual GATE_BASE* clone(void) = 0;   
  virtual ~GATE_BASE(void) { }

  GATE_BASE(void) { close_gate(); }
};

/**
 *
 * A time crop gate. Initially the gate is closed, but is opened after 
 * 'open_at' seconds has elapsed. Gate remains open for 
 * 'duration' seconds. If 'duration' is 0, gate will stay open
 * forever.
 */
class TIME_CROP_GATE : public GATE_BASE {

public:

  // Functions returning info about effect and its parameters.
  // ---
  parameter_type get_parameter(int param) const;
  void set_parameter(int param, parameter_type value);

  string name(void) const { return("Time crop gate"); }

  string parameter_names(void) const { return("open-at-sec,duration-sec"); }

  void analyze(SAMPLE_BUFFER* insample);

  TIME_CROP_GATE* clone(void)  { return new TIME_CROP_GATE(*this); }
  TIME_CROP_GATE* new_expr(void)  { return new TIME_CROP_GATE(); }
  TIME_CROP_GATE (parameter_type open_at, parameter_type duration);
  TIME_CROP_GATE (void) : curtime(0.0) {
    close_gate();
  }

private:

  parameter_type curtime, btime, etime; 
};

/**
 * A threshold open gate. When the average volume goes 
 * over 'threshold_openlevel', gate is opened. In the 
 * same way, when the average volume drops below 
 * 'threshold_closelevel', gate is closed. Either 
 * peak or RMS level is used for calculating average 
 * volume. The thresholds are given in percents. Unlike
 * noise gates, threshold gate is opened and closed 
 * only once. 
 */
class THRESHOLD_GATE : public GATE_BASE {

public:

  // Functions returning info about effect and its parameters.
  // ---
  parameter_type get_parameter(int param) const;
  void set_parameter(int param, parameter_type value);

  string name(void) const { return("Threshold gate"); }

  string parameter_names(void) const { return("threshold-openlevel-%,threshold-closelevel-%,rms-enabled"); }

  void analyze(SAMPLE_BUFFER* insample);

  THRESHOLD_GATE* clone(void)  { return new THRESHOLD_GATE(*this); }
  THRESHOLD_GATE* new_expr(void)  { return new THRESHOLD_GATE(); }
  THRESHOLD_GATE (parameter_type threshold_openlevel, parameter_type
		  threshold_closelevel,  bool use_rms = false);
  THRESHOLD_GATE (void) 
    : rms(false), is_opened(false), is_closed(false) { }
  
private:
  
  parameter_type openlevel, closelevel, avolume;
  bool rms;
  bool is_opened, is_closed;
};

#endif
