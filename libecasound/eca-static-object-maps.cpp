// ------------------------------------------------------------------------
// eca-static-object-maps.h: Static object map instances
// Copyright (C) 2000-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>

#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <kvu_numtostr.h>

#include "eca-version.h"
#include "eca-chainop.h"
#include "audiofx.h"
#include "audiofx_misc.h"
#include "audiofx_amplitude.h"
#include "audiofx_analysis.h"
#include "audiofx_envelope_modulation.h"
#include "audiofx_filter.h"
#include "audiofx_rcfilter.h"
#include "audiofx_reverb.h"
#include "audiofx_mixing.h"
#include "audiofx_timebased.h"
#include "audiogate.h"
#include "audiofx_ladspa.h"

#include "generic-controller.h"
#include "ctrl-source.h"
#include "midi-cc.h"
#include "osc-gen.h"
#include "osc-gen-file.h"
#include "osc-sine.h"
#include "linear-envelope.h"
#include "two-stage-linear-envelope.h"
#include "stamp-ctrl.h"
#include "generic-linear-envelope.h"

#include "audioio-plugin.h"
#include "audioio-cdr.h"
#include "audioio-wave.h"
#ifdef ECA_COMPILE_OSS
#include "audioio-oss.h"
#endif
#include "audioio-ewf.h"
#include "audioio-mp3.h"
#include "audioio-ogg.h"
#include "audioio-mikmod.h"
#include "audioio-timidity.h"
#include "audioio-raw.h"
#include "audioio-null.h"
#include "audioio-rtnull.h"
#include "audioio-typeselect.h"
#include "audioio-resample.h"
#include "audioio-reverse.h"

#ifndef ECA_ENABLE_AUDIOIO_PLUGINS
#ifdef ECA_COMPILE_AUDIOFILE
#include "plugins/audioio_af.h"
#endif
#ifdef ECA_COMPILE_ALSA
#include "plugins/audioio_alsa.h"
#include "plugins/audioio_alsa_named.h"
#endif
#ifdef ECA_COMPILE_ARTS
#include "plugins/audioio_arts.h"
#endif
#ifdef ECA_COMPILE_JACK
#include "plugins/audioio_jack.h"
#endif
#endif /* ECA_ENABLE_AUDIOIO_PLUGINS */

#include "midiio-raw.h"

#include "eca-object-map.h"
#include "eca-preset-map.h"
#include "eca-static-object-maps.h"

#include "eca-resources.h"
#include "eca-logger.h"
#include "eca-error.h"

using std::cerr;
using std::endl;
using std::find;
using std::string;
using std::vector;

/**
 * Declarations for static private helper functions
 */

static vector<EFFECT_LADSPA*> eca_create_ladspa_plugins(const string& fname);
static void eca_import_ladspa_plugins(ECA_OBJECT_MAP* objmap, bool reg_with_id);

#ifdef ECA_ENABLE_AUDIOIO_PLUGINS
static void eca_import_internal_audioio_plugin(ECA_OBJECT_MAP* objmap, const string& filename);
#endif

/**
 * Definitions of static member functions
 */

void ECA_STATIC_OBJECT_MAPS::register_audio_io_rt_objects(ECA_OBJECT_MAP* objmap)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "register_audio_io_rt_objects()");

  AUDIO_IO* device = 0;  
#ifdef ECA_COMPILE_OSS
  device = new OSSDEVICE();
  objmap->register_object("/dev/dsp", "/dev/dsp[0-9]*", device);
  objmap->register_object("/dev/sound/dsp", "/dev/sound/dsp[0-9]*", device);
#endif

  device = new REALTIME_NULL();
  objmap->register_object("rtnull", "^rtnull$", device);

#ifdef ECA_ENABLE_AUDIOIO_PLUGINS
  eca_import_internal_audioio_plugin(objmap, "libaudioio_alsa.so");
  eca_import_internal_audioio_plugin(objmap, "libaudioio_alsa_named.so");
  eca_import_internal_audioio_plugin(objmap, "libaudioio_arts.so");
  eca_import_internal_audioio_plugin(objmap, "libaudioio_jack.so");
