// ------------------------------------------------------------------------
// eca-alsa-dyn.cpp: Dynamic loading of ALSA-support.
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

#include <dlfcn.h>

#include "eca-debug.h"
#include "eca-error.h"
#include "eca-alsa-dyn.h"

void eca_alsa_load_dl_snd_pcm(void);
void eca_alsa_load_dl_snd_pcm_loopback(void);
void eca_alsa_load_dl_snd_rawmidi(void);

static void *eca_alsa_dynlib_handle;
static int eca_alsa_client_count = 0;
static bool eca_alsa_dynlib_initialized = false;

// ---------------------------------------------------------------------

// ---
// ALSA-lib pcm routines (snd_pcm_*)
// ---

int (*dl_snd_pcm_open)(snd_pcm_t **,int,int,int);
int (*dl_snd_pcm_close)(snd_pcm_t *handle);
const char* (*dl_snd_strerror)(int);
ssize_t (*dl_snd_pcm_write)(snd_pcm_t *handle, const void *buffer, size_t size);
ssize_t (*dl_snd_pcm_read)(snd_pcm_t *handle, void *buffer, size_t size);
#ifdef ALSALIB_032
int (*dl_snd_pcm_block_mode)(snd_pcm_t *handle, int enable);
int (*dl_snd_pcm_info)(snd_pcm_t *handle, snd_pcm_info_t * info);
int (*dl_snd_pcm_playback_info)(snd_pcm_t *handle, snd_pcm_playback_info_t * info);
int (*dl_snd_pcm_playback_format)(snd_pcm_t *handle, snd_pcm_format_t * format);
int (*dl_snd_pcm_playback_params)(snd_pcm_t *handle, snd_pcm_playback_params_t * params);
int (*dl_snd_pcm_playback_status)(snd_pcm_t *handle, snd_pcm_playback_status_t * status);
int (*dl_snd_pcm_flush_capture)(snd_pcm_t *handle);
int (*dl_snd_pcm_drain_playback)(snd_pcm_t *handle);
int (*dl_snd_pcm_flush_playback)(snd_pcm_t *handle);
int (*dl_snd_pcm_playback_pause)(snd_pcm_t *handle, int enable);
int (*dl_snd_pcm_playback_time)(snd_pcm_t *handle, int enable);
int (*dl_snd_pcm_capture_info)(snd_pcm_t *handle, snd_pcm_capture_info_t * info);
int (*dl_snd_pcm_capture_format)(snd_pcm_t *handle, snd_pcm_format_t * format);
int (*dl_snd_pcm_capture_params)(snd_pcm_t *handle, snd_pcm_capture_params_t * params);
int (*dl_snd_pcm_capture_status)(snd_pcm_t *handle, snd_pcm_capture_status_t * status);
int (*dl_snd_pcm_capture_time)(snd_pcm_t *handle, int enable);
#else 
int (*dl_snd_pcm_open_subdevice)(snd_pcm_t **,int,int,int,int);
int (*dl_snd_pcm_nonblock_mode)(snd_pcm_t *handle, int enable);
int (*dl_snd_pcm_info)(snd_pcm_t *handle, snd_pcm_info_t * info);
int (*dl_snd_pcm_channel_info)(snd_pcm_t *handle, snd_pcm_channel_info_t * info);
int (*dl_snd_pcm_channel_params)(snd_pcm_t *handle, snd_pcm_channel_params_t * params);
int (*dl_snd_pcm_channel_status)(snd_pcm_t *handle, snd_pcm_channel_status_t * status);
int (*dl_snd_pcm_channel_setup)(snd_pcm_t *handle, snd_pcm_channel_setup_t * setup);
int (*dl_snd_pcm_channel_prepare)(snd_pcm_t *handle, int);
int (*dl_snd_pcm_channel_go)(snd_pcm_t *handle, int channel);
int (*dl_snd_pcm_sync_go)(snd_pcm_t *handle, snd_pcm_sync_t *sync);
int (*dl_snd_pcm_playback_drain)(snd_pcm_t *handle);
int (*dl_snd_pcm_channel_flush)(snd_pcm_t *handle, int channel);
int (*dl_snd_pcm_playback_pause)(snd_pcm_t *handle, int enable);
#endif

// ---
// ALSA-lib pcm loopback routines (snd_pcm_loopback_*)
// ---

int (*dl_snd_pcm_loopback_close)(snd_pcm_loopback_t *handle);
int (*dl_snd_pcm_loopback_file_descriptor)(snd_pcm_loopback_t *handle);
int (*dl_snd_pcm_loopback_block_mode)(snd_pcm_loopback_t *handle, int enable);
int (*dl_snd_pcm_loopback_stream_mode)(snd_pcm_loopback_t *handle, int mode);
int (*dl_snd_pcm_loopback_format)(snd_pcm_loopback_t *handle, snd_pcm_format_t * format);

