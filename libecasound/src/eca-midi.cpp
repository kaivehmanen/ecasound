// ------------------------------------------------------------------------
// eca-midi.cpp: Routines for accessing raw MIDI -devices (OSS or ALSA).
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

#include <config.h>

#include <cstdio>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <vector>

#ifdef COMPILE_ALSA
#include "eca-alsa-dyn.h"
#endif

#include "eca-resources.h"
#include "eca-midi.h"
#include "eca-debug.h"
#include "eca-error.h"

MIDI_IN_QUEUE midi_in_queue;
pthread_mutex_t midi_in_lock;     // mutex ensuring exclusive access to MIDI-buffer
pthread_cond_t midi_in_cond;
bool midi_in_locked;

MIDI_IN_QUEUE::MIDI_IN_QUEUE(void) {
  right = false;

  current_put = 0;
  current_get = 0;

  bufsize = MIDI_IN_QUEUE_SIZE;

  controller_value = 0.0;

  buffer = vector<char> (bufsize, char(0));
}

bool MIDI_IN_QUEUE::is_status_byte(char byte) const {
  if ((byte & 128) == 128) return(true);
  else return(false);
}

void MIDI_IN_QUEUE::put(char byte) {
  //  cerr << "P:" << current_put << "=" << (int)byte << "\n";
  buffer[current_put] = byte;
  current_put++;
  if (current_put == bufsize) current_put = 0;
}

double MIDI_IN_QUEUE::last_controller_value(void) const { 
 return(controller_value); 
}

bool MIDI_IN_QUEUE::update_controller_value(double controller, double channel) {
  ::pthread_mutex_lock(&::midi_in_lock);
  while (midi_in_locked == true) pthread_cond_wait(&::midi_in_cond,
						   &::midi_in_lock);
  ::midi_in_locked = true;

  bool value_found = false;
  int value_used = 0;
  //  cerr << "ucv:" << current_put << "->";
  for(current_get = current_put;;) {
    if (is_status_byte(buffer[current_get]) == true) {
      // ---
      // check whether control-change status-byte matches
      // ---
      if ((buffer[current_get] & (15 << 4)) == (11 << 4)) {
	if ((buffer[current_get] & 15) == channel) {
	  right = true;
	}
	else {
	  right = false;
	}
      }
      if (!forth_get()) break;
      continue;
    }

    if (right == true) {
      // ---
      // control-change number-byte
      // ---
      if (buffer[current_get] != controller) {
	if (!forth_get()) break;
	if (!forth_get()) break;
	continue;                
      }

      //      cerr << "cc-buffer:" << current_get << "\n";
      // ---
      // control-change data-byte
      // ---
      if (!forth_get()) break;
      if (is_status_byte(buffer[current_get])) continue;

      controller_value = (double)buffer[current_get];
      value_found = true;
      value_used = current_get;
    }

    if (!forth_get()) break;
  }
  //  cerr << current_get << ".\n";
  //  if (value_found) cerr << "vu:" << value_used << "\n";

  ::midi_in_locked = false;
  ::pthread_cond_signal(&midi_in_cond);
  ::pthread_mutex_unlock(&midi_in_lock);
  
  return(value_found);
}

bool MIDI_IN_QUEUE::forth_get(void) {
  current_get++;
  if (current_get == current_put - 1) return(false);
  else if (current_put == 0 && current_get == bufsize) return(false);
  else if (current_put == 1 && current_get == 1) return(false);
  if (current_get == bufsize) current_get = 0;
  return(true);
};

void init_midi_queues(void) throw(ECA_ERROR*) {
  static bool ready = false;

  if (ready == true) return; 
  else ready = true;

  ::pthread_mutex_init(&::midi_in_lock, NULL);
  ::pthread_cond_init(&::midi_in_cond, NULL);
  ::midi_in_locked = false;
  pthread_t th_midi;
  int retcode = ::pthread_create(&th_midi, NULL, update_midi_queues, NULL);
  if (retcode != 0)
    throw(new ECA_ERROR("ECA-MIDI", "unable to create MIDI-thread"));
}

void *update_midi_queues(void *) {
  struct timeval tv;
  int fd;
  char buf[MIDI_IN_QUEUE_SIZE];
  int temp;
  
  ::pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // other threads can cancel this one
    
  ECA_RESOURCES erc;
  string midi_dev = erc.resource("midi-device");

  bool use_alsa = false;

#ifdef COMPILE_ALSA
  snd_rawmidi_t *midihandle;
#endif
  if (midi_dev.find("/dev/snd/") != string::npos) {
    string cardstr,devicestr;
    string::const_iterator p = midi_dev.begin();
    while(p != midi_dev.end() && *p != 'C') ++p;
    ++p;
    while(p != midi_dev.end() && isdigit(*p)) {
      cardstr += " ";
      cardstr[cardstr.size() - 1] = *p;
      ++p;
    }
    while(p != midi_dev.end() && *p != 'D') ++p;
    ++p;
    while(p != midi_dev.end() && isdigit(*p)) {
      devicestr += " ";
      devicestr[devicestr.size() - 1] = *p;
      ++p;
    }
    
    int card = ::atoi(cardstr.c_str());
    int device = ::atoi(devicestr.c_str());
    
    use_alsa = true;
#ifdef COMPILE_ALSA
    ::eca_alsa_load_dynamic_support();

    if (::dl_snd_rawmidi_open(&midihandle, card, device, SND_RAWMIDI_OPEN_INPUT) < 0) {
      throw(new ECA_ERROR("ECA-MIDI", "unable to open ALSA raw-MIDI device " +
			erc.resource("midi-device") + "."));
    }

    fd = ::dl_snd_rawmidi_file_descriptor(midihandle);
#else 
    throw(new ECA_ERROR("ECA-MIDI", "Unable to open ALSA raw-MIDI device, because ALSA was disabled during compilation."));
#endif
  }
  else {
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    fd = ::open("/dev/midi", O_RDONLY);
    if (fd == -1) {
      throw(new ECA_ERROR("ECA-MIDI", "unable to open OSS raw-MIDI device " +
			  erc.resource("midi-device") + "."));
    }
  }
  
  ecadebug->control_flow("MIDI-thread ready " + erc.resource("midi-device"));

  while(true) {
    if (use_alsa) {
#ifdef COMPILE_ALSA
      temp = ::dl_snd_rawmidi_read(midihandle, buf, 1);
#endif
    }
    else {
      temp = ::read(fd, buf, 1);
    }
    ::pthread_mutex_lock(&::midi_in_lock);
    while (::midi_in_locked == true) ::pthread_cond_wait(&::midi_in_cond,
							 &::midi_in_lock);
    ::midi_in_locked = true;
    if (temp < 0) {
      cerr << "ERROR: Can't read from MIDI-device: " << midi_dev << ".\n";
      break;
    }
    for(int n = 0; n < temp; n++) {
      ::midi_in_queue.put(buf[n]);
    }
    ::midi_in_locked = false;
    ::pthread_cond_signal(&::midi_in_cond);
    ::pthread_mutex_unlock(&::midi_in_lock);
  }

  if (use_alsa) {
#ifdef COMPILE_ALSA
    ::dl_snd_rawmidi_close(midihandle);
    ::eca_alsa_unload_dynamic_support();   
#endif
  }
  else {
    ::close(fd);
  }

  return(0);
}