#else /* ECA_ENABLE_AUDIOIO_PLUGINS */
#ifdef ECA_COMPILE_ALSA
  device = new AUDIO_IO_ALSA_PCM();
  objmap->register_object("alsahw_09", "(^alsahw_09$)|(^alsaplugin_09$)", device);

  device = new AUDIO_IO_ALSA_PCM_NAMED();
  objmap->register_object("alsa_09", "^alsa_09$", device);
#endif

#ifdef ECA_COMPILE_ARTS
  device = new ARTS_INTERFACE();
  objmap->register_object("arts", "^arts$", device);
#endif

#ifdef ECA_COMPILE_JACK
  device = new AUDIO_IO_JACK();
  objmap->register_object("jack", "(^jack$)|(^jack_alsa$)|(^jack_auto$)|(^jack_generic$)", device);
#endif
#endif /* ECA_ENABLE_AUDIOIO_PLUGINS */

  const ECA_OBJECT* aobj = 0;

  aobj = objmap->object("alsahw_09");
  if (aobj != 0) {
    objmap->register_object("alsahw", "^alsahw$", const_cast<ECA_OBJECT*>(aobj));
    objmap->register_object("alsaplugin", "^alsaplugin$", const_cast<ECA_OBJECT*>(aobj));
  }

  aobj = objmap->object("alsa_09");
  if (aobj != 0) {
    objmap->register_object("alsa", "^alsa$", const_cast<ECA_OBJECT*>(aobj));
  }
}

void ECA_STATIC_OBJECT_MAPS::register_audio_io_nonrt_objects(ECA_OBJECT_MAP* objmap)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "register_audio_io_nonrt_objects()");

  objmap->register_object("wav", "wav$", new WAVEFILE());
  objmap->register_object("ewf", "ewf$", new EWFFILE());
  objmap->register_object("cdr", "cdr$", new CDRFILE());

  AUDIO_IO* raw = new RAWFILE();
  objmap->register_object("raw", "raw$", raw);

  AUDIO_IO* mp3 = new MP3FILE();
  objmap->register_object("mp3", "mp3$", mp3);
  objmap->register_object("mp2", "mp2$", mp3);

  AUDIO_IO* ogg = new OGG_VORBIS_INTERFACE();
  objmap->register_object("ogg", "ogg$", ogg);

  AUDIO_IO* mikmod = new MIKMOD_INTERFACE();
  objmap->register_object("mikmod_669", "669$", mikmod);
  objmap->register_object("mikmod_amf", "amf$", mikmod);
  objmap->register_object("mikmod_dsm", "dsm$", mikmod);
  objmap->register_object("mikmod_far", "far$", mikmod);
  objmap->register_object("mikmod_gdm", "gdm$", mikmod);
  objmap->register_object("mikmod_imf", "imf$", mikmod);
  objmap->register_object("mikmod_it", "it$", mikmod);
  objmap->register_object("mikmod_m15", "m15$", mikmod);
  objmap->register_object("mikmod_ed", "ed$", mikmod);
  objmap->register_object("mikmod_mod", "mod$", mikmod);
  objmap->register_object("mikmod_mtm", "mtm$", mikmod);
  objmap->register_object("mikmod_s3m", "s3m$", mikmod);
  objmap->register_object("mikmod_stm", "stm$", mikmod);
  objmap->register_object("mikmod_stx", "stx$", mikmod);
  objmap->register_object("mikmod_ult", "ult$", mikmod);
  objmap->register_object("mikmod_uni", "uni$", mikmod);
  objmap->register_object("mikmod_xm", "xm$", mikmod);

  AUDIO_IO* timidity = new TIMIDITY_INTERFACE();
  objmap->register_object("mid", "mid$", timidity);
  objmap->register_object("midi", "midi$", timidity);

#ifdef ECA_ENABLE_AUDIOIO_PLUGINS
  eca_import_internal_audioio_plugin(objmap, "libaudioio_af.so");
#else
#ifdef ECA_COMPILE_AUDIOFILLE
  AUDIO_IO* af = new AUDIOFILE_INTERFACE();
  objmap->register_object("audiofile_aiff_au_snd", "(aif*$)|(au$)|(snd$)", af);