#ifdef ALSALIB_032
int (*dl_snd_pcm_loopback_open)(snd_pcm_loopback_t **handle, int card, int device, int mode);
ssize_t (*dl_snd_pcm_loopback_read)(snd_pcm_loopback_t *handle, void *buffer, size_t size);
#else
int (*dl_snd_pcm_loopback_open)(snd_pcm_loopback_t **handle, int card, int device, int subdev, int mode);
int (*dl_snd_pcm_loopback_status)(snd_pcm_loopback_t *handle, snd_pcm_loopback_status_t * status);
ssize_t (*dl_snd_pcm_loopback_read)(snd_pcm_loopback_t *handle, snd_pcm_loopback_callbacks_t * callbacks);
#endif

// ---------------------------------------------------------------------

// ---
// ALSA-lib rawmidi routines (snd_rawmidi_*)
// ---

int (*dl_snd_rawmidi_open)(snd_rawmidi_t **handle, int card, int device, int mode);
int (*dl_snd_rawmidi_close)(snd_rawmidi_t *handle);
int (*dl_snd_rawmidi_file_descriptor)(snd_rawmidi_t *handle);
int (*dl_snd_rawmidi_block_mode)(snd_rawmidi_t *handle, int enable);
int (*dl_snd_rawmidi_info)(snd_rawmidi_t *handle, snd_rawmidi_info_t * info);
ssize_t (*dl_snd_rawmidi_write)(snd_rawmidi_t *handle, const void *buffer, size_t size);
ssize_t (*dl_snd_rawmidi_read)(snd_rawmidi_t *handle, void *buffer, size_t size);

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

void eca_alsa_load_dynamic_support(void) throw(ECA_ERROR*) {
  ++eca_alsa_client_count;

  if (eca_alsa_dynlib_initialized == true) return; 
  else eca_alsa_dynlib_initialized = true;

  ecadebug->msg("(audioio-alsa) Loading libasound shared library.");
  
  //  eca_alsa_dynlib_handle = dlopen ("libasound.so", RTLD_LAZY);
  eca_alsa_dynlib_handle = dlopen ("libasound.so", RTLD_NOW);
  if (!eca_alsa_dynlib_handle) {
    throw(new ECA_ERROR("AUDIOIO-ALSA", "Unable to load asound.so dynamic library."));
  }

  eca_alsa_load_dl_snd_pcm();
  eca_alsa_load_dl_snd_pcm_loopback();
  eca_alsa_load_dl_snd_rawmidi();

  if (dlerror() != NULL) {
    throw(new ECA_ERROR("AUDIOIO-ALSA", "Error while loading asound.so dynamic library."));
  }
}

void eca_alsa_unload_dynamic_support(void) {
  assert(eca_alsa_client_count >= 0);
  
  --eca_alsa_client_count;

  if (eca_alsa_client_count == 0) {
    dlclose(eca_alsa_dynlib_handle);
    eca_alsa_dynlib_initialized = false;
  }
}

