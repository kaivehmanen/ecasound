// ------------------------------------------------------------------------
// audioio-alsa3.cpp: ALSA 0.9.x PCM input and output.
// Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi),
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

#include <kvutils/dbc.h>
#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>
#include <kvutils.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ALSALIB_060
#include <alsa/asoundlib.h>

#include "samplebuffer.h"
#include "audioio-device.h"
#include "audioio_alsa3.h"

#include "eca-version.h"
#include "eca-error.h"
#include "eca-debug.h"

#ifndef timersub
#define	timersub(a, b, result) \
do { \
	(result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
	(result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
	if ((result)->tv_usec < 0) { \
		--(result)->tv_sec; \
		(result)->tv_usec += 1000000; \
	} \
} while (0)
#endif

const string ALSA_PCM_DEVICE_06X::default_pcm_device_rep = "default";

ALSA_PCM_DEVICE_06X::ALSA_PCM_DEVICE_06X (int card, 
					  int device, 
					  int subdevice) 
  : AUDIO_IO_DEVICE() 
{
  // ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) construct");
  card_number_rep = card;
  device_number_rep = device;
  subdevice_number_rep = subdevice;
  is_triggered_rep = false;
  is_prepared_rep = false;
  trigger_request_rep = false;
  overruns_rep = underruns_rep = 0;
  position_in_samples_rep = 0;
  nbufs_repp = 0;
  allocate_structs();
}

ALSA_PCM_DEVICE_06X::~ALSA_PCM_DEVICE_06X(void) { 
  if (is_open() == true) close(); 

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
  deallocate_structs();
}

void ALSA_PCM_DEVICE_06X::allocate_structs(void) {
  int err = snd_pcm_hw_params_malloc(&pcm_hw_params_repp);
  DBC_CHECK(!err);

  err = snd_pcm_sw_params_malloc(&pcm_sw_params_repp);
  DBC_CHECK(!err);
}

void ALSA_PCM_DEVICE_06X::deallocate_structs(void) {
  snd_pcm_hw_params_free(pcm_hw_params_repp);
  snd_pcm_sw_params_free(pcm_sw_params_repp);
}


void ALSA_PCM_DEVICE_06X::open_device(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) open");

  // -------------------------------------------------------------------
  // Device name initialization

  string device_name = pcm_device_name();

  // -------------------------------------------------------------------
  // Open devices

  int err;
  if (io_mode() == io_read) {
    pcm_stream_rep = SND_PCM_STREAM_CAPTURE;
    err = snd_pcm_open(&audio_fd_repp, 
			 (char*)device_name.c_str(),
			 pcm_stream_rep,
			 SND_PCM_NONBLOCK);

    if (err < 0) {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-ALSA3: Unable to open ALSA--device for capture; error: " + 
			string(snd_strerror(err))));
    }
  }    
  else if (io_mode() == io_write) {
    pcm_stream_rep = SND_PCM_STREAM_PLAYBACK;
    err = snd_pcm_open(&audio_fd_repp, 
			 (char*)device_name.c_str(),
			 pcm_stream_rep,
			 SND_PCM_NONBLOCK);
    
    if (err < 0) {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-ALSA3: Unable to open ALSA-device for playback; error: " +  
			string(snd_strerror(err))));
    }
  }
  else if (io_mode() == io_readwrite) {
    throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-ALSA3: Simultaneous input/output not supported."));
  }

  // -------------------------------------------------------------------
  // enables blocking mode
  snd_pcm_nonblock(audio_fd_repp, 0);
}

