#ifndef _ECA_ALSA_DYN_H
#define _ECA_ALSA_DYN_H

#include <config.h>
#ifdef COMPILE_ALSA

#include <sys/asoundlib.h>

extern const char* (*dl_snd_strerror)(int);

extern int (*dl_snd_pcm_open)(snd_pcm_t **,int,int,int);
extern int (*dl_snd_pcm_close)(snd_pcm_t *handle);
extern int (*dl_snd_pcm_block_mode)(snd_pcm_t *handle, int enable);
extern int (*dl_snd_pcm_info)(snd_pcm_t *handle, snd_pcm_info_t * info);
extern int (*dl_snd_pcm_playback_info)(snd_pcm_t *handle, snd_pcm_playback_info_t * info);
extern int (*dl_snd_pcm_playback_format)(snd_pcm_t *handle, snd_pcm_format_t * format);
extern int (*dl_snd_pcm_playback_params)(snd_pcm_t *handle, snd_pcm_playback_params_t * params);
extern int (*dl_snd_pcm_playback_status)(snd_pcm_t *handle, snd_pcm_playback_status_t * status);
extern int (*dl_snd_pcm_flush_capture)(snd_pcm_t *handle);
extern int (*dl_snd_pcm_drain_playback)(snd_pcm_t *handle);
extern int (*dl_snd_pcm_flush_playback)(snd_pcm_t *handle);
extern int (*dl_snd_pcm_playback_pause)(snd_pcm_t *handle, int enable);
extern int (*dl_snd_pcm_playback_time)(snd_pcm_t *handle, int enable);
extern ssize_t (*dl_snd_pcm_write)(snd_pcm_t *handle, const void *buffer, size_t size);
extern ssize_t (*dl_snd_pcm_read)(snd_pcm_t *handle, void *buffer, size_t size);
extern int (*dl_snd_pcm_capture_info)(snd_pcm_t *handle, snd_pcm_capture_info_t * info);
extern int (*dl_snd_pcm_capture_format)(snd_pcm_t *handle, snd_pcm_format_t * format);
extern int (*dl_snd_pcm_capture_params)(snd_pcm_t *handle, snd_pcm_capture_params_t * params);
extern int (*dl_snd_pcm_capture_status)(snd_pcm_t *handle, snd_pcm_capture_status_t * status);
extern int (*dl_snd_pcm_capture_time)(snd_pcm_t *handle, int enable);

// ---
// ALSA-lib pcm loopback routines (snd_pcm_loopback_*)
// ---

extern int (*dl_snd_pcm_loopback_open)(snd_pcm_loopback_t **handle, int card, int device, int mode);
extern int (*dl_snd_pcm_loopback_close)(snd_pcm_loopback_t *handle);
// extern int (*dl_snd_pcm_loopback_file_descriptor)(snd_pcm_loopback_t *handle);
extern int (*dl_snd_pcm_loopback_block_mode)(snd_pcm_loopback_t *handle, int enable);
extern int (*dl_snd_pcm_loopback_stream_mode)(snd_pcm_loopback_t *handle, int mode);
extern int (*dl_snd_pcm_loopback_format)(snd_pcm_loopback_t *handle, snd_pcm_format_t * format);
extern ssize_t (*dl_snd_pcm_loopback_read)(snd_pcm_loopback_t *handle, void *buffer, size_t size);

// ---------------------------------------------------------------------

// ---
// ALSA-lib rawmidi routines (snd_rawmidi_*)
// ---

extern int (*dl_snd_rawmidi_open)(snd_rawmidi_t **handle, int card, int device, int mode);
extern int (*dl_snd_rawmidi_close)(snd_rawmidi_t *handle);
extern int (*dl_snd_rawmidi_file_descriptor)(snd_rawmidi_t *handle);
extern int (*dl_snd_rawmidi_block_mode)(snd_rawmidi_t *handle, int enable);
extern int (*dl_snd_rawmidi_info)(snd_rawmidi_t *handle, snd_rawmidi_info_t * info);
extern int (*dl_snd_rawmidi_output_params)(snd_rawmidi_t *handle, snd_rawmidi_output_params_t * params);
extern int (*dl_snd_rawmidi_input_params)(snd_rawmidi_t *handle, snd_rawmidi_input_params_t * params);
extern int (*dl_snd_rawmidi_output_status)(snd_rawmidi_t *handle, snd_rawmidi_output_status_t * status);
extern int (*dl_snd_rawmidi_input_status)(snd_rawmidi_t *handle, snd_rawmidi_input_status_t * status);
extern int (*dl_snd_rawmidi_drain_output)(snd_rawmidi_t *handle);
extern int (*dl_snd_rawmidi_flush_output)(snd_rawmidi_t *handle);
extern int (*dl_snd_rawmidi_flush_input)(snd_rawmidi_t *handle);
extern ssize_t (*dl_snd_rawmidi_write)(snd_rawmidi_t *handle, const void *buffer, size_t size);
extern ssize_t (*dl_snd_rawmidi_read)(snd_rawmidi_t *handle, void *buffer, size_t size);

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

void eca_alsa_load_dynamic_support(void);
void eca_alsa_unload_dynamic_support(void);

#endif // COMPILE_ALSA
#endif
