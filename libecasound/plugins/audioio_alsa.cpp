// ------------------------------------------------------------------------
// audioio-alsa.cpp: ALSA (/dev/snd/pcm*) input/output.
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

#include <string>
#include <cstring>
#include <cstdio>
#include <dlfcn.h>  
#include <kvutils.h>

#include "samplebuffer.h"
#include "audioio-types.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef ALSALIB_032
#include <sys/asoundlib.h>
#include "audioio-alsa.h"

#include "eca-error.h"
#include "eca-debug.h"

ALSA_PCM_DEVICE::ALSA_PCM_DEVICE (int card, 
				  int device) {
  card_number_rep = card;
  device_number_rep = device;
  is_triggered_rep = false;
  overruns_rep = underruns_rep = 0;
}

void ALSA_PCM_DEVICE::open(void) throw(ECA_ERROR*) {
  if (is_open() == true) return;

  int err;
  if (io_mode() == io_read) {
    err = ::snd_pcm_open(&audio_fd_repp, 
			 card_number_rep, 
			 device_number_rep,
			 SND_PCM_OPEN_CAPTURE);
    if (err != 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "unable to open ALSA-device for recording; error: " + string(snd_strerror(err))));
    }
  }    
  else if (io_mode() == io_write) {
    err = ::snd_pcm_open(&audio_fd_repp, 
			  card_number_rep, 
			  device_number_rep,
			  SND_PCM_OPEN_PLAYBACK);
    if (err != 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "unable to open ALSA-device for playback; error: " +  string(snd_strerror(err))));
    }
    // ---
    // output triggering
    ::snd_pcm_playback_pause(audio_fd_repp, 1);
  }
  else if (io_mode() == io_readwrite) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Simultaneous intput/ouput not supported."));
  }

  // -------------------------------------------------------------------
  // Set blocking mode.

  ::snd_pcm_block_mode(audio_fd_repp, 1);    // enable block mode

  // -------------------------------------------------------------------
  // Set fragment size.

  if (buffersize() == 0) 
    throw(new ECA_ERROR("AUDIOIO-ALSA", "buffersize() is 0!", ECA_ERROR::stop));
    
  if (io_mode() == io_read) {
    snd_pcm_capture_info_t pcm_info;
    snd_pcm_capture_params_t pp;
    ::snd_pcm_capture_info(audio_fd_repp, &pcm_info);
    memset(&pp, 0, sizeof(pp));

    if (buffersize() * frame_size() > (int)pcm_info.buffer_size) 
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Buffer size too big, can't setup fragments."));

    pp.fragment_size = buffersize() * frame_size();
    pp.fragments_min = 1;

    err = ::snd_pcm_capture_params(audio_fd_repp, &pp);

    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting up buffer fragments: " + string(snd_strerror(err))));
    }
  }
  else {
    snd_pcm_playback_info_t pcm_info;
    ::snd_pcm_playback_info(audio_fd_repp, &pcm_info);

    snd_pcm_playback_params_t pp;
    memset(&pp, 0, sizeof(pp));

    pp.fragment_size = buffersize() * frame_size();
    //    pp.fragments_max = (pcm_info.buffer_size / pp.fragment_size) - 1;
    pp.fragments_max = -1;
    pp.fragments_room = 1;
    
    err = ::snd_pcm_playback_params(audio_fd_repp, &pp);
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting up buffer fragments: " + string(snd_strerror(err))));
    }
  }

  // -------------------------------------------------------------------
  // Select audio format

  snd_pcm_format_t pf;

  memset(&pf, 0, sizeof(pf));
  int format;
  switch(sample_format()) 
    {
    case ECA_AUDIO_FORMAT::sfmt_u8:  { format = SND_PCM_SFMT_U8; break; }
    case ECA_AUDIO_FORMAT::sfmt_s8:  { format = SND_PCM_SFMT_S8; break; }
    case ECA_AUDIO_FORMAT::sfmt_s16_le:  { format = SND_PCM_SFMT_S16_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s16_be:  { format = SND_PCM_SFMT_S16_BE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s24_le:  { format = SND_PCM_SFMT_S24_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s24_be:  { format = SND_PCM_SFMT_S24_BE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s32_le:  { format = SND_PCM_SFMT_S32_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s32_be:  { format = SND_PCM_SFMT_S32_BE; break; }
      
    default:
      {
	throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting audio format not supported (1)"));
      }
    }

  pf.format = format;
  pf.rate = samples_per_second();
  pf.channels = channels();

  if (io_mode() == io_read) {
    ::snd_pcm_capture_time(audio_fd_repp, 1);
    err = ::snd_pcm_capture_format(audio_fd_repp, &pf);
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting up record parameters: " + string(snd_strerror(err))));
    }
  }
  else {
    ::snd_pcm_playback_time(audio_fd_repp, 1);
    err = ::snd_pcm_playback_format(audio_fd_repp, &pf);
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting up playback parameters: " + string(snd_strerror(err))));
    }
  }

  is_triggered_rep = false;
  is_prepared_rep = false;
  toggle_open_state(true);
}

void ALSA_PCM_DEVICE::stop(void) {
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa) Audio device \"" + label() + "\" disabled.");
  if (io_mode() == io_write) {
    ::snd_pcm_playback_pause(audio_fd_repp, 1);
  }
  else {
    if (is_open()) close();
  }
  is_triggered_rep = false;
}

