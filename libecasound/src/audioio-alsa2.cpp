// ------------------------------------------------------------------------
// audioio-alsa2.cpp: ALSA (/dev/snd/pcm*) input/output.
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

#include <config.h>
#ifdef ALSALIB_050

#include <string>
#include <cstring>
#include <cstdio>
#include <dlfcn.h>  
#include <unistd.h>  

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>
#include <kvutils.h>

#include <sys/asoundlib.h>

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio-alsa2.h"

#include "eca-error.h"
#include "eca-debug.h"

ALSA_PCM2_DEVICE::ALSA_PCM2_DEVICE (int card, 
				    int device, 
				    int subdevice) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa2) construct");
  card_number = card;
  device_number = device;
  subdevice_number = subdevice;
  pcm_mode = SND_PCM_MODE_BLOCK;
  is_triggered = false;
  is_prepared = false;
  overruns = underruns = 0;
}

void ALSA_PCM2_DEVICE::open(void) throw(ECA_ERROR*) {
  assert(is_open() == false);
  assert(is_triggered == false);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa2) open");

  //  ::eca_alsa_load_dynamic_support();

  // -------------------------------------------------------------------
  // Channel initialization

  struct snd_pcm_channel_params params;
  ::memset(&params, 0, sizeof(params));

  // -------------------------------------------------------------------
  // Open devices

  int err;
  if (io_mode() == io_read) {
    pcm_channel = SND_PCM_CHANNEL_CAPTURE;
    err = ::snd_pcm_open_subdevice(&audio_fd, 
				   card_number, 
				   device_number,
				   subdevice_number,
				   SND_PCM_OPEN_CAPTURE | SND_PCM_OPEN_NONBLOCK);
    
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA2", "Unable to open ALSA-device for capture; error: " + 
			  string(snd_strerror(err))));
    }
  }    
  else if (io_mode() == io_write) {
    pcm_channel = SND_PCM_CHANNEL_PLAYBACK;
    err = ::snd_pcm_open_subdevice(&audio_fd, 
				   card_number, 
				   device_number,
				   subdevice_number,
				   SND_PCM_OPEN_PLAYBACK | SND_PCM_OPEN_NONBLOCK);
    if (err < 0) {
      throw(new ECA_ERROR("AUDIOIO-ALSA2", "Unable to open ALSA-device for playback; error: " +  
			  string(snd_strerror(err))));
    }
  }
  else if (io_mode() == io_readwrite) {
      throw(new ECA_ERROR("AUDIOIO-ALSA2", "Simultaneous intput/output not supported."));
  }

  // -------------------------------------------------------------------
  // Sets non-blocking mode 
  ::snd_pcm_nonblock_mode(audio_fd, 0);

  // -------------------------------------------------------------------
  // Fetch channel info

  ::memset(&pcm_info, 0, sizeof(pcm_info));
  pcm_info.channel = pcm_channel;
  ::snd_pcm_channel_info(audio_fd, &pcm_info);

  // -------------------------------------------------------------------
  // Select audio format

  ::snd_pcm_channel_flush(audio_fd, pcm_channel);
  snd_pcm_format_t pf;
  ::memset(&pf, 0, sizeof(pf));

  if ((pcm_info.flags & SND_PCM_CHNINFO_INTERLEAVE) != SND_PCM_CHNINFO_INTERLEAVE)
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "interleaved streams not supported!", ECA_ERROR::stop));
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
    case ECA_AUDIO_FORMAT::sfmt_s32_le:  { format = SND_PCM_SFMT_S32_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s32_be:  { format = SND_PCM_SFMT_S32_BE; break; }
      
    default:
      {
	throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when setting audio format not supported (1)"));
      }
    }

  unsigned int format_mask = (1 << format);
  if ((pcm_info.formats & format_mask) != format_mask)
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Selected sample format not supported by the device!", ECA_ERROR::stop));
  pf.format = format;

  if (samples_per_second() < pcm_info.min_rate ||
      samples_per_second() > pcm_info.max_rate)
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Sample rate " +
			kvu_numtostr(samples_per_second()) + " is out of range!", ECA_ERROR::stop));
  pf.rate = samples_per_second();

  if (channels() < pcm_info.min_voices ||
      channels() > pcm_info.max_voices)
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Channel count " +
			kvu_numtostr(channels()) + " is out of range!", ECA_ERROR::stop));
  pf.voices = channels();

  ::memcpy(&params.format, &pf, sizeof(pf));

  params.mode = pcm_mode;
  params.channel = pcm_channel;
  if (params.channel == SND_PCM_CHANNEL_PLAYBACK)
    params.start_mode = SND_PCM_START_GO;
  else
    params.start_mode = SND_PCM_START_DATA;
  params.stop_mode = SND_PCM_STOP_ROLLOVER; // SND_PCM_STOP_STOP

  // -------------------------------------------------------------------
  // Set fragment size.

  if (buffersize() * frame_size() < pcm_info.min_fragment_size ||
      buffersize() * frame_size() > pcm_info.max_fragment_size) 
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "buffersize " +
			kvu_numtostr(buffersize()) + " is out of range!", ECA_ERROR::stop));
  
  params.buf.block.frag_size = buffersize() * frame_size();
  params.buf.block.frags_max = 1;
  params.buf.block.frags_min = 1;

  // -------------------------------------------------------------------
  // Channel params

  err = ::snd_pcm_channel_params(audio_fd, &params);
  if (err < 0) {
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when setting up channel params: " + string(snd_strerror(err))));
  }

  struct snd_pcm_channel_setup setup;
  setup.channel = params.channel;
  setup.mode = params.mode;
  ::snd_pcm_channel_setup(audio_fd, &setup);
  fragment_size = setup.buf.block.frag_size;
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa2) Fragment size: " +
		kvu_numtostr(setup.buf.block.frag_size) + ", max: " +
		kvu_numtostr(setup.buf.block.frags_max) + ", min: " +
		kvu_numtostr(setup.buf.block.frags_min) + ", current: " +
		kvu_numtostr(setup.buf.block.frags) + ".");