void ALSA_PCM_DEVICE_06X::set_audio_format_params(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) set_audio_format_params");
  format_rep = SND_PCM_FORMAT_LAST;
  switch(sample_format()) 
    {
    case ECA_AUDIO_FORMAT::sfmt_u8:  { format_rep = SND_PCM_FORMAT_U8; break; }
    case ECA_AUDIO_FORMAT::sfmt_s8:  { format_rep = SND_PCM_FORMAT_S8; break; }
    case ECA_AUDIO_FORMAT::sfmt_s16_le:  { format_rep = SND_PCM_FORMAT_S16_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s16_be:  { format_rep = SND_PCM_FORMAT_S16_BE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s24_le:  { format_rep = SND_PCM_FORMAT_S24_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s24_be:  { format_rep = SND_PCM_FORMAT_S24_BE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s32_le:  { format_rep = SND_PCM_FORMAT_S32_LE; break; }
    case ECA_AUDIO_FORMAT::sfmt_s32_be:  { format_rep = SND_PCM_FORMAT_S32_BE; break; }
      
    default:
      {
	throw(SETUP_ERROR(SETUP_ERROR::sample_format, "AUDIOIO-ALSA3: Error when setting audio format not supported (1)"));
      }
    }
}

void ALSA_PCM_DEVICE_06X::print_pcm_info(void) {
}

void ALSA_PCM_DEVICE_06X::fill_and_set_hw_params(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) fill_and_set_hw_params");

  /* 1. create one param combination */
  int err = snd_pcm_hw_params_any(audio_fd_repp, pcm_hw_params_repp);
  if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up hwparams/any: " + string(snd_strerror(err))));
  
  /* 2. set interleaving mode */
  if (interleaved_channels() == true)
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Using interleaved stream format.");
  else
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Using noninterleaved stream format.");

  if (interleaved_channels() == true)
    err = snd_pcm_hw_params_set_access(audio_fd_repp, pcm_hw_params_repp,
					 SND_PCM_ACCESS_RW_INTERLEAVED
					 );
  else
    err = snd_pcm_hw_params_set_access(audio_fd_repp, pcm_hw_params_repp,
					 SND_PCM_ACCESS_RW_NONINTERLEAVED
					);
  if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up hwparams/access: " + string(snd_strerror(err))));

  /* 3. set sample format */
  err = snd_pcm_hw_params_set_format(audio_fd_repp, 
				     pcm_hw_params_repp, 
				     format_rep
				     );
  if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::sample_format, "AUDIOIO-ALSA3: Audio format not supported."));

  /* 4. set channel count */
  err = snd_pcm_hw_params_set_channels(audio_fd_repp, 
					 pcm_hw_params_repp, 
					 channels()
					 );
  if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::channels, "AUDIOIO-ALSA3: Channel count " +
				 kvu_numtostr(channels()) + " is out of range!"));

  /* 5. set sampling rate */
  err = snd_pcm_hw_params_set_rate_near(audio_fd_repp, 
					  pcm_hw_params_repp,
					  samples_per_second(), 
					  0);
  if (err < 0)   throw(SETUP_ERROR(SETUP_ERROR::sample_rate, "AUDIOIO-ALSA3: Sample rate " +
				   kvu_numtostr(samples_per_second()) + " is out of range!"));

  /* 6. create buffers for noninterleaved i/o */
  if (interleaved_channels() != true) {
    if (nbufs_repp == 0)
      nbufs_repp = new unsigned char* [channels()];
  }

  /* 7. sets period size (period = one fragment) */
  err = snd_pcm_hw_params_set_period_size(audio_fd_repp, 
					  pcm_hw_params_repp,
					  buffersize(), 
					  0);
  if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::buffersize, "AUDIOIO-ALSA3: buffersize " +
				 kvu_numtostr(buffersize()) + " is out of range!"));

  /* 7. sets buffer size */
  if (max_buffers() == true) {
    err = snd_pcm_hw_params_set_buffer_size_near(audio_fd_repp, 
						 pcm_hw_params_repp,
						 buffersize() * 1024);
    if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::unexpected,
				   "AUDIOIO-ALSA3: Error when setting up hwparams/btime (1): " + string(snd_strerror(err))));
  }
  else {
    err = snd_pcm_hw_params_set_buffer_size_near(audio_fd_repp, 
						 pcm_hw_params_repp,
						 3 * buffersize());
    if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::unexpected,
				   "AUDIOIO-ALSA3: Error when setting up hwparams/btime (2): " + string(snd_strerror(err))));
  }
   
  /* 8. print debug information */
  unsigned int value = snd_pcm_hw_params_get_buffer_time(pcm_hw_params_repp, 0);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) buffer time set to " + kvu_numtostr(value) + " usecs.");

  value = snd_pcm_hw_params_get_buffer_size(pcm_hw_params_repp);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) buffer time set to " + kvu_numtostr(value) + " frames.");
  
  value = snd_pcm_hw_params_get_period_time(pcm_hw_params_repp, 0);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) period time set to " + kvu_numtostr(value) + " usecs.");

  value = snd_pcm_hw_params_get_period_size(pcm_hw_params_repp, 0);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) period time set to " + kvu_numtostr(value) + " frames.");
  DBC_CHECK(value == static_cast<unsigned int>(buffersize()));

  /* 9. all set, now active hw params */
  err = snd_pcm_hw_params(audio_fd_repp, pcm_hw_params_repp);
  if (err < 0) {
    throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up hwparams: " + string(snd_strerror(err))));
  }
}

