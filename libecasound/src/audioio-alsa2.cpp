// ------------------------------------------------------------------------
// audioio-alsa.cpp: ALSA (/dev/snd/pcm*) input/output.
// Copyright (C) 1999,2000 Kai Vehmanen (kaiv@wakkanet.fi)
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
#ifdef ALSALIB_050

#include <string>
#include <cstring>
#include <cstdio>
#include <dlfcn.h>  
#include <unistd.h>  

#include <kvutils.h>

#include <sys/asoundlib.h>

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio-alsa2.h"
#include "eca-alsa-dyn.h"

#include "eca-error.h"
#include "eca-debug.h"

ALSA_PCM2_DEVICE::ALSA_PCM2_DEVICE (int card, 
				    int device, 
				    int subdevice, 
				    const SIMODE mode, 
				    const ECA_AUDIO_FORMAT& form, 
				    long int bsize)
  :  AUDIO_IO_DEVICE(string("alsa,") + kvu_numtostr(card) +
		     string(",") + kvu_numtostr(device) +
		     string(",") + kvu_numtostr(subdevice), mode, form)
{
  card_number = card;
  device_number = device;
  subdevice_number = subdevice;
  pcm_mode = SND_PCM_MODE_BLOCK;
  is_triggered = false;
  is_prepared = false;
  overruns = underruns = 0;
  eca_alsa_load_dynamic_support();
  buffersize(bsize, samples_per_second());
}

void ALSA_PCM2_DEVICE::open(void) throw(ECA_ERROR*) {
  if (is_open() == true) return;

  // -------------------------------------------------------------------
  // Channel initialization

  struct snd_pcm_channel_params params;
  memset(&params, 0, sizeof(params));

  // -------------------------------------------------------------------
  // Open devices

  int err;
  if (io_mode() == si_read) {
    pcm_channel = SND_PCM_CHANNEL_CAPTURE;
    err = dl_snd_pcm_open_subdevice(&audio_fd, 
				    card_number, 
				    device_number,
				    subdevice_number,
				    SND_PCM_OPEN_CAPTURE | SND_PCM_OPEN_NONBLOCK);
    
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA2", "unable to open ALSA-device for capture; error: " + 
			  string(dl_snd_strerror(err))));
    }
  }    
  else if (io_mode() == si_write) {
    pcm_channel = SND_PCM_CHANNEL_PLAYBACK;
    err = dl_snd_pcm_open_subdevice(&audio_fd, 
				    card_number, 
				    device_number,
				    subdevice_number,
				    SND_PCM_OPEN_PLAYBACK | SND_PCM_OPEN_NONBLOCK);
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA2", "unable to open ALSA-device for playback; error: " +  
			  string(dl_snd_strerror(err))));
    }
  }
  else if (io_mode() == si_readwrite) {
      throw(new ECA_ERROR("AUDIOIO-ALSA2", "Simultaneous intput/output not supported."));
  }

  dl_snd_pcm_nonblock_mode(audio_fd, 0);

  // -------------------------------------------------------------------
  // Select audio format

  dl_snd_pcm_channel_flush(audio_fd, pcm_channel);
  snd_pcm_format_t pf;
  memset(&pf, 0, sizeof(pf));

  pf.interleave = 1;
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
	throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when setting audio format not supported (1)"));
      }
    }

  pf.format = format;
  pf.rate = samples_per_second();
  pf.voices = channels();

  memcpy(&params.format, &pf, sizeof(pf));

  if (pcm_channel == SND_PCM_CHANNEL_PLAYBACK)
    params.start_mode = SND_PCM_START_GO;
  else 
    params.start_mode = SND_PCM_START_GO;
  params.stop_mode = SND_PCM_STOP_ROLLOVER;

  //  params.stop_mode = SND_PCM_STOP_STOP;

  params.mode = pcm_mode;
  params.channel = pcm_channel;

  // -------------------------------------------------------------------
  // Set fragment size.

  if (buffersize() == 0) 
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "buffersize() is 0!", ECA_ERROR::stop));

  params.buf.block.frag_size = buffersize() * frame_size();
  params.buf.block.frags_max = -1;
  params.buf.block.frags_min = 1;

  // -------------------------------------------------------------------
  // Channel params

  err = dl_snd_pcm_channel_params(audio_fd, &params);
  if (err < 0) {
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when setting up channel params: " + string(dl_snd_strerror(err))));
  }

  struct snd_pcm_channel_setup setup;
  setup.channel = params.channel;
  setup.mode = params.mode;
  dl_snd_pcm_channel_setup(audio_fd, &setup);
  fragment_size = setup.buf.block.frag_size;
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa2) Fragment size: " +
		kvu_numtostr(setup.buf.block.frag_size) + ", max: " +
		kvu_numtostr(setup.buf.block.frags_max) + ", min: " +
		kvu_numtostr(setup.buf.block.frags_min) + ", current: " +
		kvu_numtostr(setup.buf.block.frags) + ".");