#endif
#endif /* ECA_ENABLE_AUDIOIO_PLUGINS */

  objmap->register_object("-", "^-$", raw);
  objmap->register_object("stdin", "^stdin$", raw);
  objmap->register_object("stdout", "^stdout$", raw);
  objmap->register_object("null", "^null$", new NULLFILE());
  objmap->register_object("typeselect", "^typeselect$", new AUDIO_IO_TYPESELECT());
  objmap->register_object("resample", "^resample$", new AUDIO_IO_RESAMPLE());
  objmap->register_object("reverse", "^reverse$", new AUDIO_IO_REVERSE());
}

void ECA_STATIC_OBJECT_MAPS::register_chain_operator_objects(ECA_OBJECT_MAP* objmap)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "register_chain_operator_objects()");

  objmap->register_object("eS", "^eS$", new EFFECT_AUDIO_STAMP());
  objmap->register_object("ea", "^ea$", new EFFECT_AMPLIFY());
  objmap->register_object("eac", "^eac$", new EFFECT_AMPLIFY_CHANNEL());
  objmap->register_object("eal", "^eal$", new EFFECT_LIMITER());
  objmap->register_object("eaw", "^eaw$", new EFFECT_AMPLIFY_CLIPCOUNT());
  objmap->register_object("ec", "^ec$", new EFFECT_COMPRESS());
  objmap->register_object("eca", "^eca$", new ADVANCED_COMPRESSOR());
  objmap->register_object("eemb", "^eemb$", new EFFECT_PULSE_GATE_BPM());
  objmap->register_object("eemp", "^eemp$", new EFFECT_PULSE_GATE());
  objmap->register_object("eemt", "^eemt$", new EFFECT_TREMOLO());
  objmap->register_object("ef1", "^ef1$", new EFFECT_RESONANT_BANDPASS());
  objmap->register_object("ef3", "^ef3$", new EFFECT_RESONANT_LOWPASS());
  objmap->register_object("ef4", "^ef4$", new EFFECT_RC_LOWPASS_FILTER());
  objmap->register_object("efa", "^efa$", new EFFECT_ALLPASS_FILTER());
  objmap->register_object("efb", "^efb$", new EFFECT_BANDPASS());
  objmap->register_object("efc", "^efc$", new EFFECT_COMB_FILTER());
  objmap->register_object("efh", "^efh$", new EFFECT_HIGHPASS());
  objmap->register_object("efi", "^efi$", new EFFECT_INVERSE_COMB_FILTER());
  objmap->register_object("efl", "^efl$", new EFFECT_LOWPASS());
  objmap->register_object("efr", "^efr$", new EFFECT_BANDREJECT());
  objmap->register_object("efs", "^efs$", new EFFECT_RESONATOR());
  objmap->register_object("ei", "^ei$", new EFFECT_PITCH_SHIFT());
  objmap->register_object("enm", "^enm$", new EFFECT_NOISEGATE());
  objmap->register_object("epp", "^epp$", new EFFECT_NORMAL_PAN());
  objmap->register_object("erc", "^erc$", new EFFECT_CHANNEL_COPY());
  objmap->register_object("erm", "^erm$", new EFFECT_MIX_TO_CHANNEL());
  objmap->register_object("etc", "^etc$", new EFFECT_CHORUS());
  objmap->register_object("etd", "^etd$", new EFFECT_DELAY());
  objmap->register_object("ete", "^ete$", new ADVANCED_REVERB());
  objmap->register_object("etf", "^etf$", new EFFECT_FAKE_STEREO());
  objmap->register_object("etl", "^etl$", new EFFECT_FLANGER());
  objmap->register_object("etm", "^etm$", new EFFECT_MULTITAP_DELAY());
  objmap->register_object("etp", "^etp$", new EFFECT_PHASER());
  objmap->register_object("etr", "^etr$", new EFFECT_REVERB());
  objmap->register_object("ev", "^ev$", new EFFECT_VOLUME_BUCKETS());
  objmap->register_object("evp", "^evp$", new EFFECT_VOLUME_PEAK());
  objmap->register_object("ezf", "^ezf$", new EFFECT_DCFIND());
  objmap->register_object("ezx", "^ezx$", new EFFECT_DCFIX());
  objmap->register_object("gc", "^gc$", new TIME_CROP_GATE());
  objmap->register_object("ge", "^ge$", new THRESHOLD_GATE());
}

