// ------------------------------------------------------------------------
// eca-midi.cpp: Routines for accessing raw MIDI -devices (OSS or ALSA).
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

// ------
// FIXME: Note! This is code is old, ugly and soon obsolete. :)
//              Ecasound 1.9.x will offer much better MIDI-subsystem.
// ------

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdio>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <vector>

#ifdef COMPILE_ALSA_RAWMIDI
#include <sys/asoundlib.h>
#endif

#include "eca-resources.h"
#include "eca-midi.h"
#include "eca-debug.h"
#include "eca-error.h"

MIDI_IN_QUEUE midi_in_queue;

const unsigned int MIDI_IN_QUEUE::max_queue_size_rep = 32768;

void *start_midi_queues(void *ptr);
string get_midi_device(void);

void init_midi_queues(void) throw(ECA_ERROR&) {
  static bool ready = false;

  if (ready == true) return; 
  else ready = true;

  pthread_t th_midi;
  int retcode = ::pthread_create(&th_midi,
				 0,
				 start_midi_queues,
				 static_cast<void *>(&midi_in_queue));
  if (retcode != 0)
    throw(ECA_ERROR("ECA-MIDI", "unable to create MIDI-thread"));
}

/**
 * Helper function for starting the slave thread.
 */
void* start_midi_queues(void *ptr) {
  ::pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // other threads can cancel this one
  MIDI_IN_QUEUE* mqueue = static_cast<MIDI_IN_QUEUE*>(ptr);
  mqueue->update_midi_queues();
}

string get_midi_device(void) {
  ECA_RESOURCES erc;
  return(erc.resource("midi-device"));
}

MIDI_IN_QUEUE::MIDI_IN_QUEUE(void) {
  running_status_rep = 0;
  current_ctrl_channel_rep = -1;
  ::pthread_mutex_init(&midi_in_lock_rep, 0);
  ::pthread_cond_init(&midi_in_cond_rep, 0);
  midi_in_locked_rep = false;
}

/**
 * Whether 'byte' belong to Voice Category status messages
 * (ie. 0x80 to 0xef)?
 */
bool MIDI_IN_QUEUE::is_voice_category_status_byte(unsigned char byte) const {
  if (byte >= 0x80 && byte < 0xf0) return(true);
  return(false);
}

/**
 * Whether 'byte' belong to System Common Category status messages
 * (ie. 0xf0 to 0xf7)?
 */
bool MIDI_IN_QUEUE::is_system_common_category_status_byte(unsigned char byte) const {
  if (byte >= 0xf0 && byte < 0xf8) return(true);
  return(false);
}

/**
 * Whether 'byte' belong to Realtime Category status messages
 * (ie. 0xf8 to 0xff)?
 */
bool MIDI_IN_QUEUE::is_realtime_category_status_byte(unsigned char byte) const {
  if (byte > 0xf7) return(true);
  return(false);
}

/**
 * Whether 'byte' is a statuus byte (0x80 to 0xff)?
 */
bool MIDI_IN_QUEUE::is_status_byte(unsigned char byte) const {
  if (byte & 0x80) return(true);
  return(false);
}


/**
 * Puts a single byte into the MIDI input queue
 */
void MIDI_IN_QUEUE::put(unsigned char byte) {
  buffer_rep.push_back(byte);
  while(buffer_rep.size() > max_queue_size_rep) {
//      cerr << "(eca-midi) dropping midi bytes" << endl;
    buffer_rep.pop_front();
  }
}

int MIDI_IN_QUEUE::last_controller_value(int channel, int controller) const {  
  // FIXME: what if not found?
  return(controller_values_rep[pair<int,int>(channel,controller)]); 
}