//  dl_snd_pcm_channel_flush(audio_fd, pcm_channel);
//  err = dl_snd_pcm_channel_prepare(audio_fd, pcm_channel);
//  if (err < 0)
//    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when preparing channel: " + string(dl_snd_strerror(err))));

  toggle_open_state(true);
}

void ALSA_PCM2_DEVICE::stop(void) {
  assert(is_triggered == true);
  assert(is_open() == true);
  assert(is_prepared == true);

  snd_pcm_channel_status_t status;
  memset(&status, 0, sizeof(status));
  status.channel = pcm_channel;
  dl_snd_pcm_channel_status(audio_fd, &status);
  underruns += status.underrun;

  int err = dl_snd_pcm_channel_flush(audio_fd, pcm_channel);
  if (err < 0)
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when flushing channel: " + string(dl_snd_strerror(err))));

  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa2) Audio device \"" + label() + "\" disabled.");

  is_triggered = false;
  is_prepared = false;
}

void ALSA_PCM2_DEVICE::close(void) {
  assert(is_open() == true);

  if (is_triggered == true) stop();
  dl_snd_pcm_close(audio_fd);
  toggle_open_state(false);
}

void ALSA_PCM2_DEVICE::prepare(void) {
  assert(is_open() == true);
  assert(is_prepared == false);

  int err = dl_snd_pcm_channel_prepare(audio_fd, pcm_channel);
  if (err < 0)
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when preparing channel: " + string(dl_snd_strerror(err))));
  is_prepared = true;
}

void ALSA_PCM2_DEVICE::start(void) {
  assert(is_triggered == false);
  assert(is_open() == true);
  assert(is_prepared == true);

  if (io_mode() == si_write) {
    snd_pcm_channel_status_t status;
    memset(&status, 0, sizeof(status));
    status.channel = pcm_channel;
    dl_snd_pcm_channel_status(audio_fd, &status);
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa2) Bytes in output-queue: " + kvu_numtostr(status.count) + ".");
  }
  dl_snd_pcm_channel_go(audio_fd, pcm_channel);
  is_triggered = true;
}

long int ALSA_PCM2_DEVICE::read_samples(void* target_buffer, 
					long int samples) {
  assert(samples * frame_size() <= fragment_size);
  return(dl_snd_pcm_read(audio_fd, target_buffer, fragment_size) / frame_size());
}

void ALSA_PCM2_DEVICE::write_samples(void* target_buffer, long int samples) {
  dl_snd_pcm_write(audio_fd, target_buffer, fragment_size);
}

long ALSA_PCM2_DEVICE::position_in_samples(void) const {
  if (is_triggered == false) return(0);
  snd_pcm_channel_status_t status;
  memset(&status, 0, sizeof(status));
  status.channel = pcm_channel;
  dl_snd_pcm_channel_status(audio_fd, &status);
  return(status.scount / frame_size());
}

ALSA_PCM2_DEVICE::~ALSA_PCM2_DEVICE(void) { 
  if (is_open() == true) close(); 

  if (io_mode() != si_read) {
    if (underruns != 0) {
      cerr << "(audioio-alsa2) WARNING! While writing to ALSA-pcm device ";
      cerr << "C" << card_number << "D" << device_number;
      cerr << ", there were " << underruns << " underruns.\n";
    }
  }
  else {
    if (overruns != 0) {
      cerr << "(audioio-alsa2) WARNING! While reading from ALSA-pcm device ";
      cerr << "C" << card_number << "D" << device_number;
      cerr << ", there were " << overruns << " overruns.\n";
    }
  }

  eca_alsa_unload_dynamic_support();
}

#endif // ALSALIB_050
