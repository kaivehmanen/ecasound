// ------------------------------------------------------------------------
// audioio-rtnull.cpp: Null audio object with realtime behaviour
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include <string>
#include <time.h>
#include <kvutils.h>

#include "audioio-types.h"
#include "audioio-rtnull.h"

#include "eca-error.h"
#include "eca-debug.h"

REALTIME_NULL::REALTIME_NULL(const string& name) {
  //cerr << "delay " << buffer_delay.tv_sec << "sec.\n";
  //cerr << "delay " << buffer_delay.tv_usec << "usec.\n";
}

void REALTIME_NULL::open(void) throw (AUDIO_IO::SETUP_ERROR &) {
  toggle_open_state(true);
  double t = static_cast<double>(buffersize()) / samples_per_second();
  buffer_delay.tv_sec = static_cast<time_t>(floor(t));
  buffer_delay.tv_usec = static_cast<long>((t - buffer_delay.tv_sec) * 1000000.0);
}

void REALTIME_NULL::close(void) {
  toggle_open_state(false);
}

void REALTIME_NULL::prepare(void) {
  if (io_mode() == io_read) {
    buffer_fill.tv_sec = 0; 
    buffer_fill.tv_usec = 0;
    ::gettimeofday(&access_time, NULL);
    ::gettimeofday(&start_time, NULL);
    toggle_prepared_state(true);
  }
}

void REALTIME_NULL::start(void) {
  if (io_mode() == io_read) {
    toggle_running_state(true);
  }
}

void REALTIME_NULL::stop(void) {
    toggle_running_state(false);
    toggle_prepared_state(false);
}

long int REALTIME_NULL::read_samples(void* target_buffer, 
				     long int samples) {
  assert(is_running() == true);

  for(int n = 0; n < samples * frame_size(); n++) ((char*)target_buffer)[n] = 0;

  struct timeval d,n;
  ::gettimeofday(&d, NULL);
  n.tv_sec = d.tv_sec;
  n.tv_usec = d.tv_usec;

  d.tv_sec -= access_time.tv_sec;
  d.tv_usec -= access_time.tv_usec;
  if (d.tv_usec < 0) { 
    d.tv_sec -= 1;
    d.tv_usec += 1000000;
  }
  buffer_fill.tv_sec += d.tv_sec;
  buffer_fill.tv_usec += d.tv_usec;
  if (buffer_fill.tv_usec > 1000000) { 
    buffer_fill.tv_sec += 1;
    buffer_fill.tv_usec -= 1000000;
  }
 
  if (buffer_fill.tv_sec * 1000000 + buffer_fill.tv_usec > 
      (2 * (buffer_delay.tv_sec * 1000000 + buffer_delay.tv_usec))) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-rtnull) Overrun occured!");
    buffer_fill.tv_sec = 0;
    buffer_fill.tv_usec = 0;
  }
  else if (buffer_fill.tv_sec * 1000000 + buffer_fill.tv_usec <
	   (buffer_delay.tv_sec * 1000000 + buffer_delay.tv_usec)) {
    timespec delay;
    delay.tv_sec = buffer_delay.tv_sec - buffer_fill.tv_sec;
    delay.tv_nsec = buffer_delay.tv_usec - buffer_fill.tv_usec;
    if (delay.tv_nsec < 0) { 
      delay.tv_sec -= 1;
      delay.tv_nsec += 1000000;
    }
    if (delay.tv_sec >= 0) {
      delay.tv_nsec *= 1000;
      //      cerr << "(audioio-rtnull) delay: " << delay.tv_sec << " sec, " << delay.tv_nsec << " nanoseconds.\n";
      nanosleep(&delay, NULL);
    }
  }
  buffer_fill.tv_sec -= buffer_delay.tv_sec; 
  buffer_fill.tv_usec -= buffer_delay.tv_usec;
  if (buffer_fill.tv_usec < 0) { 
    buffer_fill.tv_sec -= 1;
    buffer_fill.tv_usec += 1000000;
  }

  access_time.tv_sec = n.tv_sec;
  access_time.tv_usec = n.tv_usec;

  return(buffersize());
}

void REALTIME_NULL::write_samples(void* target_buffer, long int
				  samples) { 
  if (is_running() != true) {
    ::gettimeofday(&start_time, NULL);
    ::gettimeofday(&access_time, NULL);
    buffer_fill.tv_sec = 0; 
    buffer_fill.tv_usec = 0;
    toggle_running_state(true);
  }
  else {
    struct timeval d,n;
    ::gettimeofday(&d, NULL);
    n.tv_sec = d.tv_sec;
    n.tv_usec = d.tv_usec;
    d.tv_sec -= access_time.tv_sec;
    d.tv_usec -= access_time.tv_usec;
    if (d.tv_usec < 0) { 
      d.tv_sec -= 1;
      d.tv_usec += 1000000;
    }
    assert(d.tv_sec >= 0);
    buffer_fill.tv_sec -= d.tv_sec;
    buffer_fill.tv_usec -= d.tv_usec;
    if (buffer_fill.tv_usec < 0) { 
      buffer_fill.tv_sec -= 1;
      buffer_fill.tv_usec += 1000000;
    }

    if (buffer_fill.tv_sec < 0) {
      ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-rtnull) Underrun occured!");
      buffer_fill.tv_sec = 0; 
      buffer_fill.tv_usec = 0;
    }
    else if (buffer_fill.tv_sec > buffer_delay.tv_sec ||
	     (buffer_fill.tv_sec == buffer_delay.tv_sec &&
	      buffer_fill.tv_usec >= buffer_delay.tv_usec)) {
      timespec delay;
      delay.tv_sec = buffer_fill.tv_sec - buffer_delay.tv_sec;
      delay.tv_nsec = buffer_fill.tv_usec - buffer_delay.tv_usec;
      if (delay.tv_nsec < 0) { 
	delay.tv_sec -= 1;
	delay.tv_nsec += 1000000;
      }
      if (delay.tv_sec >= 0) {
	delay.tv_nsec *= 1000;
	//	cerr << "(audioio-rtnull) delay: " << delay.tv_sec << " sec, " << delay.tv_nsec << " nanoseconds.\n";
	nanosleep(&delay, NULL);
      }
    }
    access_time.tv_sec = n.tv_sec;
    access_time.tv_usec = n.tv_usec;
  }
  buffer_fill.tv_sec += buffer_delay.tv_sec; 
  buffer_fill.tv_usec += buffer_delay.tv_usec;
  if (buffer_fill.tv_usec > 1000000) { 
    buffer_fill.tv_sec += 1;
    buffer_fill.tv_usec -= 1000000;
  }
}

long int REALTIME_NULL::latency(void) const {
  return(buffersize());
}

long REALTIME_NULL::position_in_samples(void) const { 
  if (is_running() != true) return(0);
  struct timeval now;
  ::gettimeofday(&now, NULL);
  double time = now.tv_sec * 1000000.0 + now.tv_usec -
    start_time.tv_sec * 1000000.0 - start_time.tv_usec;
  return(static_cast<long>(time * samples_per_second() / 1000000.0));
}

REALTIME_NULL::~REALTIME_NULL(void) { close(); }
