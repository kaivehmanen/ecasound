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

#include <kvutils.h>

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio-alsalb.h"
#include "eca-alsa-dyn.h"

#include "eca-error.h"
#include "eca-debug.h"

ALSALBDEVICE::ALSALBDEVICE (int card, 
			    int device, 
			    const SIMODE mode, 
			    const ECA_AUDIO_FORMAT& form, 
			    long int bsize,
			    bool playback_mode)
  :  AUDIO_IO_DEVICE(string("alsa_loopback,") + kvu_numtostr(card) +
		     string(",") + kvu_numtostr(device) , mode, form)
{
  buffersize(bsize, samples_per_second());
  card_number = card;
  device_number = device;
  is_triggered = false;
  pb_mode = playback_mode;
  eca_alsa_load_dynamic_support();
}

void ALSALBDEVICE::open(void) throw(ECA_ERROR*) {
  if (is_open() == true) return;
  int err;
  if (io_mode() == si_read) {
    if (pb_mode) {
      if ((err = dl_snd_pcm_loopback_open(&audio_fd, 
					  card_number, 
					  device_number,
					  SND_PCM_LB_OPEN_PLAYBACK)) < 0) {
	throw(new ECA_ERROR("AUDIOIO-ALSALB", "unable to open ALSA-device for reading; error: " + string(dl_snd_strerror(err))));
      }
    }
    else {
      if ((err = dl_snd_pcm_loopback_open(&audio_fd, 
					  card_number, 
					  device_number,
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
    default: format = ECA_AUDIO_FORMAT::default_sample_format;
  }

  pf.rate = samples_per_second();
  pf.channels = channels();

  if (io_mode() == si_read) {
    err = dl_snd_pcm_loopback_format(audio_fd, &pf);
    if (err < 0) {
    throw(new ECA_ERROR("AUDIOIO-ALSALB", "Error when setting up record parameters: " + string(dl_snd_strerror(err))));
    }
  }
  toggle_open_state(true);
}

void ALSALBDEVICE::close(void) {
  if (is_open()) {
    dl_snd_pcm_loopback_close(audio_fd);
  }    
  toggle_open_state(false);
}

long int ALSALBDEVICE::read_samples(void* target_buffer, 
				 long int samples) {
  return(dl_snd_pcm_loopback_read(audio_fd, target_buffer, frame_size() * samples) / frame_size());
}

ALSALBDEVICE::~ALSALBDEVICE(void) {
  close();
  eca_alsa_unload_dynamic_support();
}

#endif // COMPILE_ALSA

