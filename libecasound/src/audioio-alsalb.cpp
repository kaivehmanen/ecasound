// ------------------------------------------------------------------------
// audioio-alsalb.cpp: ALSA (/dev/snd/pcm*) loopback input.
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
#ifdef COMPILE_ALSA

#include <string>
#include <cstring>
#include <cstdio>
#include <dlfcn.h>  

#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sched.h>

#include <kvutils.h>

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio-alsalb.h"
#include "eca-alsa-dyn.h"

#include "eca-error.h"
#include "eca-debug.h"

#ifdef ALSALIB_050
void *loopback_controller(void* params);

void loopback_callback_data(void *private_data, char *buffer, size_t count);
void loopback_callback_position_change(void *private_data, unsigned int pos);
void loopback_callback_format_change(void *private_data, snd_pcm_format_t *format);
void loopback_callback_silence(void *private_data, size_t count);

size_t loopback_last_count = 0;
size_t callback_count = 0;
size_t loopback_count = 0;
char *loopback_buffer = 0;
size_t loopback_buffer_size, callback_buffer_size;
bool loopback_format_change = false;
pthread_cond_t loopback_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t loopback_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

ALSA_LOOPBACK_DEVICE::ALSA_LOOPBACK_DEVICE (int card, 
					    int device,
					    bool playback_mode) {
  card_number = card;
  device_number = device;
  is_triggered = false;
  pb_mode = playback_mode;

#ifdef ALSALIB_050
  loopback_buffer = new char [64 * 1024];
  loopback_buffer_size = 64 * 1024;
#endif
}