bool MIDI_IN_QUEUE::update_controller_value(void) {
  ::pthread_mutex_lock(&midi_in_lock_rep);
  while (midi_in_locked_rep == true) pthread_cond_wait(&midi_in_cond_rep,
						       &midi_in_lock_rep);
  midi_in_locked_rep = true;

  bool value_found = false;
  int current_ctrl_number = -1;

  while(buffer_rep.size() > 0) {
    unsigned char byte = buffer_rep.front();
    buffer_rep.pop_front();

    if (is_status_byte(byte) == true) {
      if (is_voice_category_status_byte(byte) == true) {
	running_status_rep = byte;
	if ((running_status_rep & 0xb0) == 0xb0)
	  current_ctrl_channel_rep = static_cast<int>((byte & 15));
      }
      else if (is_system_common_category_status_byte(byte) == true) {
	current_ctrl_channel_rep = -1;
	running_status_rep = 0;

      }
      
    }
    else { /* non-status bytes */
      /** 
       * Any data bytes are ignored if no running status
       */
      if (running_status_rep != 0) {

	/**
	 * Check for 'controller messages' (status 0xb0 to 0xbf and
	 * two data bytes)
	 */
	if (current_ctrl_channel_rep != -1) {
	  if (current_ctrl_number == -1) {
	    current_ctrl_number = static_cast<int>(byte);
//    	    cerr << endl << "C:" << current_ctrl_number << ".";
	  }
	  else {
	    controller_values_rep[pair<int,int>(current_ctrl_channel_rep,current_ctrl_number)] = static_cast<int>(byte);
	    value_found = true;
//    	    cerr << endl << "D:" 
//  		 << controller_values_rep[pair<int,int>(current_ctrl_channel_rep,current_ctrl_number)] 
//  		 << ", ch:" << current_ctrl_channel_rep 
//  		 << ", ctrl:" << current_ctrl_number << ".";
	    current_ctrl_number = -1;
	  }
	}
      }
    }
  }

  midi_in_locked_rep = false;
  ::pthread_cond_signal(&midi_in_cond_rep);
  ::pthread_mutex_unlock(&midi_in_lock_rep);
  
  return(value_found);
}

void MIDI_IN_QUEUE::update_midi_queues(void) {
  int fd;
  fd_set fds;
  unsigned char buf[16];
  int temp;
  string rc_midi_device = get_midi_device();
  bool use_alsa = false;
#ifdef COMPILE_ALSA_RAWMIDI
  snd_rawmidi_t *midihandle;
#endif

  string midi_dev = rc_midi_device;
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
    
    use_alsa = true;
#ifdef COMPILE_ALSA_RAWMIDI
    int card = ::atoi(cardstr.c_str());
    int device = ::atoi(devicestr.c_str());

    if (::snd_rawmidi_open(&midihandle, card, device, SND_RAWMIDI_OPEN_INPUT) < 0) {
      throw(ECA_ERROR("ECA-MIDI", "unable to open ALSA raw-MIDI device " +
			rc_midi_device + "."));
    }
    ::snd_rawmidi_nonblock(&midihandle, 1);

#ifdef ALSALIB_060
    fd = ::snd_rawmidi_poll_descriptor(midihandle);
#else
    fd = ::snd_rawmidi_file_descriptor(midihandle);
#endif
#else 
    throw(ECA_ERROR("ECA-MIDI", "Unable to open ALSA raw-MIDI device, because ALSA was disabled during compilation."));
#endif
  }
  else {
    fd = ::open(rc_midi_device.c_str(), O_RDONLY);
    if (fd == -1) {
      throw(ECA_ERROR("ECA-MIDI", "unable to open OSS raw-MIDI device " +
			  rc_midi_device + "."));
    }
    ::fcntl(fd, F_SETFL, O_NONBLOCK);
  }

  ecadebug->control_flow("MIDI-thread ready " + rc_midi_device);

  while(true) {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    int retval = select(fd + 1 , &fds, NULL, NULL, NULL);
    
    if (retval) {
      if (use_alsa) {
#ifdef COMPILE_ALSA_RAWMIDI
	temp = ::snd_rawmidi_read(midihandle, buf, 16);
#endif
      }
      else {
	temp = ::read(fd, buf, 16);
      }
    }

    ::pthread_mutex_lock(&midi_in_lock_rep);
    while (midi_in_locked_rep == true) ::pthread_cond_wait(&midi_in_cond_rep,
							   &midi_in_lock_rep);
    midi_in_locked_rep = true;
    if (temp < 0) {
      cerr << "ERROR: Can't read from MIDI-device: " << midi_dev << ".\n";
      break;
    }
    for(int n = 0; n < temp; n++) {
      ::midi_in_queue.put(buf[n]);
    }
    midi_in_locked_rep = false;
    ::pthread_cond_signal(&midi_in_cond_rep);
    ::pthread_mutex_unlock(&midi_in_lock_rep);
  }

  if (use_alsa) {
#ifdef COMPILE_ALSA_RAWMIDI
    ::snd_rawmidi_close(midihandle);
#endif
  }
  else {
    ::close(fd);
  }
}
