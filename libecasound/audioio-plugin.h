#ifndef INCLUDED_AUDIOIO_PLUGIN_H
#define INCLUDED_AUDIOIO_PLUGIN_H

class AUDIO_IO;

/**
 * Prefix path
 **/
const char* ecasound_plugin_path = "/usr/local";

typedef const AUDIO_IO * 
(*audio_io_descriptor)(void);

#endif