void ALSA_PCM_DEVICE_06X::fill_and_set_sw_params(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) fill_and_set_sw_params");

  /* 1. get current params */
  snd_pcm_sw_params_current(audio_fd_repp, pcm_sw_params_repp);

  /* 2. set start threshold (should be big enough so that processing 
        won't start until a explicit snd_pcm_start() is issued */
  int err = snd_pcm_sw_params_set_start_threshold(audio_fd_repp, 
						  pcm_sw_params_repp,
						  buffersize() * 1024);
  if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up pcm_sw_params/start_threshold: " + string(snd_strerror(err))));
  
  /* 3. set align to one frame (like the OSS-emulation layer) */
  err = snd_pcm_sw_params_set_xfer_align(audio_fd_repp,
					 pcm_sw_params_repp,
					 1);
  if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up pcm_sw_params_repp/xfer_align: " + string(snd_strerror(err))));
  
  /* 4. activate params */
  err = snd_pcm_sw_params(audio_fd_repp, pcm_sw_params_repp);
  if (err < 0) throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up pcm_sw_params_repp: " + string(snd_strerror(err))));
}

void ALSA_PCM_DEVICE_06X::open(void) throw(AUDIO_IO::SETUP_ERROR&) {
  // --
  DBC_REQUIRE(is_open() == false);
  DBC_REQUIRE(is_triggered_rep == false);
  // --

  open_device();
  set_audio_format_params();
  fill_and_set_hw_params();
  print_pcm_info();
  fill_and_set_sw_params();

  is_triggered_rep = false;
  is_prepared_rep = false;
  toggle_open_state(true);
}

void ALSA_PCM_DEVICE_06X::stop(void) {
  // --
  DBC_REQUIRE(is_open() == true);
  DBC_REQUIRE(is_prepared_rep == true);
  // --

  AUDIO_IO_DEVICE::stop();
  snd_pcm_drop(audio_fd_repp); /* non-blocking */
  // snd_pcm_drain(audio_fd_repp); /* blocking */
  
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) stop - " + label() + ".");
  
  is_triggered_rep = false;
  is_prepared_rep = false;
}

void ALSA_PCM_DEVICE_06X::close(void) {
  // --
  DBC_REQUIRE(is_open() == true);
  // --

  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) close - " + label() + ".");

  if (is_prepared_rep == true && is_triggered_rep == true) stop();
  snd_pcm_close(audio_fd_repp);
  toggle_open_state(false);

  // --
  DBC_ENSURE(is_triggered_rep == false);
  // --
}

void ALSA_PCM_DEVICE_06X::prepare(void) {
  // --
  DBC_REQUIRE(is_triggered_rep == false);
  DBC_REQUIRE(is_open() == true);
  DBC_REQUIRE(is_prepared_rep == false);
  // --

  AUDIO_IO_DEVICE::prepare();
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) prepare - " + label() + ".");

  int err = snd_pcm_prepare(audio_fd_repp);
  if (err < 0)
    ecadebug->msg(ECA_DEBUG::info, "(audioio-alsa3) Error when preparing stream: " + string(snd_strerror(err)));
  is_prepared_rep = true;
}

