// -*- mode: C++; -*-
#ifndef INCLUDED_KVU_PROCEDURE_TIMER
#define INCLUDED_KVU_PROCEDURE_TIMER

#include <sys/time.h>
#include <unistd.h>
#include <string>

/**
 * Procedure timer. Meant for timing and gathering statistics of 
 * repeating events.
 * 
 * @author Kai Vehmanen
 */
class PROCEDURE_TIMER {

 public:
  void set_upper_bound(const struct timeval *);
  void set_upper_bound_seconds(double secs);
  void set_lower_bound(const struct timeval *);
  void set_lower_bound_seconds(double secs);
  
  void start(void);
  void start(const struct timespec *t) { start_rep = *t; }
  void stop(void);
  void stop(const struct timespec *t) { now_rep = *t; stop_helper(); }
  void reset(void);

  long int events_over_upper_bound(void) const;
  long int events_under_lower_bound(void) const;
  long int event_count(void) const;
  double max_duration_seconds(void) const;
  double min_duration_seconds(void) const;
  double average_duration_seconds(void) const;
  double last_duration_seconds(void) const;
  const struct timespec* min_duration(void) const;
  const struct timespec* max_duration(void) const;
  std::string to_string(void) const;

  PROCEDURE_TIMER(int id = 0);
  ~PROCEDURE_TIMER(void);

 private:

  PROCEDURE_TIMER(const PROCEDURE_TIMER& x) { }
  PROCEDURE_TIMER& operator=(const PROCEDURE_TIMER& x) { return *this; }

  void stop_helper(void);
  bool less_than(const struct timespec *i, const struct timespec *ii) const;
  void subtract(struct timespec *i, const struct timespec *ii) const;
  double to_seconds(const struct timeval *i) const;
  double to_seconds(const struct timespec *i) const;

  struct timespec start_rep;
  struct timespec now_rep;
  struct timespec min_event_rep;
  struct timespec max_event_rep;
  struct timespec lower_bound_rep;
  struct timespec upper_bound_rep;

  double event_time_total_rep;
  double last_duration_rep;
  long int events_rep;
  long int events_over_bound_rep;
  long int events_under_bound_rep;

  std::string idstr_rep;
};

#endif