void eca_alsa_load_dl_snd_pcm(void) {
  dl_snd_pcm_open = 
    (int (*)(snd_pcm_t **, int, int, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_open");
  dl_snd_pcm_close = 
    (int (*)(snd_pcm_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_close");
  dl_snd_strerror = (const char* (*)(int))dlsym(eca_alsa_dynlib_handle, "snd_strerror");
  dl_snd_pcm_write = 
    (ssize_t (*)(snd_pcm_t *, const void *, size_t))dlsym(eca_alsa_dynlib_handle, "snd_pcm_write");
  dl_snd_pcm_read = 
    (ssize_t (*)(snd_pcm_t *, void *, size_t))dlsym(eca_alsa_dynlib_handle, "snd_pcm_read");
#ifdef ALSALIB_032
  dl_snd_pcm_playback_pause =
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_pause");
  dl_snd_pcm_block_mode = 
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_block_mode");
  dl_snd_pcm_info = 
    (int (*)(snd_pcm_t *, snd_pcm_info_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_info");
  dl_snd_pcm_playback_info = 
    (int (*)(snd_pcm_t *, snd_pcm_playback_info_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_info");
  dl_snd_pcm_playback_format = 
    (int (*)(snd_pcm_t *, snd_pcm_format_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_format");
  dl_snd_pcm_playback_params = 
    (int (*)(snd_pcm_t *handle, snd_pcm_playback_params_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_params");
  dl_snd_pcm_playback_status = 
    (int (*)(snd_pcm_t *handle, snd_pcm_playback_status_t * status))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_status");
  dl_snd_pcm_drain_playback = 
    (int (*)(snd_pcm_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_drain_playback");
  dl_snd_pcm_flush_playback = 
    (int (*)(snd_pcm_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_flush_playback");
  dl_snd_pcm_playback_pause = 
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_pause");
  dl_snd_pcm_playback_time = 
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_time");

  dl_snd_pcm_capture_info = 
    (int (*)(snd_pcm_t *, snd_pcm_capture_info_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_capture_info");
  dl_snd_pcm_capture_format = 
    (int (*)(snd_pcm_t *handle, snd_pcm_format_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_capture_format");
  dl_snd_pcm_capture_params = 
    (int (*)(snd_pcm_t *handle, snd_pcm_capture_params_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_capture_params");
  dl_snd_pcm_capture_status = 
    (int (*)(snd_pcm_t *handle, snd_pcm_capture_status_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_capture_status");
  dl_snd_pcm_flush_capture = 
    (int (*)(snd_pcm_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_flush_capture");
  dl_snd_pcm_capture_time = 
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_capture_time");
#else
  dl_snd_pcm_open_subdevice = 
    (int (*)(snd_pcm_t **, int, int, int, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_open_subdevice");
  dl_snd_pcm_nonblock_mode = 
    (int (*)(snd_pcm_t *handle, int ))dlsym(eca_alsa_dynlib_handle, "snd_pcm_nonblock_mode");
  dl_snd_pcm_info =
    (int (*)(snd_pcm_t *, snd_pcm_info_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_info");
  dl_snd_pcm_channel_info =
    (int (*)(snd_pcm_t *, snd_pcm_channel_info_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_channel_info");
  dl_snd_pcm_channel_params =
    (int (*)(snd_pcm_t *, snd_pcm_channel_params_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_channel_params");
  dl_snd_pcm_channel_status =
    (int (*)(snd_pcm_t *, snd_pcm_channel_status_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_channel_status");
  dl_snd_pcm_channel_setup =
    (int (*)(snd_pcm_t *, snd_pcm_channel_setup_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_channel_setup");
  dl_snd_pcm_playback_drain = 
    (int (*)(snd_pcm_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_drain");
  dl_snd_pcm_channel_prepare =
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_channel_prepare");
  dl_snd_pcm_channel_go =
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_channel_go");
  dl_snd_pcm_sync_go =
    (int (*)(snd_pcm_t *, snd_pcm_sync_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_sync_go");
  dl_snd_pcm_channel_flush =
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_channel_flush");
  dl_snd_pcm_playback_pause =
    (int (*)(snd_pcm_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_playback_pause");
#endif
}

void eca_alsa_load_dl_snd_pcm_loopback(void) {
  dl_snd_pcm_loopback_close = 
    (int (*)(snd_pcm_loopback_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_close");
  dl_snd_pcm_loopback_block_mode =
    (int (*)(snd_pcm_loopback_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_block_mode");
  dl_snd_pcm_loopback_stream_mode = 
    (int (*)(snd_pcm_loopback_t *, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_stream_mode");
  dl_snd_pcm_loopback_format =
    (int (*)(snd_pcm_loopback_t *handle, snd_pcm_format_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_format");
#ifdef ALSALIB_032
  dl_snd_pcm_loopback_open = 
    (int (*)(snd_pcm_loopback_t **, int, int, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_open");
  dl_snd_pcm_loopback_read = 
    (int (*)(snd_pcm_loopback_t *, void *, size_t))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_read");
#else
  dl_snd_pcm_loopback_open = 
    (int (*)(snd_pcm_loopback_t **, int, int, int, int))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_open");
  dl_snd_pcm_loopback_status = 
    (int (*)(snd_pcm_loopback_t *, snd_pcm_loopback_status_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_status");
  dl_snd_pcm_loopback_read = 
    (ssize_t (*)(snd_pcm_loopback_t *, snd_pcm_loopback_callbacks_t *))dlsym(eca_alsa_dynlib_handle, "snd_pcm_loopback_read");
#endif
}

void eca_alsa_load_dl_snd_rawmidi(void) {
  dl_snd_rawmidi_open =
    (int (*)(snd_rawmidi_t **, int, int, int))
    dlsym(eca_alsa_dynlib_handle, "snd_rawmidi_open");

  dl_snd_rawmidi_close =
    (int (*)(snd_rawmidi_t *handle)) dlsym(eca_alsa_dynlib_handle, "snd_rawmidi_close");

  dl_snd_rawmidi_file_descriptor =
    (int (*)(snd_rawmidi_t *handle)) dlsym(eca_alsa_dynlib_handle, "snd_rawmidi_file_descriptor");

  dl_snd_rawmidi_block_mode =
    (int (*)(snd_rawmidi_t *handle, int)) dlsym(eca_alsa_dynlib_handle, "snd_rawmidi_block_mode");

  dl_snd_rawmidi_info =
    (int (*)(snd_rawmidi_t *handle, snd_rawmidi_info_t * info))
    dlsym(eca_alsa_dynlib_handle, "snd_rawmidi_info");

  dl_snd_rawmidi_write =
    (int (*)(snd_rawmidi_t *handle, const void *buffer, size_t size)) dlsym(eca_alsa_dynlib_handle, "snd_rawmidi_write");

  dl_snd_rawmidi_read =
    (int (*)(snd_rawmidi_t *handle, void *buffer, size_t size)) dlsym(eca_alsa_dynlib_handle, "snd_rawmidi_read");
}

#endif