void ALSA_PCM_DEVICE::close(void) {
  if (is_open()) {
    if (io_mode() != io_read) {
      snd_pcm_playback_status_t pb_status;
      ::snd_pcm_playback_status(audio_fd_repp, &pb_status);
      underruns_rep += pb_status.underrun;
      ::snd_pcm_drain_playback(audio_fd_repp);
    }
    else if (io_mode() == io_read) {
      snd_pcm_capture_status_t ca_status;
      ::snd_pcm_capture_status(audio_fd_repp, &ca_status);
      overruns_rep += ca_status.overrun;
      ::snd_pcm_flush_capture(audio_fd_repp);
    }
    ::snd_pcm_close(audio_fd_repp);
  }    
  toggle_open_state(false);
}

void ALSA_PCM_DEVICE::start(void) {
  if (is_open() == false) {
    open();
  }
  if (is_triggered_rep == false) {
    if (io_mode() == io_write) {
      snd_pcm_playback_status_t pb_status;
      ::snd_pcm_playback_status(audio_fd_repp, &pb_status);
      ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa) Bytes in output-queue: " + kvu_numtostr(pb_status.queue) + ".");
      ::snd_pcm_playback_pause(audio_fd_repp, 0);
    }
    else {
      ::snd_pcm_flush_capture(audio_fd_repp);
    }
    is_triggered_rep = true;
  }
}

long int ALSA_PCM_DEVICE::read_samples(void* target_buffer, 
				 long int samples) {
  return(::snd_pcm_read(audio_fd_repp, target_buffer, frame_size() * samples) / frame_size());
}

void ALSA_PCM_DEVICE::write_samples(void* target_buffer, long int samples) {
  ::snd_pcm_write(audio_fd_repp, target_buffer, frame_size() * samples);
}

long ALSA_PCM_DEVICE::position_in_samples(void) const {
  if (is_triggered_rep == false) return(0);
  if (io_mode() != io_read) {
    snd_pcm_playback_status_t pb_status;
    ::snd_pcm_playback_status(audio_fd_repp, &pb_status);
    double time = pb_status.stime.tv_sec * 1000000.0 + pb_status.stime.tv_usec;
    return(static_cast<long>(time * samples_per_second() / 1000000.0));
  }
  snd_pcm_capture_status_t ca_status;
  ::snd_pcm_capture_status(audio_fd_repp, &ca_status);
  double time = ca_status.stime.tv_sec * 1000000.0 + ca_status.stime.tv_usec;
  return(static_cast<long>(time * samples_per_second() / 1000000.0));
    //  return(ca_status.scount / frame_size());
}

ALSA_PCM_DEVICE::~ALSA_PCM_DEVICE(void) { 
  close(); 

  if (io_mode() != io_read) {
    if (underruns_rep != 0) {
      cerr << "(audioio-alsa) WARNING! While writing to ALSA-pcm device ";
      cerr << "C" << card_number_rep << "D" << device_number_rep;
      cerr << ", there were " << underruns_rep << " underruns_rep.\n";
    }
  }
  else {
    if (overruns_rep != 0) {
      cerr << "(audioio-alsa) WARNING! While reading from ALSA-pcm device ";
      cerr << "C" << card_number_rep << "D" << device_number_rep;
      cerr << ", there were " << overruns_rep << " overruns_rep.\n";
    }
  }
  //  eca_alsa_unload_dynamic_support();
}

void ALSA_PCM_DEVICE::set_parameter(int param, 
				     string value) {
  switch (param) {
  case 1: 
    label(value);
    break;

  case 2: 
    card_number_rep = atoi(value.c_str());
    break;

  case 3: 
    device_number_rep = atoi(value.c_str());
    break;
  }
}

string ALSA_PCM_DEVICE::get_parameter(int param) const {
  switch (param) {
  case 1: 
    return(label());

  case 2: 
    return(kvu_numtostr(card_number_rep));

  case 3: 
    return(kvu_numtostr(device_number_rep));
  }
  return("");
}

#endif // ALSALIB_032