void ECA_STATIC_OBJECT_MAPS::register_ladspa_plugin_objects(ECA_OBJECT_MAP* objmap)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "register_ladspa_plugin_objects()");

  eca_import_ladspa_plugins(objmap, false);
}

void ECA_STATIC_OBJECT_MAPS::register_ladspa_plugin_id_objects(ECA_OBJECT_MAP* objmap)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "register_ladspa_plugin_id_objects()");

  eca_import_ladspa_plugins(objmap, true);
}

void ECA_STATIC_OBJECT_MAPS::register_preset_objects(ECA_PRESET_MAP* objmap)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "register_preset_objects()");
  /* @see ECA_PRESET_MAP */
}

void ECA_STATIC_OBJECT_MAPS::register_controller_objects(ECA_OBJECT_MAP* objmap)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "register_controller_objects()");

  objmap->register_object("kf", "^kf$", new GENERIC_CONTROLLER(new GENERIC_OSCILLATOR_FILE()));
  objmap->register_object("kog", "^kog$", new GENERIC_CONTROLLER(new GENERIC_OSCILLATOR()));
  objmap->register_object("kl", "^kl$", new GENERIC_CONTROLLER(new LINEAR_ENVELOPE()));
  objmap->register_object("kl2", "^kl2$", new GENERIC_CONTROLLER(new TWO_STAGE_LINEAR_ENVELOPE()));
  objmap->register_object("klg", "^klg$", new GENERIC_CONTROLLER(new GENERIC_LINEAR_ENVELOPE()));
  objmap->register_object("km", "^km$", new GENERIC_CONTROLLER(new MIDI_CONTROLLER()));
  objmap->register_object("kos", "^kos$", new GENERIC_CONTROLLER(new SINE_OSCILLATOR()));
  objmap->register_object("ksv", "^ksv$", new GENERIC_CONTROLLER(new VOLUME_ANALYZE_CONTROLLER()));
}

void ECA_STATIC_OBJECT_MAPS::register_midi_device_objects(ECA_OBJECT_MAP* objmap)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "register_midi_device_objects()");

  objmap->register_object("rawmidi", "^rawmidi$", new MIDI_IO_RAW());
}

/**
 * Definitions for static private helper functions
 */

#ifdef ECA_ENABLE_AUDIOIO_PLUGINS
static void eca_import_internal_audioio_plugin(ECA_OBJECT_MAP* objmap,
					       const string& filename)
{
  ECA_RESOURCES ecarc;
  string libdir = ecarc.resource("internal-plugin-directory");

  struct stat fbuf;
  if (stat(libdir.c_str(), &fbuf) < 0) {
    ECA_LOG_MSG(ECA_LOGGER::info, "(eca-static-object-maps) Internal-plugin directory not found. Check your ~/.ecasoundrc!");
    return;
  }

  string file = libdir + string("/") + filename;

  audio_io_descriptor desc_func = 0;
  audio_io_interface_version plugin_version = 0;
  audio_io_keyword plugin_keyword = 0;
  audio_io_keyword_regex plugin_keyword_regex = 0;
  
  void *plugin_handle = dlopen(file.c_str(), RTLD_NOW | RTLD_GLOBAL); /* RTLD_LAZY */

  if (plugin_handle != 0) {
    plugin_version = (audio_io_interface_version)dlsym(plugin_handle, "audio_io_interface_version");
    if (plugin_version != 0) {
      int version = plugin_version();
      if (version < ecasound_library_version_current -
	  ecasound_library_version_age ||
	  version > ecasound_library_version_current) {
	ECA_LOG_MSG(ECA_LOGGER::info, 
		      "(eca-static-object-maps) Opening internal plugin file \"" + 
		      file + 
		      "\" failed. Plugin version " + 
		      kvu_numtostr(version) +
		      " doesn't match libecasound version " +
		      kvu_numtostr(ecasound_library_version_current) + "." +
		      kvu_numtostr(ecasound_library_version_revision) + "." +
		      kvu_numtostr(ecasound_library_version_age) + ".");
      }
      else {
	desc_func = (audio_io_descriptor)dlsym(plugin_handle, "audio_io_descriptor");
	plugin_keyword = (audio_io_keyword)dlsym(plugin_handle, "audio_io_keyword");
	plugin_keyword_regex = (audio_io_keyword_regex)dlsym(plugin_handle, "audio_io_keyword_regex");
	if (desc_func != 0) {
	  AUDIO_IO* aobj = desc_func();
	  if (plugin_keyword != 0 && plugin_keyword_regex != 0) {
	    objmap->register_object(plugin_keyword(), plugin_keyword_regex(), aobj);
	    // std::cerr << "Registering audio io type: " << aobj->name()  << "\nType keyword " << plugin_keyword() << ",  regex " << plugin_keyword_regex() << "." << std::endl;
	  }
	}
      }
    }
    else {
      std::cerr << "(eca-static-object-maps) dlsym() failed; " << file;
      std::cerr << ": \"" << dlerror() << "\"." << std::endl;
    }
  }
  else {
    ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"(eca-static-object-maps) dlopen() failed; " + file +
		+ ": \"" + string(dlerror()) + "\".");
  }

  if (plugin_handle == 0 ||
      plugin_version == 0 ||
      plugin_keyword == 0 ||
      plugin_keyword_regex == 0 ||
      desc_func == 0) {
    ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		  "(eca-static-object-maps) Opening internal plugin file \"" + 
		  file + "\" failed.");
  }
}
#endif

