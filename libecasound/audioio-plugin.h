#ifndef INCLUDED_AUDIOIO_PLUGIN_H
#define INCLUDED_AUDIOIO_PLUGIN_H

class AUDIO_IO;

typedef AUDIO_IO * 
(*audio_io_descriptor)(void);
typedef int 
(*audio_io_interface_version)(void);

#endif
