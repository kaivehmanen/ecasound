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
#include <errno.h>

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

ALSA_PCM_DEVICE_06X::ALSA_PCM_DEVICE_06X (int card, 
				  int device, 
				  int subdevice) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) construct");
  card_number_rep = card;
  device_number_rep = device;
  subdevice_number_rep = subdevice;
  is_triggered_rep = false;
  is_prepared_rep = false;
  overruns_rep = underruns_rep = 0;
  position_in_samples_rep = 0;
  nbufs_repp = 0;
}

void ALSA_PCM_DEVICE_06X::open(void) throw(SETUP_ERROR&) {
  assert(is_open() == false);
  assert(is_triggered_rep == false);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) open");

  // -------------------------------------------------------------------
  // Channel initialization

  if (label().find("alsaplugin") != string::npos) {
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
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-ALSA3: Unable to open ALSA-plugin-device for capture; error: " + 
			    string(snd_strerror(err))));
      else
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-ALSA3: Unable to open ALSA-device for capture; error: " + 
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
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-ALSA3: Unable to open ALSA-plugin-device for playback; error: " +  
			    string(snd_strerror(err))));
      else
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-ALSA3: Unable to open ALSA-device for playback; error: " +  
			    string(snd_strerror(err))));
    }
  }
  else if (io_mode() == io_readwrite) {
    throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-ALSA3: Simultaneous input/output not supported."));
  }

  // -------------------------------------------------------------------
  // Sets blocking mode
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

  snd_pcm_format_t pf;
  ::memset(&pf, 0, sizeof(pf));

  if ((pcm_params_info_rep.flags & SND_PCM_INFO_INTERLEAVED) == SND_PCM_INFO_INTERLEAVED) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Device supports interleaved stream format.");
  }
  else {
    if (interleaved_channels() == true)
      toggle_interleaved_channels(false);
  }

  if ((pcm_params_info_rep.flags & SND_PCM_INFO_NONINTERLEAVED) == SND_PCM_INFO_NONINTERLEAVED) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Device supports noninterleaved stream format.");
  }
  else {
    if (interleaved_channels() != true)
      toggle_interleaved_channels(true);
  }
  
  if (interleaved_channels() == true)
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Using interleaved stream format.");
  else
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Using noninterleaved stream format.");
  
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
	throw(SETUP_ERROR(SETUP_ERROR::sample_format, "AUDIOIO-ALSA3: Error when setting audio format not supported (1)"));
      }
    }

  unsigned int format_mask = (1 << format);
  if ((pcm_params_info_rep.formats & format_mask) != format_mask) 
    throw(SETUP_ERROR(SETUP_ERROR::sample_format, "AUDIOIO-ALSA3: Selected sample format not supported by the device!"));
  pf.sfmt = format;

  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Supported sample rates; min: " + kvu_numtostr(pcm_params_info_rep.min_rate) + ", max: " + kvu_numtostr(pcm_params_info_rep.max_rate) + ".");
  if (static_cast<unsigned int>(samples_per_second()) < pcm_params_info_rep.min_rate ||
      static_cast<unsigned int>(samples_per_second()) > pcm_params_info_rep.max_rate)
    throw(SETUP_ERROR(SETUP_ERROR::sample_rate, "AUDIOIO-ALSA3: Sample rate " +
			kvu_numtostr(samples_per_second()) + " is out of range!"));
  pf.rate = samples_per_second();

  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Supported channel counts; min: " + kvu_numtostr(pcm_params_info_rep.min_channels) + ", max: " + kvu_numtostr(pcm_params_info_rep.max_channels) + ".");
  if (static_cast<unsigned int>(channels()) < pcm_params_info_rep.min_channels ||
      static_cast<unsigned int>(channels()) > pcm_params_info_rep.max_channels)
    throw(SETUP_ERROR(SETUP_ERROR::channels, "AUDIOIO-ALSA3: Channel count " +
			kvu_numtostr(channels()) + " is out of range!"));
  pf.channels = channels();
  
  if (interleaved_channels() != true) {
    if (nbufs_repp == 0)
      nbufs_repp = new unsigned char* [channels()];
  }

  ::memcpy(&params.format, &pf, sizeof(pf));

  params.start_mode = SND_PCM_START_EXPLICIT;
  if (interleaved_channels() == true) 
    params.xfer_mode = SND_PCM_XFER_INTERLEAVED;
  else
    params.xfer_mode = SND_PCM_XFER_NONINTERLEAVED;
  params.xrun_mode = SND_PCM_XRUN_ASAP; /* FIXME: _FRAGMENT? */
  params.ready_mode = SND_PCM_READY_ASAP; /* FIXME: _FRAGMENT? */