void ALSA_LOOPBACK_DEVICE::open(void) throw(ECA_ERROR*) {
  // #ifdef ALSALIB_050
  //  throw(new ECA_ERROR("AUDIOIO-ALSALB", "support for ALSA versions >0.5.0 not implemented"));
  //#endif

  eca_alsa_load_dynamic_support();

  if (is_open() == true) return;
  int err;
  if (io_mode() == io_read) {
    if (pb_mode) {
      if ((err = dl_snd_pcm_loopback_open(&audio_fd, 
					  card_number, 
					  device_number,
#ifdef ALSALIB_050
					  subdevice_number, // subdev
#endif
					  SND_PCM_LB_OPEN_PLAYBACK)) < 0) {
	throw(new ECA_ERROR("AUDIOIO-ALSALB", "unable to open ALSA-device for reading; error: " + string(dl_snd_strerror(err))));
      }
    }
    else {
      if ((err = dl_snd_pcm_loopback_open(&audio_fd, 
					  card_number, 
					  device_number,
#ifdef ALSALIB_050
					  subdevice_number, // subdev
#endif
					  SND_PCM_LB_OPEN_CAPTURE)) < 0) {
	throw(new ECA_ERROR("AUDIOIO-ALSALB", "unable to open ALSA-device for reading; error: " + string(dl_snd_strerror(err))));
      }
    }
  }    
  else {
      throw(new ECA_ERROR("AUDIOIO-ALSALB", "Only readinng support with a loopback device."));
  }
  
  // -------------------------------------------------------------------
  // Set blocking mode.

  dl_snd_pcm_loopback_block_mode(audio_fd, 1);    // enable block mode

  // -------------------------------------------------------------------
  // Set fragment size.

  if (buffersize() == 0) 
    throw(new ECA_ERROR("AUDIOIO-ALSALB", "Buffersize() is 0!", ECA_ERROR::stop));
  
  // -------------------------------------------------------------------
  // Select audio format

  snd_pcm_format_t loopback_format;
  memset(&loopback_format, 0, sizeof(loopback_format));

  int format;
  switch(sample_format()) 
    {
    case ECA_AUDIO_FORMAT::sfmt_u8:  { format = SND_PCM_SFMT_U8; break; }
    case ECA_AUDIO_FORMAT::sfmt_s8:  { format = SND_PCM_SFMT_S8; break; }
    case ECA_AUDIO_FORMAT::sfmt_s16_le:  { format = SND_PCM_SFMT_S16_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s16_be:  { format = SND_PCM_SFMT_S16_BE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s24_le:  { format = SND_PCM_SFMT_S24_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s24_be:  { format = SND_PCM_SFMT_S24_BE; break; }
    default: 
      throw(new ECA_ERROR("AUDIOIO-ALSALB", "Unknown sample format!", ECA_ERROR::stop));
    }

  loopback_format.rate = samples_per_second();
#ifdef ALSALIB_032
  loopback_format.channels = channels();
#else
  loopback_format.voices = channels();
#endif

  if (io_mode() == io_read) {
    err = dl_snd_pcm_loopback_format(audio_fd, &loopback_format);
    if (err < 0) {
    throw(new ECA_ERROR("AUDIOIO-ALSALB", "Error when setting up record parameters: " + string(dl_snd_strerror(err))));
    }
  }
  toggle_open_state(true);
}

void ALSA_LOOPBACK_DEVICE::close(void) {
  if (is_open()) {
    dl_snd_pcm_loopback_close(audio_fd);
  }    
  toggle_open_state(false);
}

long int ALSA_LOOPBACK_DEVICE::read_samples(void* target_buffer, 
					    long int samples) {
#ifdef ALSALIB_032
  return(dl_snd_pcm_loopback_read(audio_fd, target_buffer, frame_size() * samples) / frame_size());
#else
  static bool first_time = true;
  if (first_time == true) {
    pthread_t loopback_thread;
    int retcode = pthread_create(&loopback_thread, NULL, loopback_controller, audio_fd);
    if (retcode != 0)
      throw(new ECA_ERROR("AUDIOIO-ALSALB", "unable to create thread for alsalb"));
    
    if (sched_getscheduler(0) == SCHED_FIFO) {
      struct sched_param sparam;
      sparam.sched_priority = 10;
      if (pthread_setschedparam(loopback_thread, SCHED_FIFO, &sparam) != 0)
	ecadebug->msg("(audioio-alsalb) Unable to change scheduling policy!");
      else 
	ecadebug->msg("(audioio-alsalb) Using realtime-scheduling (SCHED_FIFO/10).");
    }
  }
  first_time = false;

  pthread_mutex_lock(&loopback_mutex);
  while(loopback_count == callback_count) {
    pthread_cond_signal(&loopback_cond);
    pthread_cond_wait(&loopback_cond, &loopback_mutex);
  }

  if (loopback_format_change == true) {
    ecadebug->msg("Warning! Loopback audio format has changed!");
    loopback_format_change = false;
  }
  if (loopback_count + samples * frame_size() > loopback_buffer_size) {
    //    cerr << "C1" << 0 << " - " << loopback_count << " - " << loopback_buffer_size - loopback_count << ".\n";
    memcpy(target_buffer,
	   loopback_buffer + loopback_count, 
	   loopback_buffer_size - loopback_count);
    //    cerr << "C2" << loopback_buffer_size - loopback_count << " - " << 0 << " - " << samples * frame_size()- loopback_buffer_size + loopback_count << ".\n";
    memcpy(reinterpret_cast<char*>(target_buffer) + loopback_buffer_size - loopback_count, 
	   loopback_buffer, 
	   samples * frame_size() - loopback_buffer_size + loopback_count);
  }
  else {
    //    cerr << "C3" << 0 << " - " << loopback_count << " - " << samples << ".\n";
    memcpy(target_buffer, 
	   loopback_buffer + loopback_count, 
	   samples * frame_size());
  }

  loopback_count += samples * frame_size();
  if (loopback_count > loopback_buffer_size) loopback_count -= loopback_buffer_size;

  pthread_mutex_unlock(&loopback_mutex);
  return(samples);
#endif
}

void *loopback_controller(void* params) {
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  
  snd_pcm_loopback_callbacks_t callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.data = loopback_callback_data;
  callbacks.position_change = loopback_callback_position_change;
  callbacks.format_change = loopback_callback_format_change;
  callbacks.silence = loopback_callback_silence;
  callback_buffer_size = callbacks.max_buffer_size = 16 * 1024;
  snd_pcm_loopback_t* handle = reinterpret_cast<snd_pcm_loopback_t*>(params);
  dl_snd_pcm_loopback_read(handle, &callbacks);

  return(0);
}

ALSA_LOOPBACK_DEVICE::~ALSA_LOOPBACK_DEVICE(void) {
  close();
  eca_alsa_unload_dynamic_support();
}

#ifdef ALSALIB_050
void loopback_callback_data(void *private_data, 
			    char *buffer,
			    size_t count) {
  //  cerr << "(audioio-alsalb) loopback_callback_data - ";
  //  cerr << "buffer " << (int)buffer[0] << ", ";
  //  cerr << "count " << count << ".\n";

  pthread_mutex_lock(&loopback_mutex);

  if (callback_count + count > loopback_buffer_size) {
    //    cerr << "C2-1: "<< callback_count << " - " << loopback_buffer_size - callback_count << ".\n";
    memcpy(loopback_buffer + callback_count, 
	   buffer,
	   loopback_buffer_size - callback_count);
    //    cerr << "C2-2: " <<  loopback_buffer_size - callback_count << " - " << count - loopback_buffer_size + callback_count << ".\n";
    memcpy(loopback_buffer + loopback_buffer_size - callback_count, 
	   buffer, 
	   count - loopback_buffer_size + callback_count);
  }
  else {
    //    cerr << "C2-3: " << callback_count << " - " << count << ".\n";
    memcpy(loopback_buffer + callback_count, buffer, count);
  }
  
  callback_count += count;
  if (callback_count > loopback_buffer_size) callback_count -= loopback_buffer_size;

  pthread_cond_signal(&loopback_cond);
  pthread_mutex_unlock(&loopback_mutex);
}

void loopback_callback_position_change(void *private_data, 
				       unsigned int pos) {
  //  cerr << "(audioio-alsalb) loopback_callback_position_change - ";
}

void loopback_callback_format_change(void *private_data,
				     snd_pcm_format_t *format) {
  //  cerr << "(audioio-alsalb) loopback_callback_format_change.\n";
  pthread_mutex_lock(&loopback_mutex);
  loopback_format_change = true;
  //  loopback_format = format;
  pthread_mutex_unlock(&loopback_mutex);
}

void loopback_callback_silence(void *private_data,
			       size_t count) {
  //  cerr << "(audioio-alsalb) loopback_callback_silence - count " << count << ".\n";
  pthread_mutex_lock(&loopback_mutex);

  if (callback_count + count > loopback_buffer_size) {
    memset(loopback_buffer + callback_count, 
	   0,
	   loopback_buffer_size - callback_count);
    memset(loopback_buffer + loopback_buffer_size - callback_count, 
	   0, 
	   count - loopback_buffer_size + callback_count);
  }
  else {
    memset(loopback_buffer + callback_count, 0, count);
  }
  
  callback_count += count;
  if (callback_count > loopback_buffer_size) callback_count -= loopback_buffer_size;

  pthread_cond_signal(&loopback_cond);
  pthread_mutex_unlock(&loopback_mutex);
}
#endif

void ALSA_LOOPBACK_DEVICE::set_parameter(int param, 
					 string value) {
  switch (param) {
  case 1: 
    label(value);
    break;

  case 2: 
    card_number = atoi(value.c_str());
    break;

  case 3: 
    device_number = atoi(value.c_str());
    break;

#ifdef ALSALIB_050
  case 4: 
    subdevice_number = atoi(value.c_str());
    break;
#endif
  }
}

string ALSA_LOOPBACK_DEVICE::get_parameter(int param) const {
  switch (param) {
  case 1: 
    return(label());

  case 2: 
    return(kvu_numtostr(card_number));

  case 3: 
    return(kvu_numtostr(device_number));

#ifdef ALSALIB_050
  case 4: 
    return(kvu_numtostr(subdevice_number));
#endif
  }
  return("");
}

#endif // COMPILE_ALSA
