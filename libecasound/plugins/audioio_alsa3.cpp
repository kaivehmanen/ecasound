// ------------------------------------------------------------------------
// audioio-alsa3.cpp: ALSA (/dev/snd/pcm*) input/output.
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi),
//                         Jeremy Hall (jhall@uu.net)
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
#include <unistd.h>  

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>
#include <kvutils.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ALSALIB_060
#include <sys/asoundlib.h>

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio_alsa3.h"

#include "eca-error.h"
#include "eca-debug.h"

ALSA_PCM_DEVICE::ALSA_PCM_DEVICE (int card, 
				  int device, 
				  int subdevice) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) construct");
  card_number_rep = card;
  device_number_rep = device;
  subdevice_number_rep = subdevice;
  is_triggered_rep = false;
  is_prepared_rep = false;
  overruns_rep = underruns_rep = 0;
}

void ALSA_PCM_DEVICE::open(void) throw(ECA_ERROR*) {
  assert(is_open() == false);
  assert(is_triggered_rep == false);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) open");

  // -------------------------------------------------------------------
  // Channel initialization

  if (label() == "alsaplugin") {
    using_plugin_rep = true;
  }

  struct snd_pcm_params params;
  ::memset(&params, 0, sizeof(params));

  // -------------------------------------------------------------------
  // Open devices

  int err;
  if (io_mode() == io_read) {
    pcm_stream_rep = SND_PCM_STREAM_CAPTURE;
    if (using_plugin_rep)
      err = ::snd_pcm_plug_open_subdevice(&audio_fd_repp, 
					  card_number_rep, 
					  device_number_rep,
					  subdevice_number_rep,
					  pcm_stream_rep,
					  SND_PCM_NONBLOCK);
    else
      err = ::snd_pcm_hw_open_subdevice(&audio_fd_repp, 
				     card_number_rep, 
				     device_number_rep,
				     subdevice_number_rep,
				     pcm_stream_rep,
				     SND_PCM_NONBLOCK);
    
    if (err < 0) {
      if (using_plugin_rep)
	throw(new ECA_ERROR("AUDIOIO-ALSA3", "Unable to open ALSA-plugin-device for capture; error: " + 
			    string(snd_strerror(err))));
      else
	throw(new ECA_ERROR("AUDIOIO-ALSA3", "Unable to open ALSA-device for capture; error: " + 
			    string(snd_strerror(err))));
    }
  }    
  else if (io_mode() == io_write) {
    pcm_stream_rep = SND_PCM_STREAM_PLAYBACK;
    if (using_plugin_rep)
      err = ::snd_pcm_plug_open_subdevice(&audio_fd_repp, 
					  card_number_rep, 
					  device_number_rep,
					  subdevice_number_rep,
					  pcm_stream_rep,
					  SND_PCM_NONBLOCK);
    else
      err = ::snd_pcm_hw_open_subdevice(&audio_fd_repp, 
				     card_number_rep, 
				     device_number_rep,
				     subdevice_number_rep,
				     pcm_stream_rep,
				     SND_PCM_NONBLOCK);

    if (err < 0) {
      if (using_plugin_rep)
	throw(new ECA_ERROR("AUDIOIO-ALSA3", "Unable to open ALSA-plugin-device for playback; error: " +  
			    string(snd_strerror(err))));
      else
	throw(new ECA_ERROR("AUDIOIO-ALSA3", "Unable to open ALSA-device for playback; error: " +  
			    string(snd_strerror(err))));
    }
  }
  else if (io_mode() == io_readwrite) {
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "Simultaneous input/output not supported."));
  }

  // -------------------------------------------------------------------
  // Sets non-blocking mode 
  ::snd_pcm_nonblock(audio_fd_repp, 0);

  // -------------------------------------------------------------------
  // Fetch stream info

  ::memset(&pcm_info_rep, 0, sizeof(pcm_info_rep));
  pcm_info_rep.stream = pcm_stream_rep;
  ::snd_pcm_info(audio_fd_repp, &pcm_info_rep);

  ::memset(&pcm_params_info_rep, 0, sizeof(pcm_params_info_rep));
  ::snd_pcm_params_info(audio_fd_repp, &pcm_params_info_rep);

  // -------------------------------------------------------------------
  // Select audio format

  ::snd_pcm_flush(audio_fd_repp);

  snd_pcm_format_t pf;
  ::memset(&pf, 0, sizeof(pf));

  if (channels() > 1 &&
      (pcm_params_info_rep.flags & SND_PCM_INFO_INTERLEAVED) != SND_PCM_INFO_INTERLEAVED)
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "device can't handle interleaved streams!", ECA_ERROR::stop));
//    pf.interleave = 1;

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
	throw(new ECA_ERROR("AUDIOIO-ALSA3", "Error when setting audio format not supported (1)"));
      }
    }

  unsigned int format_mask = (1 << format);
  if ((pcm_params_info_rep.formats & format_mask) != format_mask)
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "Selected sample format not supported by the device!", ECA_ERROR::stop));
  pf.sfmt = format;

  if (static_cast<unsigned int>(samples_per_second()) < pcm_params_info_rep.min_rate ||
      static_cast<unsigned int>(samples_per_second()) > pcm_params_info_rep.max_rate)
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "Sample rate " +
			kvu_numtostr(samples_per_second()) + " is out of range!", ECA_ERROR::stop));
  pf.rate = samples_per_second();

  if (static_cast<unsigned int>(channels()) < pcm_params_info_rep.min_channels ||
      static_cast<unsigned int>(channels()) > pcm_params_info_rep.max_channels)
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "Channel count " +
			kvu_numtostr(channels()) + " is out of range!", ECA_ERROR::stop));
  pf.channels = channels();

  ::memcpy(&params.format, &pf, sizeof(pf));

  if (pcm_stream_rep == SND_PCM_STREAM_PLAYBACK)
    params.start_mode = SND_PCM_START_EXPLICIT;
  else
    params.start_mode = SND_PCM_START_DATA;

  params.xfer_mode = SND_PCM_XFER_INTERLEAVED;
  params.xrun_mode = SND_PCM_XRUN_FRAGMENT;
  params.xrun_act = SND_PCM_XRUN_ACT_RESTART;

  // -------------------------------------------------------------------
  // Fetch stream info (2nd time, now getting frame size info)
  pcm_params_info_rep.req.format.sfmt = format;
  pcm_params_info_rep.req.format.channels = channels();
  pcm_params_info_rep.req_mask = SND_PCM_PARAMS_SFMT | SND_PCM_PARAMS_CHANNELS;
  err = ::snd_pcm_params_info(audio_fd_repp, &pcm_params_info_rep);
  if (err < 0)
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "Error when querying stream info: " + string(snd_strerror(err))));

  // -------------------------------------------------------------------
  // Set fragment size.

  if (static_cast<unsigned int>(buffersize()) < pcm_params_info_rep.min_fragment_size ||
      static_cast<unsigned int>(buffersize()) > pcm_params_info_rep.max_fragment_size) 
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "buffersize " +
			kvu_numtostr(buffersize()) + " is out of range!", ECA_ERROR::stop));
  // <--
  params.buffer_size = pcm_params_info_rep.buffer_size;
  params.frag_size = buffersize();
  params.xfer_align = params.frag_size;
  params.xrun_max = ~0U;
  params.xfer_min = params.frag_size;
  params.avail_min = params.frag_size;

  // -------------------------------------------------------------------
  // Stream params
  err = ::snd_pcm_params(audio_fd_repp, &params);
  if (err < 0)
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "Error when setting up stream params: " + string(snd_strerror(err))));

  // -------------------------------------------------------------------
  // Fetch the current pcm-setup and print out some debug info
  struct snd_pcm_setup setup;
  setup.xfer_mode = params.xfer_mode;
  ::snd_pcm_setup(audio_fd_repp, &setup);

  fragment_size_rep = setup.frag_size;
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Fragment size: " +
		kvu_numtostr(setup.frag_size) + ", max: " +
		kvu_numtostr(setup.xrun_max) + ", min: " +
		kvu_numtostr(setup.xfer_min) + ", current: " +
		kvu_numtostr(setup.frags) + ".");

  is_triggered_rep = false;
  is_prepared_rep = false;
  toggle_open_state(true);
}