//    params.xrun_act = SND_PCM_XRUN_ACT_RESTART;

  // -------------------------------------------------------------------
  // Fetch stream info (2nd time, now getting frame size info)
  pcm_params_info_rep.req.format.sfmt = format;
  pcm_params_info_rep.req.format.channels = channels();
  pcm_params_info_rep.req_mask = SND_PCM_PARAMS_SFMT | 
                                 SND_PCM_PARAMS_CHANNELS | 
                                 SND_PCM_PARAMS_FRAGMENT_SIZE;
  err = ::snd_pcm_params_info(audio_fd_repp, &pcm_params_info_rep);
  if (err < 0)
    throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when querying stream info: " + string(snd_strerror(err))));

  // -------------------------------------------------------------------
  // Set fragment size.

  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Supported fragment sizes; min: " + kvu_numtostr(pcm_params_info_rep.min_fragment_size) + ", max: " + kvu_numtostr(pcm_params_info_rep.max_fragment_size) + ".");
  if (using_plugin_rep != true && 
      (static_cast<unsigned int>(buffersize()) < pcm_params_info_rep.min_fragment_size ||
       static_cast<unsigned int>(buffersize()) > pcm_params_info_rep.max_fragment_size))
    throw(SETUP_ERROR(SETUP_ERROR::buffersize, "AUDIOIO-ALSA3: buffersize " +
			kvu_numtostr(buffersize()) + " is out of range!"));

  // <--
  params.buffer_size = pcm_params_info_rep.buffer_size;
  params.frag_size = buffersize();
  params.xfer_align = params.frag_size;
//    params.xrun_max = ~0U;
  params.xfer_min = params.frag_size;
  params.avail_min = params.frag_size;

  // -------------------------------------------------------------------
  // Stream params
  err = ::snd_pcm_params(audio_fd_repp, &params);
  if (err < 0)
    throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up stream params: " + string(snd_strerror(err))));

  // -------------------------------------------------------------------
  // Fetch the current pcm-setup and print out some debug info
  struct snd_pcm_setup setup;
  setup.xfer_mode = params.xfer_mode;
  ::snd_pcm_setup(audio_fd_repp, &setup);

  fragment_size_rep = setup.frag_size;
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Fragment size: " +
		kvu_numtostr(setup.frag_size) + ", xfermin: " +
		kvu_numtostr(setup.xfer_min) + ", current: " +
		kvu_numtostr(setup.frags) + ".");

  is_triggered_rep = false;
  is_prepared_rep = false;
  toggle_open_state(true);
}

void ALSA_PCM_DEVICE_06X::stop(void) {
  assert(is_triggered_rep == true);
  assert(is_open() == true);
  assert(is_prepared_rep == true);

  AUDIO_IO_DEVICE::stop();
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) stop");

  snd_pcm_status_t status;
  ::memset(&status, 0, sizeof(status));
  ::snd_pcm_status(audio_fd_repp, &status);
  if (pcm_stream_rep == SND_PCM_STREAM_PLAYBACK &&
      status.state == SND_PCM_STATE_XRUN)
    underruns_rep++;
  else if (pcm_stream_rep == SND_PCM_STREAM_CAPTURE &&
	   status.state == SND_PCM_STATE_XRUN)
    overruns_rep++;

  ::snd_pcm_drop(audio_fd_repp);
  
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Audio device \"" + label() + "\" disabled.");
  
  is_triggered_rep = false;
  is_prepared_rep = false;
}

void ALSA_PCM_DEVICE_06X::close(void) {
  assert(is_open() == true);

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) close");

  if (is_triggered_rep == true) stop();
  ::snd_pcm_close(audio_fd_repp);
  toggle_open_state(false);

  assert(is_triggered_rep == false);
}

void ALSA_PCM_DEVICE_06X::prepare(void) {
  assert(is_triggered_rep == false);
  assert(is_open() == true);
  assert(is_prepared_rep == false);

  AUDIO_IO_DEVICE::prepare();
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) prepare");

  int err = ::snd_pcm_prepare(audio_fd_repp);
  if (err < 0)
    ecadebug->msg(ECA_DEBUG::info, "AUDIOIO-ALSA3: Error when preparing stream: " + string(snd_strerror(err)));
  is_prepared_rep = true;
}

