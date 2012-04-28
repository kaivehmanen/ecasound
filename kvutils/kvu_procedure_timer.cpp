// ------------------------------------------------------------------------
// kvu_procedure_timer.cpp: Procedure timer. Meant for timing and gathering 
//                          statistics of repeating events.
// Copyright (C) 2000,2004,2012 Kai Vehmanen
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <cmath>
#include <math.h> /* floor() */
#include "kvu_numtostr.h"
#include "kvu_procedure_timer.h"

#include <cstring>

PROCEDURE_TIMER::PROCEDURE_TIMER(int id) {
  reset();
  idstr_rep = "Timer-" + kvu_numtostr(id);
}

PROCEDURE_TIMER::~PROCEDURE_TIMER(void) {

}

bool PROCEDURE_TIMER::less_than(const struct timespec *i, 
                                const struct timespec *ii) const
{
  bool result = false;
  if (i->tv_sec < ii->tv_sec)
    result = true;

  if (i->tv_sec == ii->tv_sec) {
    result = false;
    if (i->tv_nsec < ii->tv_nsec)
      result = true;
  }

  return result;
}

void PROCEDURE_TIMER::subtract(struct timespec *i, 
                               const struct timespec *ii) const
{
  i->tv_nsec -= ii->tv_nsec;
  if (i->tv_nsec < 0) {
    i->tv_nsec += 1000000000;
    i->tv_sec -= 1;
  }
  i->tv_sec -= ii->tv_sec;
}

double PROCEDURE_TIMER::to_seconds(const struct timeval *i) const {
  return(i->tv_sec + static_cast<double>(i->tv_usec) / 1000000.0);
}

double PROCEDURE_TIMER::to_seconds(const struct timespec *i) const {
  return(i->tv_sec + static_cast<double>(i->tv_nsec) / 1000000000.0);
}

void PROCEDURE_TIMER::set_upper_bound(const struct timeval *p) {
  upper_bound_rep.tv_sec = p->tv_sec;
  upper_bound_rep.tv_nsec = p->tv_usec * 1000;
}

void PROCEDURE_TIMER::set_upper_bound_seconds(double secs) {
  struct timeval buf;
  buf.tv_sec = static_cast<time_t>(floor(secs));
  buf.tv_usec = static_cast<time_t>(1000000.0f * (secs - static_cast<double>(buf.tv_sec)));
  set_upper_bound(&buf);
}

void PROCEDURE_TIMER::set_lower_bound(const struct timeval *p) {
  lower_bound_rep.tv_sec = p->tv_sec;
  lower_bound_rep.tv_nsec = p->tv_usec * 1000;
}

void PROCEDURE_TIMER::set_lower_bound_seconds(double secs) {
  struct timeval buf;
  buf.tv_sec = static_cast<time_t>(floor(secs));
  buf.tv_usec = static_cast<time_t>(1000000.0f * (secs - static_cast<double>(buf.tv_sec)));
  set_lower_bound(&buf);
}

static void priv_gettime(struct timespec *dst) 
{
#if HAVE_CLOCK_GETTIME
#if defined CLOCK_MONOTONIC
  clock_gettime(CLOCK_MONOTONIC, dst);
#else
  clock_gettime(CLOCK_REALTIME, dst);
#endif
#else
  struct timeval tmp;
  gettimeofday(&tmp, 0);
  dst->tv_sec = tmp.tv_sec;
  dst->tv_nsec = tmp.tv_usec * 1000;
#endif
}

void PROCEDURE_TIMER::start(void)
{
  priv_gettime(&start_rep);
}

void PROCEDURE_TIMER::stop(void)
{
  priv_gettime(&now_rep);
  stop_helper();
}

/**
 * Do heavy operations in a non-inlined helper
 * functions for stop().
 *
 * This is especially useful for the inlined stop()
 * variant.
 */
void PROCEDURE_TIMER::stop_helper(void)
{
  subtract(&now_rep, &start_rep);
  last_duration_rep = to_seconds(&now_rep);
  //  cerr << idstr_rep << ": " << kvu_numtostr(length, 16) << " secs." << endl;
  events_rep++;
  event_time_total_rep += last_duration_rep;
  if (events_rep == 1) 
    memcpy(&min_event_rep, &now_rep, sizeof(struct timeval));
  if (less_than(&now_rep, &min_event_rep))
    memcpy(&min_event_rep, &now_rep, sizeof(struct timeval));
  if (less_than(&max_event_rep, &now_rep))
    memcpy(&max_event_rep, &now_rep, sizeof(struct timeval));
  if (less_than(&now_rep, &lower_bound_rep))
    events_under_bound_rep++;
  if (less_than(&upper_bound_rep, &now_rep))
    events_over_bound_rep++;
}

void PROCEDURE_TIMER::reset(void) {
  events_over_bound_rep = 0;
  events_under_bound_rep = 0;
  events_rep = 0;
  event_time_total_rep = 0.0f;
  last_duration_rep = 0.0f;
  memset(&now_rep, 0, sizeof(struct timeval));
  memset(&start_rep, 0, sizeof(struct timeval));
  memset(&min_event_rep, 0, sizeof(struct timeval));
  memset(&max_event_rep, 0, sizeof(struct timeval));
  memset(&lower_bound_rep, 0, sizeof(struct timeval));
  memset(&upper_bound_rep, 0, sizeof(struct timeval));
}

long int PROCEDURE_TIMER::events_over_upper_bound(void) const { return(events_over_bound_rep); }
long int PROCEDURE_TIMER::events_under_lower_bound(void) const { return(events_under_bound_rep); }
long int PROCEDURE_TIMER::event_count(void) const { return(events_rep); }
double PROCEDURE_TIMER::max_duration_seconds(void) const { return(to_seconds(&max_event_rep)); }
double PROCEDURE_TIMER::min_duration_seconds(void) const { return(to_seconds(&min_event_rep)); }
double PROCEDURE_TIMER::average_duration_seconds(void) const { return(event_time_total_rep / event_count()); }
double PROCEDURE_TIMER::last_duration_seconds(void) const { return(last_duration_rep); }
const struct timespec* PROCEDURE_TIMER::min_duration(void) const { return(&max_event_rep); }
const struct timespec* PROCEDURE_TIMER::max_duration(void) const { return(&min_event_rep); }

static std::string priv_s2us(double sec)
{
  return kvu_numtostr(sec * 1000000.0, 6);
}

std::string PROCEDURE_TIMER::to_string(void) const
{ 
  std::string res;

  res = idstr_rep + ":\n";
  res += "Number of events: " + kvu_numtostr(event_count()) + "\n";
  res += "Events over bound: "  + kvu_numtostr(events_over_upper_bound());
  res += " (" + kvu_numtostr(to_seconds(&upper_bound_rep), 8) + "sec)\n";
  res += "Events under bound: "  + kvu_numtostr(events_under_lower_bound());
  res += " (" + kvu_numtostr(to_seconds(&lower_bound_rep), 8) + "sec)\n";
  res += "Min duration, us: " + priv_s2us(min_duration_seconds()) + "\n";
  res += "Max duration, us: " + priv_s2us(max_duration_seconds()) + "\n";
  res += "Average duration, us: " + priv_s2us(average_duration_seconds()) + "\n";
  res += "Duration of last event, us: " + priv_s2us(last_duration_rep) + "\n";

  return(res); 
}
