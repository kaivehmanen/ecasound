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
  ::memset(&pcm_hw_info_rep, 0, sizeof(pcm_hw_info_rep));
  ::memset(&pcm_info_rep, 0, sizeof(pcm_info_rep));
  ::memset(&pcm_hw_params_rep, 0, sizeof(pcm_hw_params_rep));
}

void ALSA_PCM_DEVICE_06X::open_device(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) open");

  // -------------------------------------------------------------------
  // Channel initialization

  if (label().find("alsaplugin") != string::npos) {
    using_plugin_rep = true;
  }

  // -------------------------------------------------------------------
  // Open devices

  int err;
  if (io_mode() == io_read) {
    pcm_stream_rep = SND_PCM_STREAM_CAPTURE;
    if (using_plugin_rep)
      err = ::snd_pcm_open(&audio_fd_repp, 
			   (string("plug:") + 
			   kvu_numtostr(card_number_rep) +
			   "," +
			   kvu_numtostr(device_number_rep) +
			   "," +
			   kvu_numtostr(subdevice_number_rep)).c_str(),
			   pcm_stream_rep,
			   SND_PCM_NONBLOCK);
    else
      err = ::snd_pcm_open(&audio_fd_repp, 
			   (string("hw:") + 
			   kvu_numtostr(card_number_rep) +
			   "," +
			   kvu_numtostr(device_number_rep) +
			   "," +
			   kvu_numtostr(subdevice_number_rep)).c_str(),
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
      err = ::snd_pcm_open(&audio_fd_repp, 
			   (string("plug:") + 
			   kvu_numtostr(card_number_rep) +
			   "," +
			   kvu_numtostr(device_number_rep) +
			   "," +
			   kvu_numtostr(subdevice_number_rep)).c_str(),
			   pcm_stream_rep,
			   SND_PCM_NONBLOCK);

    else
      err = ::snd_pcm_open(&audio_fd_repp, 
			   (string("hw:") + 
			   kvu_numtostr(card_number_rep) +
			   "," +
			   kvu_numtostr(device_number_rep) +
			   "," +
			   kvu_numtostr(subdevice_number_rep)).c_str(),
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
}

void ALSA_PCM_DEVICE_06X::get_pcm_info(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) get_pcm_info");
  pcm_info_rep.stream = pcm_stream_rep;
  ::snd_pcm_info(audio_fd_repp, &pcm_info_rep);
}

void ALSA_PCM_DEVICE_06X::get_pcm_hw_info(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) get_pcm_hw_info");
//    ::snd_pcm_hw_info_all(&pcm_hw_info_rep);
//    ::snd_pcm_hw_info(audio_fd_repp, &pcm_hw_info_rep);
  return;

  snd_pcm_strategy_t *strategy;
  snd_pcm_strategy_simple_choices_list_t formats[2] = {
    {
      value: SND_PCM_FORMAT_S16_LE,
      badness: 0,
    },
    {
      value: SND_PCM_FORMAT_S8,
      badness: 4,
    }
  };

  pcm_hw_info_rep.channels_min = 2;
  pcm_hw_info_rep.channels_max = 2;
  pcm_hw_info_rep.fragments_min = 2;

  int err = snd_pcm_strategy_simple(&strategy);
  assert(err >= 0);
  err = snd_pcm_strategy_simple_near(strategy, SND_PCM_HW_PARAM_RATE,
				     44050, 1);
  assert(err >= 0);
  err = snd_pcm_strategy_simple_choices(strategy, SND_PCM_HW_PARAM_FORMAT,
					2, formats);
  assert(err >= 0);
  err = snd_pcm_strategy_simple_near(strategy, SND_PCM_HW_PARAM_FRAGMENT_SIZE,
				     10000, 10);
  assert(err >= 0);
  err = snd_pcm_strategy_simple_near(strategy, SND_PCM_HW_PARAM_BUFFER_SIZE,
				     40000, 10);
  assert(err >= 0);
  err = snd_pcm_hw_info_strategy(audio_fd_repp, &pcm_hw_info_rep, strategy, 0, 1000000);
  printf("snd_pcm_hw_info_strategy returns %d\n", err);
  snd_pcm_dump_hw_info(&pcm_hw_info_rep, stdout);
  snd_pcm_strategy_free(strategy);

//    cerr << "-->" << endl;
//    snd_pcm_dump_hw_info(&pcm_hw_info_rep, stderr);
//    ::snd_pcm_hw_info_rulesv(audio_fd_repp, 
//  			   &pcm_hw_info_rep,
//  			   &pcm_hw_params_rep,
//  			   SND_PCM_RULE_REL_GE | SND_PCM_HW_PARAM_FORMAT,
//  			   SND_PCM_RULE_REL_GE | SND_PCM_HW_PARAM_FRAGMENT_SIZE,
//  			   SND_PCM_RULE_REL_GE | SND_PCM_HW_PARAM_FRAGMENTS,
//  			   SND_PCM_RULE_REL_GE | SND_PCM_HW_PARAM_CHANNELS,
//  			   SND_PCM_RULE_REL_GE | SND_PCM_HW_PARAM_RATE,
//  			   -1);

}