void ALSA_PCM_DEVICE_06X::start(void) {
  assert(is_triggered_rep == false);
  assert(is_open() == true);
  assert(is_prepared_rep == true);

  AUDIO_IO_DEVICE::start();
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) start");
  ::snd_pcm_start(audio_fd_repp);
  is_triggered_rep = true;
  position_in_samples_rep = 0;
}

long int ALSA_PCM_DEVICE_06X::read_samples(void* target_buffer, 
					   long int samples) {
  assert(samples <= fragment_size_rep);
  long int realsamples = 0;
  if (interleaved_channels() == true)
    realsamples = ::snd_pcm_read(audio_fd_repp, target_buffer, fragment_size_rep);
  else {
    unsigned char* ptr_to_channel = reinterpret_cast<unsigned char*>(target_buffer);
    for (int channel = 0; channel < channels(); channel++) {
      nbufs_repp[channel] = ptr_to_channel;
      ptr_to_channel += samples * frame_size();
    }
    realsamples = ::snd_pcm_readn(audio_fd_repp, reinterpret_cast<void**>(target_buffer), fragment_size_rep);
  }
  position_in_samples_rep += realsamples;
  return(realsamples);
}

void ALSA_PCM_DEVICE_06X::print_status_debug(void) {
  snd_pcm_status_t status;
  memset(&status, 0, sizeof(status));
  ::snd_pcm_status(audio_fd_repp, &status);
  if (pcm_stream_rep == SND_PCM_STREAM_PLAYBACK &&
      status.state == SND_PCM_STATE_XRUN)
    underruns_rep++;
  else if (pcm_stream_rep == SND_PCM_STREAM_CAPTURE &&
	   status.state == SND_PCM_STATE_XRUN)
    overruns_rep++;
  cerr << "status; avail:" << status.avail << ", state" << status.state << "." << endl;
  print_time_stamp();
}

void ALSA_PCM_DEVICE_06X::write_samples(void* target_buffer, long int samples) {
  bool trigger = false;
  if (samples != fragment_size_rep) {
    /**
     * We need to adjust the fragment size, because ALSA requires 
     * i/o to happen exactly one fragment at a time.
     */
    if (static_cast<size_t>(samples) < pcm_params_info_rep.min_fragment_size)
      samples = pcm_params_info_rep.min_fragment_size;
    if (static_cast<size_t>(samples) > pcm_params_info_rep.max_fragment_size)
      samples = pcm_params_info_rep.max_fragment_size;

    if (is_triggered_rep == true) { stop(); trigger = true; }
    close();
    buffersize(samples, samples_per_second());
    open();
    prepare();
    assert(samples <= fragment_size_rep);
  }
  if (interleaved_channels() == true) {
    if (::snd_pcm_write(audio_fd_repp, target_buffer,
			fragment_size_rep) == -EPIPE) {
      underruns_rep++;
      ecadebug->msg(ECA_DEBUG::info, "(audioio-alsa3) underrun! stopping device");
      stop();
    }
  }
  else {
    unsigned char* ptr_to_channel = reinterpret_cast<unsigned char*>(target_buffer);
    for (int channel = 0; channel < channels(); channel++) {
      nbufs_repp[channel] = ptr_to_channel;
//        cerr << "Pointer to channel " << channel << ": " << reinterpret_cast<void*>(nbufs_repp[channel]) << endl;
      ptr_to_channel += samples * frame_size();
//        cerr << "Advancing pointer count to " << reinterpret_cast<void*>(ptr_to_channel) << endl;
    }
    if (::snd_pcm_writen(audio_fd_repp,
		     reinterpret_cast<void**>(nbufs_repp),
		     fragment_size_rep) == -EPIPE) {
      underruns_rep++;
      ecadebug->msg(ECA_DEBUG::info, "(audioio-alsa3) underrun! stopping device");
      stop();
    }
  }
  if (trigger == true) start();
  position_in_samples_rep += samples;
}

long ALSA_PCM_DEVICE_06X::position_in_samples(void) const {
  if (is_triggered_rep == false) return(0);
  ssize_t delay = 0;
  if (::snd_pcm_delay(audio_fd_repp, &delay) != 0) 
    delay = 0;
  
  if (io_mode() == io_read)
    return(position_in_samples_rep + delay);
  
  return(position_in_samples_rep - delay);
}

ALSA_PCM_DEVICE_06X::~ALSA_PCM_DEVICE_06X(void) { 
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
  if (nbufs_repp != 0)
    delete nbufs_repp;
}

void ALSA_PCM_DEVICE_06X::set_parameter(int param, 
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

string ALSA_PCM_DEVICE_06X::get_parameter(int param) const {
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
