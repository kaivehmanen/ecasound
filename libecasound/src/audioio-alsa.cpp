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

#include <config.h>
// #ifdef COMPILE_ALSA
#ifdef ALSALIB_032

#include <string>
#include <cstring>
#include <cstdio>
#include <dlfcn.h>  

#include <kvutils.h>

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio-alsa.h"
#include "eca-alsa-dyn.h"

#include "eca-error.h"
#include "eca-debug.h"

ALSA_PCM_DEVICE::ALSA_PCM_DEVICE (int card, 
			int device, 
			const SIMODE mode, 
			const ECA_AUDIO_FORMAT& form, 
			long int bsize)
  :  AUDIO_IO_DEVICE(string("alsa,") + kvu_numtostr(card) +
		   string(",") + kvu_numtostr(device) , mode, form)
{
  card_number = card;
  device_number = device;
  is_triggered = false;
  overruns = underruns = 0;
  eca_alsa_load_dynamic_support();
  buffersize(bsize, samples_per_second());
}

void ALSA_PCM_DEVICE::open(void) {
  if (is_open() == true) return;
  int err;
  if (io_mode() == si_read) {
#ifdef ALSALIB_031
    err = dl_snd_pcm_open(&audio_fd, 
			  card_number, 
			  device_number,
			  SND_PCM_OPEN_RECORD);
#else
    err = dl_snd_pcm_open(&audio_fd, 
			  card_number, 
			  device_number,
			  SND_PCM_OPEN_CAPTURE);
#endif
    if (err != 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "unable to open ALSA-device for recording; error: " + string(dl_snd_strerror(err))));
    }
  }    
  else if (io_mode() == si_write) {
    err = dl_snd_pcm_open(&audio_fd, 
			  card_number, 
			  device_number,
			  SND_PCM_OPEN_PLAYBACK);
    if (err != 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "unable to open ALSA-device for playback; error: " +  string(dl_snd_strerror(err))));
    }
    // ---
    // output triggering
    dl_snd_pcm_playback_pause(audio_fd, 1);
  }
  else if (io_mode() == si_readwrite) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Simultaneous intput/ouput not supported."));
  }

  // -------------------------------------------------------------------
  // Set blocking mode.

  dl_snd_pcm_block_mode(audio_fd, 1);    // enable block mode

  // -------------------------------------------------------------------
  // Set fragment size.

  if (buffersize() == 0) 
    throw(new ECA_ERROR("AUDIOIO-ALSA", "buffersize() is 0!", ECA_ERROR::stop));
    
  if (io_mode() == si_read) {
#ifdef ALSALIB_031
    snd_pcm_record_info_t pcm_info;
    snd_pcm_record_params_t pp;
#else
    snd_pcm_capture_info_t pcm_info;
    snd_pcm_capture_params_t pp;
#endif
    dl_snd_pcm_capture_info(audio_fd, &pcm_info);
    memset(&pp, 0, sizeof(pp));

    if (buffersize() * frame_size() > (int)pcm_info.buffer_size) 
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Buffer size too big, can't setup fragments."));

    pp.fragment_size = buffersize() * frame_size();
    pp.fragments_min = 1;

    err = dl_snd_pcm_capture_params(audio_fd, &pp);

    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting up buffer fragments: " + string(dl_snd_strerror(err))));
    }
  }
  else {
    snd_pcm_playback_info_t pcm_info;
    dl_snd_pcm_playback_info(audio_fd, &pcm_info);

    snd_pcm_playback_params_t pp;
    memset(&pp, 0, sizeof(pp));

    pp.fragment_size = buffersize() * frame_size();
    //    pp.fragments_max = (pcm_info.buffer_size / pp.fragment_size) - 1;
    pp.fragments_max = -1;
    pp.fragments_room = 1;
    
    err = dl_snd_pcm_playback_params(audio_fd, &pp);
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting up buffer fragments: " + string(dl_snd_strerror(err))));
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
      
    default:
      {
	throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting audio format not supported (1)"));
      }
    }

  pf.format = format;
  pf.rate = samples_per_second();
  pf.channels = channels();

  if (io_mode() == si_read) {
    err = dl_snd_pcm_capture_format(audio_fd, &pf);
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting up record parameters: " + string(dl_snd_strerror(err))));
    }
  }
  else {
    err = dl_snd_pcm_playback_format(audio_fd, &pf);
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA", "Error when setting up playback parameters: " + string(dl_snd_strerror(err))));
    }
  }
  toggle_open_state(true);
}

void ALSA_PCM_DEVICE::stop(void) {
  ecadebug->msg(1, "(audioio-alsa) Audio device \"" + label() + "\" disabled.");
  if (io_mode() == si_write) {
    dl_snd_pcm_playback_pause(audio_fd, 1);
  }
  else {
    if (is_open()) close();
  }
  is_triggered = false;
}

void ALSA_PCM_DEVICE::close(void) {
  if (is_open()) {
    if (io_mode() != si_read) {
      snd_pcm_playback_status_t pb_status;
      dl_snd_pcm_playback_status(audio_fd, &pb_status);
      underruns += pb_status.underrun;
      dl_snd_pcm_drain_playback(audio_fd);
    }
    else if (io_mode() == si_read) {
#ifdef ALSALIB_031
      snd_pcm_record_status_t ca_status;
#else
      snd_pcm_capture_status_t ca_status;
#endif
      dl_snd_pcm_capture_status(audio_fd, &ca_status);
      overruns += ca_status.overrun;
      dl_snd_pcm_flush_capture(audio_fd);
    }
    dl_snd_pcm_close(audio_fd);
  }    
  toggle_open_state(false);
}

void ALSA_PCM_DEVICE::start(void) {
  if (is_open() == false) {
    open();
  }
  if (is_triggered == false) {
    if (io_mode() == si_write) {
      snd_pcm_playback_status_t pb_status;
      dl_snd_pcm_playback_status(audio_fd, &pb_status);
      ecadebug->msg(2, "(audioio-alsa) Bytes in output-queue: " + kvu_numtostr(pb_status.queue) + ".");
      dl_snd_pcm_playback_pause(audio_fd, 0);
    }
    else {
      dl_snd_pcm_flush_capture(audio_fd);
    }
    is_triggered = true;
  }
}

long int ALSA_PCM_DEVICE::read_samples(void* target_buffer, 
				 long int samples) {
  return(dl_snd_pcm_read(audio_fd, target_buffer, frame_size() * samples) / frame_size());
}

void ALSA_PCM_DEVICE::write_samples(void* target_buffer, long int samples) {
  dl_snd_pcm_write(audio_fd, target_buffer, frame_size() * samples);
}

ALSA_PCM_DEVICE::~ALSA_PCM_DEVICE(void) { 
  close(); 

  if (io_mode() != si_read) {
    if (underruns != 0) {
      cerr << "(audioio-alsa) WARNING! While writing to ALSA-pcm device ";
      cerr << "C" << card_number << "D" << device_number;
      cerr << ", there were " << underruns << " underruns.\n";
    }
  }
  else {
    if (overruns != 0) {
      cerr << "(audioio-alsa) WARNING! While reading from ALSA-pcm device ";
      cerr << "C" << card_number << "D" << device_number;
      cerr << ", there were " << overruns << " overruns.\n";
    }
  }

  eca_alsa_unload_dynamic_support();
}

#endif // ALSALIB_032
// #endif // COMPILE_ALSA