void ALSA_PCM_DEVICE_06X::start(void) {
  // --
  DBC_REQUIRE(is_triggered_rep == false);
  DBC_REQUIRE(is_open() == true);
  DBC_REQUIRE(is_prepared_rep == true);
  // --

  AUDIO_IO_DEVICE::start();
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) start - " + label() + ".");
  snd_pcm_start(audio_fd_repp);
  is_triggered_rep = true;
  position_in_samples_rep = 0;
}

long int ALSA_PCM_DEVICE_06X::read_samples(void* target_buffer, 
					   long int samples) {
  // --
  DBC_REQUIRE(samples <= buffersize());
  // --

  long int realsamples = 0;

  if (interleaved_channels() == true) {
    realsamples = snd_pcm_readi(audio_fd_repp, target_buffer,
				 buffersize());
    if (realsamples < 0) {
      if (realsamples == -EPIPE) {
	if (ignore_xruns() == true) {
	  handle_xrun_capture();
	  realsamples = snd_pcm_readi(audio_fd_repp, target_buffer,
					buffersize());
	  if (realsamples < 0) realsamples = 0;
	}
	else {
	  cerr << "(audioio-alsa3) Overrun! Stopping operation!" << endl;
	  stop();
	  close();
	}
      }
      else {
	cerr << "(audioio-alsa3) Read error! Stopping operation." << endl;
	stop();
	close();
      }
    }
  }
  else {
    unsigned char* ptr_to_channel = reinterpret_cast<unsigned char*>(target_buffer);
    for (int channel = 0; channel < channels(); channel++) {
      nbufs_repp[channel] = ptr_to_channel;
      ptr_to_channel += samples * bits() / 8;
    }
    realsamples = snd_pcm_readn(audio_fd_repp, reinterpret_cast<void**>(target_buffer), buffersize());
    if (realsamples < 0) {
      if (realsamples == -EPIPE) {
	if (ignore_xruns() == true) {
	  handle_xrun_capture();
	  realsamples = snd_pcm_readn(audio_fd_repp, reinterpret_cast<void**>(target_buffer), buffersize());
	  if (realsamples < 0) realsamples = 0;
	}
	else {
	  cerr << "(audioio-alsa3) Overrun! Stopping operation!" << endl;
	  stop();
	  close();
	}
      }
      else {
	cerr << "(audioio-alsa3) Read error! Stopping operation." << endl;
	stop();
	close();
      }
    }
  }
  position_in_samples_rep += realsamples;
  return(realsamples);
}

void ALSA_PCM_DEVICE_06X::handle_xrun_capture(void) {
  snd_pcm_status_t *status;
  snd_pcm_status_alloca(&status);

  int res = snd_pcm_status(audio_fd_repp, status);
  if (res >= 0) {
    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
 
      struct timeval now, diff, tstamp;
      gettimeofday(&now, 0);
      snd_pcm_status_get_trigger_tstamp(status, &tstamp);
      timersub(&now, &tstamp, &diff);

      std::cerr << "(audioio-alsa3) warning! playback overrun - samples lost! " 
		<< " Break was at least " << kvu_numtostr(diff.tv_sec *
							  1000 +
							  diff.tv_usec /
							  1000.0) 
		<< " ms long." << endl;

      overruns_rep++;
      stop();
      prepare();
      start();
    }
    else {
      std::cerr << "(audioio-alsa3) unknown device state!" << endl;
    }
  }
  else {
    ecadebug->msg(ECA_DEBUG::info, "(audioio-alsa3) snd_pcm_status() failed!");
  }
}