void ALSA_PCM_DEVICE_06X::set_audio_format_params(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) set_audio_format_params");
  format_rep = 0;
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
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) print_pcm_info");
  get_pcm_hw_info();
  if ((pcm_hw_info_rep.info & SND_PCM_INFO_INTERLEAVED) == SND_PCM_INFO_INTERLEAVED) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Device supports interleaved stream format.");
  }
  if ((pcm_hw_info_rep.info & SND_PCM_INFO_NONINTERLEAVED) == SND_PCM_INFO_NONINTERLEAVED) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Device supports noninterleaved stream format.");
  }
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Supported sample rates; min: " + kvu_numtostr(pcm_hw_info_rep.rate_min) + ", max: " + kvu_numtostr(pcm_hw_info_rep.rate_max) + ".");
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Supported channel counts; min: " + kvu_numtostr(pcm_hw_info_rep.channels_min) + ", max: " + kvu_numtostr(pcm_hw_info_rep.channels_max) + ".");
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Supported fragment sizes; min: " + kvu_numtostr(pcm_hw_info_rep.fragment_size_min) + ", max: " + kvu_numtostr(pcm_hw_info_rep.fragment_size_max) + ".");
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Fragment size: " +
  		kvu_numtostr(pcm_hw_params_rep.fragment_size) + ", fragments: " +
  		kvu_numtostr(pcm_hw_params_rep.fragments) + ".");
}

void ALSA_PCM_DEVICE_06X::fill_and_set_hw_params(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) fill_and_set_hw_params");

  if (interleaved_channels() == true)
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Using interleaved stream format.");
  else
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-alsa3) Using noninterleaved stream format.");
  
//    if (static_cast<unsigned int>(samples_per_second()) < pcm_hw_info_rep.rate_min ||
//        static_cast<unsigned int>(samples_per_second()) > pcm_hw_info_rep.rate_max)
//      throw(SETUP_ERROR(SETUP_ERROR::sample_rate, "AUDIOIO-ALSA3: Sample rate " +
//  		      kvu_numtostr(samples_per_second()) + " is out of range!"));

//    if (static_cast<unsigned int>(channels()) < pcm_hw_info_rep.channels_min ||
//        static_cast<unsigned int>(channels()) > pcm_hw_info_rep.channels_max)
//      throw(SETUP_ERROR(SETUP_ERROR::channels, "AUDIOIO-ALSA3: Channel count " +
//  			kvu_numtostr(channels()) + " is out of range!"));

  pcm_hw_params_rep.format = format_rep;
  pcm_hw_params_rep.rate = samples_per_second();
  pcm_hw_params_rep.channels = channels();
  if (interleaved_channels() != true) {
    if (nbufs_repp == 0)
      nbufs_repp = new unsigned char* [channels()];
  }

  if (interleaved_channels() == true)
    pcm_hw_params_rep.access = SND_PCM_ACCESS_RW_INTERLEAVED;
  else
    pcm_hw_params_rep.access = SND_PCM_ACCESS_RW_NONINTERLEAVED;
  pcm_hw_params_rep.subformat = 0;

//    if (using_plugin_rep != true && 
//        (static_cast<unsigned int>(buffersize()) < pcm_hw_info_rep.fragment_size_min ||
//         static_cast<unsigned int>(buffersize()) > pcm_hw_info_rep.fragment_size_max))
//      throw(SETUP_ERROR(SETUP_ERROR::buffersize, "AUDIOIO-ALSA3: buffersize " +
//  			kvu_numtostr(buffersize()) + " is out of range!"));

  pcm_hw_params_rep.fragment_size = buffersize();
  pcm_hw_params_rep.fragments = 16 * 1024 / buffersize(); /* pcm_hw_info_rep.fragment_size_max */
  pcm_hw_params_rep.fail_mask = 0;
  
  int err = ::snd_pcm_hw_params(audio_fd_repp, &pcm_hw_params_rep);