void ALSA_PCM_DEVICE::stop(void) {
  assert(is_triggered_rep == true);
  assert(is_open() == true);
  assert(is_prepared_rep == true);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) stop");

  snd_pcm_status_t status;
  ::memset(&status, 0, sizeof(status));
  ::snd_pcm_status(audio_fd_repp, &status);
  if (pcm_stream_rep == SND_PCM_STREAM_PLAYBACK)
    underruns_rep += status.xruns;
  else if (pcm_stream_rep == SND_PCM_STREAM_CAPTURE)
    overruns_rep += status.xruns;

  int err = ::snd_pcm_flush(audio_fd_repp);
  if (err < 0)
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "Error when flushing stream: " + string(snd_strerror(err))));
  
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Audio device \"" + label() + "\" disabled.");
  
  is_triggered_rep = false;
  is_prepared_rep = false;
}

void ALSA_PCM_DEVICE::close(void) {
  assert(is_open() == true);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) close");

  if (is_triggered_rep == true) stop();
  ::snd_pcm_close(audio_fd_repp);
  toggle_open_state(false);

  assert(is_triggered_rep == false);
}

void ALSA_PCM_DEVICE::prepare(void) {
  assert(is_triggered_rep == false);
  assert(is_open() == true);
  assert(is_prepared_rep == false);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) prepare");

  int err = ::snd_pcm_prepare(audio_fd_repp);
  if (err < 0)
    throw(new ECA_ERROR("AUDIOIO-ALSA3", "Error when preparing stream: " + string(snd_strerror(err))));
  is_prepared_rep = true;
}

