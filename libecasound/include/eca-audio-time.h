#ifndef _ECA_AUDIO_TIME_H
#define _ECA_AUDIO_TIME_H

#include <string>

/**
 * Generic class for representing time in audio environment
 */
class ECA_AUDIO_TIME {

 private:

  long int samples_rep;
  long int sample_rate_rep;

 public:

  enum format_type { format_hour_min_sec, format_min_sec, format_seconds, format_samples };

  ECA_AUDIO_TIME(long int samples, long int sample_rate);
  ECA_AUDIO_TIME(double time_in_seconds);
  ECA_AUDIO_TIME(format_type type, const string& time);
  ECA_AUDIO_TIME(void);

  void set(format_type type, const string& time);
  void set_seconds(double seconds);
  void set_samples(long int samples);
  void set_samples_per_second(long int srate);
    
  string to_string(format_type type) const;
  double seconds(void) const;
  long int samples_per_second(void) const;
  long int samples(void) const;
};

#endif