//    int err = ::snd_pcm_hw_params_rulesv(audio_fd_repp, &pcm_hw_params_rep,
//    				       SND_PCM_RULE_REL_EQ | SND_PCM_HW_PARAM_FRAGMENT_SIZE,
//      			       SND_PCM_RULE_REL_LT | SND_PCM_HW_PARAM_FRAGMENTS,
//      				       -1);
  fragment_size_rep = pcm_hw_params_rep.fragment_size;
  if (err < 0) {
    snd_pcm_dump_hw_params_fail(&pcm_hw_params_rep, stderr);
    cerr << "Fail mask: " << pcm_hw_params_rep.fail_mask << "." << endl;
    throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up hwparams: " + string(snd_strerror(err))));
  }
}

void ALSA_PCM_DEVICE_06X::fill_and_set_sw_params(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-alsa3) fill_and_set_sw_params");
  snd_pcm_sw_params_t swparams;
  ::memset(&swparams, 0, sizeof(swparams));
  swparams.start_mode = SND_PCM_START_EXPLICIT; /* FIXME: SND_PCM_START_EXPLICIT */
  swparams.ready_mode = SND_PCM_READY_FRAGMENT; /* FIXME: _ASAP? */
  swparams.xrun_mode = SND_PCM_XRUN_FRAGMENT;
  swparams.xfer_align = pcm_hw_params_rep.fragment_size;
  swparams.avail_min = pcm_hw_params_rep.fragment_size;
  swparams.xfer_min = pcm_hw_params_rep.fragment_size;
  swparams.time = 0;
  int err = ::snd_pcm_sw_params(audio_fd_repp, &swparams);
  if (err < 0) {
    cerr << "Fail mask: " << swparams.fail_mask << "." << endl;
    throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-ALSA3: Error when setting up swparams: " + string(snd_strerror(err))));
  }
}

void ALSA_PCM_DEVICE_06X::open(void) throw(SETUP_ERROR&) {
  assert(is_open() == false);
  assert(is_triggered_rep == false);

  open_device();
  get_pcm_info();
  get_pcm_hw_info();
  set_audio_format_params();
  print_pcm_info();
  fill_and_set_hw_params();
  fill_and_set_sw_params();
  print_pcm_info();

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
//    if (pcm_stream_rep == SND_PCM_STREAM_PLAYBACK &&
//        status.state == SND_PCM_STATE_XRUN)
//      underruns_rep++;
//    else if (pcm_stream_rep == SND_PCM_STREAM_CAPTURE &&
//  	   status.state == SND_PCM_STATE_XRUN)
//      overruns_rep++;

  ::snd_pcm_drop(audio_fd_repp); // non-blocking 
//    ::snd_pcm_drain(audio_fd_repp); // blocking
  
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
// ---> FIXME: should we care about this?
//    bool trigger = false;
  if (samples > fragment_size_rep) {
    /**
     * We need to adjust the fragment size, because ALSA requires 
     * i/o to happen exactly one fragment at a time.
     */
    cerr << "Aborting! Variable length output buffers not support with ALSA."; 
    cerr << "Try ALSA's /dev/dsp emulation." << endl;
    exit(-1);
    //    if (samples < 64) /* pcm_hw_info_rep.fragment_size_min */
    //      samples = 64;
    //    if (samples > 16 * 1024)
    //      samples = 16 * 1024; /* pcm_hw_info_rep.fragment_size_max */
    //      samples *= 2;
    //      if (is_triggered_rep == true) { stop(); trigger = true; }
    //      buffersize(samples, samples_per_second());
    //      fill_and_set_hw_params();
    //      prepare();
    //      assert(samples <= fragment_size_rep);
  }
// <---
  if (interleaved_channels() == true) {
    if (::snd_pcm_write(audio_fd_repp, target_buffer,
			fragment_size_rep) == -EPIPE) {
      underruns_rep++;
      ecadebug->msg(ECA_DEBUG::info, "(audioio-alsa3) underrun! stopping device");
      stop(); 
      close();
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
      close();
    }
  }
//    if (trigger == true) start();
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