void ALSA_PCM_DEVICE::start(void) {
  assert(is_triggered_rep == false);
  assert(is_open() == true);
  assert(is_prepared_rep == true);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) start");

  if (pcm_stream_rep == SND_PCM_STREAM_PLAYBACK)
    ::snd_pcm_start(audio_fd_repp);
  is_triggered_rep = true;
}

long int ALSA_PCM_DEVICE::read_samples(void* target_buffer, 
					long int samples) {
  assert(samples <= fragment_size_rep);
  return(::snd_pcm_read(audio_fd_repp, target_buffer, fragment_size_rep));
}

void ALSA_PCM_DEVICE::print_status_debug(void) {
  snd_pcm_status_t status;
  memset(&status, 0, sizeof(status));
  ::snd_pcm_status(audio_fd_repp, &status);
  if (pcm_stream_rep == SND_PCM_STREAM_PLAYBACK)
    underruns_rep += status.xruns;
  else if (pcm_stream_rep == SND_PCM_STREAM_CAPTURE)
    overruns_rep += status.xruns;
  cerr << "status:" << status.avail << "," << status.appl_ptr << "," <<
    status.xruns << "," << status.state << " ";
  print_time_stamp();
}

void ALSA_PCM_DEVICE::write_samples(void* target_buffer, long int samples) {
  if (samples == fragment_size_rep) {
    ::snd_pcm_write(audio_fd_repp, target_buffer, fragment_size_rep);
  }
  else {
    if (static_cast<size_t>(samples) < pcm_params_info_rep.min_fragment_size ||
	static_cast<size_t>(samples) > pcm_params_info_rep.max_fragment_size) {
      if (is_triggered_rep) stop();
      return; 
    }
    bool was_triggered = false;
    if (is_triggered_rep == true) { stop(); was_triggered = true; }
    close();
    buffersize(samples, samples_per_second());
    open();
    prepare();
    assert(samples <= fragment_size_rep);
    ::snd_pcm_write(audio_fd_repp, target_buffer, fragment_size_rep);
    if (was_triggered == true) start();
  }
}

long ALSA_PCM_DEVICE::position_in_samples(void) const {
  if (is_triggered_rep == false) return(0);
  snd_pcm_status_t status;
  memset(&status, 0, sizeof(status));
  ::snd_pcm_status(audio_fd_repp, &status);
  return (status.appl_ptr);
}

ALSA_PCM_DEVICE::~ALSA_PCM_DEVICE(void) { 
  if (is_open() == true) close(); 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) destruct");

  if (io_mode() != io_read) {
    if (underruns_rep != 0) {
      cerr << "(audioio-alsa3) WARNING! While writing to ALSA-pcm device ";
      cerr << "C" << card_number_rep << "D" << device_number_rep;
      cerr << ", there were " << underruns_rep << " underruns.\n";
    }
  }
  else {
    if (overruns_rep != 0) {
      cerr << "(audioio-alsa3) WARNING! While reading from ALSA-pcm device ";
      cerr << "C" << card_number_rep << "D" << device_number_rep;
      cerr << ", there were " << overruns_rep << " overruns.\n";
    }
  }
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

  case 4: 
    subdevice_number_rep = atoi(value.c_str());
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

  case 4: 
    return(kvu_numtostr(subdevice_number_rep));
  }
  return("");
}

#endif // ALSALIB_060