void ALSA_PCM_DEVICE_06X::write_samples(void* target_buffer, long int samples) {
  if (trigger_request_rep == true) {
    trigger_request_rep = false;
    start();
  }

  if (interleaved_channels() == true) {
    long int count = snd_pcm_writei(audio_fd_repp, target_buffer, samples);
    if (count < 0) {
      if (count == -EPIPE) {
	if (ignore_xruns() == true) {
	  handle_xrun_playback();
	  if (snd_pcm_writei(audio_fd_repp, target_buffer, samples) < 0) 
	    cerr << "(audioio-alsa3) Xrun handling failed!" << endl;
	  trigger_request_rep = true;
	}
	else {
	  cerr << "(audioio-alsa3) Underrun! Stopping operation!" << endl;
	  stop();
	  close();
	}
      }
      else {
	std::cerr << "(audioio-alsa3) Write error! Stopping operation (" << count << ")." << std::endl;
	stop();
	close();
      }
    }
  }
  else {
    unsigned char* ptr_to_channel = reinterpret_cast<unsigned char*>(target_buffer);
    for (int channel = 0; channel < channels(); channel++) {
      nbufs_repp[channel] = ptr_to_channel;
      // std::cerr << "Pointer to channel " << channel << ": " << reinterpret_cast<void*>(nbufs_repp[channel]) << std::endl;
      ptr_to_channel += samples * bits() / 8;
      // std::cerr << "Advancing pointer count by " << samples * bits() / 8 << " to " << reinterpret_cast<void*>(ptr_to_channel) << std::endl;
    }
    long int count =  snd_pcm_writen(audio_fd_repp,
				       reinterpret_cast<void**>(nbufs_repp), 
				       samples);
    if (count < 0) {
      if (count == -EPIPE) {
	if (ignore_xruns() == true) {
	  handle_xrun_playback();
	  snd_pcm_writen(audio_fd_repp,
			   reinterpret_cast<void**>(nbufs_repp),
			   samples);
	  trigger_request_rep = true;
	}
	else {
	  cerr << "(audioio-alsa3) Underrun! Stopping operation!" << endl;
	  stop();
	  close();
	}
      }
      else {
	cerr << "(audioio-alsa3) Write  error! Stopping operation." << endl;
	stop();
	close();
      }
    }
  }

  position_in_samples_rep += samples;
}

void ALSA_PCM_DEVICE_06X::handle_xrun_playback(void) {
  snd_pcm_status_t *status;
  snd_pcm_status_alloca(&status);

  int res = snd_pcm_status(audio_fd_repp, status);
  if (res >= 0) {
    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {

      struct timeval now, diff, tstamp;
      gettimeofday(&now, 0);
      snd_pcm_status_get_trigger_tstamp(status, &tstamp);
      timersub(&now, &tstamp, &diff);
      
      std::cerr << "(audioio-alsa3) warning! playback underrun - samples lost! " 
		<< " Break was at least " << kvu_numtostr(diff.tv_sec *
							  1000 +
							  diff.tv_usec /
							  1000.0) 
		<< " ms long." << endl;
      underruns_rep++;
      stop();
      prepare();
      trigger_request_rep = true;
    }
    else {
      std::cerr << "(audioio-alsa3) unknown device state!" << endl;
    }
  }
  else {
    ecadebug->msg(ECA_DEBUG::info, "(audioio-alsa3) snd_pcm_status() failed!");
  }
}

SAMPLE_SPECS::sample_pos_t ALSA_PCM_DEVICE_06X::position_in_samples(void) const {
  if (is_triggered_rep == false) return(0);
  snd_pcm_sframes_t delay = 0;
  if (snd_pcm_delay(audio_fd_repp, &delay) != 0) 
    delay = 0;
  
  if (io_mode() == io_read)
    return(position_in_samples_rep + delay);
  
  return(position_in_samples_rep - delay);
}

void ALSA_PCM_DEVICE_06X::set_parameter(int param, 
				     string value) {
  switch (param) {
  case 1: 
    label(value);
    if (label().find("alsaplugin") != string::npos) {
      using_plugin_rep = true;
    }
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

  if (using_plugin_rep)
    pcm_device_name_rep = 
      string("plughw:") + 
      kvu_numtostr(card_number_rep) +
      "," +
      kvu_numtostr(device_number_rep) +
      "," +
      kvu_numtostr(subdevice_number_rep);
  else
    pcm_device_name_rep = 
      string("hw:") + 
      kvu_numtostr(card_number_rep) +
      "," +
      kvu_numtostr(device_number_rep) +
      "," +
      kvu_numtostr(subdevice_number_rep);
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

void ALSA_PCM_DEVICE_06X::set_pcm_device_name(const string& n) { 
  if (n.size() > 0)
    pcm_device_name_rep = n; 
  else
    pcm_device_name_rep = default_pcm_device_rep;
}

#endif /* ALSALIB_060 */