//  ::snd_pcm_channel_flush(audio_fd, pcm_channel);
//  err = ::snd_pcm_channel_prepare(audio_fd, pcm_channel);
//  if (err < 0)
//    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when preparing channel: " + string(snd_strerror(err))));

  is_triggered = false;
  is_prepared = false;
  toggle_open_state(true);
}

void ALSA_PCM2_DEVICE::stop(void) {
  assert(is_triggered == true);
  assert(is_open() == true);
  assert(is_prepared == true);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa2) stop");

  snd_pcm_channel_status_t status;
  ::memset(&status, 0, sizeof(status));
  status.channel = pcm_channel;
  ::snd_pcm_channel_status(audio_fd, &status);
  overruns += status.overrun;
  underruns += status.underrun;

  int err = ::snd_pcm_channel_flush(audio_fd, pcm_channel);
  if (err < 0)
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when flushing channel: " + string(snd_strerror(err))));

  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa2) Audio device \"" + label() + "\" disabled.");

  is_triggered = false;
  is_prepared = false;
}

void ALSA_PCM2_DEVICE::close(void) {
  assert(is_open() == true);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa2) close");

  if (is_triggered == true) stop();
  ::snd_pcm_close(audio_fd);
  toggle_open_state(false);

  assert(is_triggered == false);
}

void ALSA_PCM2_DEVICE::prepare(void) {
  assert(is_triggered == false);
  assert(is_open() == true);
  assert(is_prepared == false);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa2) prepare");

  int err = ::snd_pcm_channel_prepare(audio_fd, pcm_channel);
  if (err < 0)
    throw(new ECA_ERROR("AUDIOIO-ALSA2", "Error when preparing channel: " + string(snd_strerror(err))));
  is_prepared = true;
}

void ALSA_PCM2_DEVICE::start(void) {
  assert(is_triggered == false);
  assert(is_open() == true);
  assert(is_prepared == true);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa2) start");

  if (pcm_channel == SND_PCM_CHANNEL_PLAYBACK)
    ::snd_pcm_channel_go(audio_fd, pcm_channel);
  is_triggered = true;
  //  if (io_mode() == io_write) print_status_debug();
}

long int ALSA_PCM2_DEVICE::read_samples(void* target_buffer, 
					long int samples) {
  assert(samples * frame_size() <= fragment_size);
  //    cerr << "R"; print_status_debug();
  return(::snd_pcm_read(audio_fd, target_buffer, fragment_size) / frame_size());
}

void ALSA_PCM2_DEVICE::print_status_debug(void) {
  snd_pcm_channel_status_t status;
  memset(&status, 0, sizeof(status));
  status.channel = pcm_channel;
  ::snd_pcm_channel_status(audio_fd, &status);
  overruns += status.overrun;
  underruns += status.underrun;
  cerr << "status:" << status.count << "," << status.scount << "," <<
    status.overrun << "," << status.status << " ";
  print_time_stamp();
}

void ALSA_PCM2_DEVICE::write_samples(void* target_buffer, long int samples) {
  if (samples * frame_size()== fragment_size) {
    ::snd_pcm_write(audio_fd, target_buffer, fragment_size);
    //    cerr << "W"; print_status_debug();
  }
  else {
    if ((samples * frame_size()) < pcm_info.min_fragment_size ||
	(samples * frame_size()) > pcm_info.max_fragment_size) {
      if (is_triggered) stop();
      return; 
    }
    bool was_triggered = false;
    if (is_triggered == true) { stop(); was_triggered = true; }
    close();
    buffersize(samples, samples_per_second());
    open();
    prepare();
    assert(samples * frame_size() <= fragment_size);
    ::snd_pcm_write(audio_fd, target_buffer, fragment_size);
    if (was_triggered == true) start();
  }
}

long ALSA_PCM2_DEVICE::position_in_samples(void) const {
  if (is_triggered == false) return(0);
  snd_pcm_channel_status_t status;
  memset(&status, 0, sizeof(status));
  status.channel = pcm_channel;
  ::snd_pcm_channel_status(audio_fd, &status);
  return(status.scount / frame_size());
}

ALSA_PCM2_DEVICE::~ALSA_PCM2_DEVICE(void) { 
  if (is_open() == true) close(); 

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa2) destruct");

  if (io_mode() != io_read) {
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

  //  eca_alsa_unload_dynamic_support();
}

void ALSA_PCM2_DEVICE::set_parameter(int param, 
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

  case 4: 
    subdevice_number = atoi(value.c_str());
    break;
  }
}

string ALSA_PCM2_DEVICE::get_parameter(int param) const {
  switch (param) {
  case 1: 
    return(label());

  case 2: 
    return(kvu_numtostr(card_number));

  case 3: 
    return(kvu_numtostr(device_number));

  case 4: 
    return(kvu_numtostr(subdevice_number));
  }
  return("");
}

#endif // ALSALIB_050