static void eca_import_ladspa_plugins(ECA_OBJECT_MAP* objmap, bool reg_with_id)
{
  DIR *dp;

  vector<string> dir_names;
  char* env = std::getenv("LADSPA_PATH");
  if (env != 0) {
    dir_names = kvu_string_to_vector(string(env), ':');
  }

  ECA_RESOURCES ecarc;
  string add_file = ecarc.resource("ladspa-plugin-directory");
  if (std::find(dir_names.begin(), dir_names.end(), add_file) == dir_names.end()) dir_names.push_back(add_file);

  struct stat statbuf;
  vector<string>::const_iterator p = dir_names.begin();
  while (p != dir_names.end()) {
    dp = opendir(p->c_str());
    if (dp != 0) {
      struct dirent *entry = readdir(dp);
      while(entry != 0) {
	lstat(entry->d_name, &statbuf);
	vector<EFFECT_LADSPA*> ladspa_plugins;

	try {
	  string entry_name (entry->d_name);
	  if (entry_name.size() > 0 && entry_name[0] != '.')
	    ladspa_plugins = eca_create_ladspa_plugins(*p + "/" + entry_name);
	}
	catch(ECA_ERROR& e) {  }

	for(unsigned int n = 0; n < ladspa_plugins.size(); n++) {
	  if (reg_with_id == true) {
	    objmap->register_object(kvu_numtostr(ladspa_plugins[n]->unique_number()),
				    kvu_numtostr(ladspa_plugins[n]->unique_number()), 
				    ladspa_plugins[n]);
	  }
	  else {
	    objmap->register_object(ladspa_plugins[n]->unique(), "^" + ladspa_plugins[n]->unique() + "$", ladspa_plugins[n]);
	  }
	}

	entry = readdir(dp);
      }
    }
    ++p;
  }
}

static vector<EFFECT_LADSPA*> eca_create_ladspa_plugins(const string& fname)
{
  vector<EFFECT_LADSPA*> plugins;

  void *plugin_handle = dlopen(fname.c_str(), RTLD_NOW);
  if (plugin_handle != 0) {
    LADSPA_Descriptor_Function desc_func;
    
    desc_func = (LADSPA_Descriptor_Function)dlsym(plugin_handle, "ladspa_descriptor");
    if (desc_func != 0) {
      const LADSPA_Descriptor *plugin_desc = 0;
      for (int i = 0;; i++) {
	plugin_desc = desc_func(i);
	if (plugin_desc == 0) break;
	try {
	  plugins.push_back(new EFFECT_LADSPA(plugin_desc));
	}
	catch (ECA_ERROR&) { }
	plugin_desc = 0;
      }
    }
    else { 
      ECA_LOG_MSG(ECA_LOGGER::user_objects,
		    string("(eca-static-object-maps) ") + 
		    "Unable find plugin LADSPA-descriptor.");
    }
  }
  else {
    ECA_LOG_MSG(ECA_LOGGER::user_objects,
		  string("(eca-static-object-maps) ") + 
		  "Unable to open plugin file \"" + fname + "\".");
  }
  
  return(plugins);
}
