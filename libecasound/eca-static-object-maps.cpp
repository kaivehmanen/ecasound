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

#ifdef ECA_FEELING_EXPERIMENTAL
extern "C" {
#include "audiofx_vst.h"
}
#endif

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
#include "audioio-reverse.h"

#include "midiio-raw.h"

#include "eca-object-map.h"
#include "eca-preset-map.h"
#include "eca-static-object-maps.h"

#include "eca-resources.h"
#include "eca-logger.h"
#include "eca-error.h"

using std::cerr;
using std::endl;
using std::string;
using std::vector;

static ECA_OBJECT_MAP* eca_audio_object_map = 0;
static ECA_OBJECT_MAP* eca_chain_operator_map = 0;
static ECA_OBJECT_MAP* eca_ladspa_plugin_map = 0;
static ECA_OBJECT_MAP* eca_ladspa_plugin_id_map = 0;
static ECA_OBJECT_MAP* eca_controller_map = 0;
static ECA_OBJECT_MAP* eca_midi_device_map = 0;
static ECA_PRESET_MAP* eca_preset_map = 0;
static ECA_OBJECT_MAP* eca_empty_map = 0;

/* static ECA_VST_PLUGIN_MAP* eca_vst_plugin_map = 0; */

static int eca_default_objects_registered = 0;

static void register_default_audio_objects(void);
static void register_default_controllers(void);
static void register_default_chainops(void);
//  static void register_default_presets(void);
static void register_ladspa_plugins(void);
static void register_internal_audioio_plugins(void);
static void register_default_midi_devices(void);

static vector<EFFECT_LADSPA*> create_plugins(const string& fname);
static void register_internal_audioio_plugin(const string& libdir, const string& filename);
static ECA_OBJECT_MAP* check_map_validity(ECA_OBJECT_MAP* mapobj);

static ECA_OBJECT_MAP* check_map_validity(ECA_OBJECT_MAP* mapobj) {
  if (mapobj == 0) {
    if (eca_empty_map == 0) {
      eca_empty_map = new ECA_OBJECT_MAP();
    }
    std::cerr << "(eca-static-object-maps) Warning! Access to uninitialized object map!" << std::endl;
    return(eca_empty_map);
  }
  return(mapobj);
}


ECA_OBJECT_MAP* ECA_STATIC_OBJECT_MAPS::audio_object_map(void) 
{
  return(check_map_validity(eca_audio_object_map));
}

ECA_OBJECT_MAP* ECA_STATIC_OBJECT_MAPS::chain_operator_map(void)
{
  return(check_map_validity(eca_chain_operator_map));
}

ECA_OBJECT_MAP* ECA_STATIC_OBJECT_MAPS::ladspa_plugin_map(void)
{
  return(check_map_validity(eca_ladspa_plugin_map));
}

ECA_OBJECT_MAP* ECA_STATIC_OBJECT_MAPS::ladspa_plugin_id_map(void)
{
  return(check_map_validity(eca_ladspa_plugin_id_map));
}

ECA_OBJECT_MAP* ECA_STATIC_OBJECT_MAPS::controller_map(void) 
{
  return(check_map_validity(eca_controller_map));
}

ECA_OBJECT_MAP* ECA_STATIC_OBJECT_MAPS::midi_device_map(void) 
{
  return(check_map_validity(eca_midi_device_map));
}

ECA_PRESET_MAP* ECA_STATIC_OBJECT_MAPS::preset_map(void) 
{
  return(static_cast<ECA_PRESET_MAP*>(check_map_validity(static_cast<ECA_OBJECT_MAP*>(eca_preset_map))));
}

bool ECA_STATIC_OBJECT_MAPS::default_objects_registered(void) {
  if (eca_default_objects_registered == 0) return(false);

  return(true);
}

void ECA_STATIC_OBJECT_MAPS::register_default_objects(void) {
  if (eca_default_objects_registered == 0) {

    //  std::cerr << "Creating factory." << endl;

    eca_audio_object_map = new ECA_OBJECT_MAP();
    eca_chain_operator_map = new ECA_OBJECT_MAP();
    eca_ladspa_plugin_map = new ECA_OBJECT_MAP();
    eca_ladspa_plugin_id_map = new ECA_OBJECT_MAP();
    eca_controller_map = new ECA_OBJECT_MAP();
    eca_midi_device_map = new ECA_OBJECT_MAP();
    eca_preset_map = new ECA_PRESET_MAP();

    /*  eca_vst_plugin_map = new ECA_VST_PLUGIN_MAP(); */

    register_default_controllers();
    register_default_chainops();
    register_default_audio_objects();
    register_default_midi_devices();

    register_internal_audioio_plugins();
    register_ladspa_plugins();
  }

  ++eca_default_objects_registered;
}

void ECA_STATIC_OBJECT_MAPS::unregister_default_objects(void) {
  --eca_default_objects_registered;
  if (eca_default_objects_registered == 0) {

    //  std::cerr << "Deleting factory." << endl;

    delete eca_audio_object_map;
    delete eca_chain_operator_map;
    delete eca_ladspa_plugin_map;
    eca_ladspa_plugin_id_map->flush();
    delete eca_ladspa_plugin_id_map;
    delete eca_controller_map;
    delete eca_midi_device_map;
    delete eca_preset_map;
    
#ifdef FEELING_EXPERIMENTAL
    delete eca_vst_plugin_map;
#endif
    
    eca_audio_object_map = 0;
    eca_chain_operator_map = 0;
    eca_ladspa_plugin_map = 0;
    eca_ladspa_plugin_id_map = 0;
    eca_controller_map = 0;
    eca_midi_device_map = 0;
    eca_preset_map = 0;
    
#ifdef FEELING_EXPERIMENTAL
    eca_vst_plugin_map = 0;
#endif
  }
}

static void register_default_audio_objects(void) {
  eca_audio_object_map->register_object("wav", "wav$", new WAVEFILE());
  eca_audio_object_map->register_object("ewf", "ewf$", new EWFFILE());
  eca_audio_object_map->register_object("cdr", "cdr$", new CDRFILE());

  AUDIO_IO* raw = new RAWFILE();
  eca_audio_object_map->register_object("raw", "raw$", raw);

  AUDIO_IO* mp3 = new MP3FILE();
  eca_audio_object_map->register_object("mp3", "mp3$", mp3);
  eca_audio_object_map->register_object("mp2", "mp2$", mp3);

  AUDIO_IO* ogg = new OGG_VORBIS_INTERFACE();
  eca_audio_object_map->register_object("ogg", "ogg$", ogg);

  AUDIO_IO* mikmod = new MIKMOD_INTERFACE();
  eca_audio_object_map->register_object("mikmod_669", "669$", mikmod);
  eca_audio_object_map->register_object("mikmod_amf", "amf$", mikmod);
  eca_audio_object_map->register_object("mikmod_dsm", "dsm$", mikmod);
  eca_audio_object_map->register_object("mikmod_far", "far$", mikmod);
  eca_audio_object_map->register_object("mikmod_gdm", "gdm$", mikmod);
  eca_audio_object_map->register_object("mikmod_imf", "imf$", mikmod);
  eca_audio_object_map->register_object("mikmod_it", "it$", mikmod);
  eca_audio_object_map->register_object("mikmod_m15", "m15$", mikmod);
  eca_audio_object_map->register_object("mikmod_ed", "ed$", mikmod);
  eca_audio_object_map->register_object("mikmod_mod", "mod$", mikmod);
  eca_audio_object_map->register_object("mikmod_mtm", "mtm$", mikmod);
  eca_audio_object_map->register_object("mikmod_s3m", "s3m$", mikmod);
  eca_audio_object_map->register_object("mikmod_stm", "stm$", mikmod);
  eca_audio_object_map->register_object("mikmod_stx", "stx$", mikmod);
  eca_audio_object_map->register_object("mikmod_ult", "ult$", mikmod);
  eca_audio_object_map->register_object("mikmod_uni", "uni$", mikmod);
  eca_audio_object_map->register_object("mikmod_xm", "xm$", mikmod);

  AUDIO_IO* timidity = new TIMIDITY_INTERFACE();
  eca_audio_object_map->register_object("mid", "mid$", timidity);
  eca_audio_object_map->register_object("midi", "midi$", timidity);

  AUDIO_IO* device = 0;  
#ifdef ECA_COMPILE_OSS
  device = new OSSDEVICE();
  eca_audio_object_map->register_object("/dev/dsp", "/dev/dsp[0-9]*", device);
  eca_audio_object_map->register_object("/dev/sound/dsp", "/dev/sound/dsp[0-9]*", device);
#endif

  device = new REALTIME_NULL();
  eca_audio_object_map->register_object("rtnull", "^rtnull$", device);

  eca_audio_object_map->register_object("-", "^-$", raw);
  eca_audio_object_map->register_object("stdin", "^stdin$", raw);
  eca_audio_object_map->register_object("stdout", "^stdout$", raw);
  eca_audio_object_map->register_object("null", "^null$", new NULLFILE());
  eca_audio_object_map->register_object("typeselect", "^typeselect$", new AUDIO_IO_TYPESELECT());
  eca_audio_object_map->register_object("reverse", "^reverse$", new AUDIO_IO_REVERSE());
}

static void register_default_chainops(void) {
  eca_chain_operator_map->register_object("eS", "^eS$", new EFFECT_AUDIO_STAMP());
  eca_chain_operator_map->register_object("ea", "^ea$", new EFFECT_AMPLIFY());
  eca_chain_operator_map->register_object("eac", "^eac$", new EFFECT_AMPLIFY_CHANNEL());
  eca_chain_operator_map->register_object("eal", "^eal$", new EFFECT_LIMITER());
  eca_chain_operator_map->register_object("eaw", "^eaw$", new EFFECT_AMPLIFY_CLIPCOUNT());
  eca_chain_operator_map->register_object("ec", "^ec$", new EFFECT_COMPRESS());
  eca_chain_operator_map->register_object("eca", "^eca$", new ADVANCED_COMPRESSOR());
  eca_chain_operator_map->register_object("eemb", "^eemb$", new EFFECT_PULSE_GATE_BPM());
  eca_chain_operator_map->register_object("eemp", "^eemp$", new EFFECT_PULSE_GATE());
  eca_chain_operator_map->register_object("eemt", "^eemt$", new EFFECT_TREMOLO());
  eca_chain_operator_map->register_object("ef1", "^ef1$", new EFFECT_RESONANT_BANDPASS());
  eca_chain_operator_map->register_object("ef3", "^ef3$", new EFFECT_RESONANT_LOWPASS());
  eca_chain_operator_map->register_object("ef4", "^ef4$", new EFFECT_RC_LOWPASS_FILTER());
  eca_chain_operator_map->register_object("efa", "^efa$", new EFFECT_ALLPASS_FILTER());
  eca_chain_operator_map->register_object("efb", "^efb$", new EFFECT_BANDPASS());
  eca_chain_operator_map->register_object("efc", "^efc$", new EFFECT_COMB_FILTER());
  eca_chain_operator_map->register_object("efh", "^efh$", new EFFECT_HIGHPASS());
  eca_chain_operator_map->register_object("efi", "^efi$", new EFFECT_INVERSE_COMB_FILTER());
  eca_chain_operator_map->register_object("efl", "^efl$", new EFFECT_LOWPASS());
  eca_chain_operator_map->register_object("efr", "^efr$", new EFFECT_BANDREJECT());
  eca_chain_operator_map->register_object("efs", "^efs$", new EFFECT_RESONATOR());
  eca_chain_operator_map->register_object("ei", "^ei$", new EFFECT_PITCH_SHIFT());
  eca_chain_operator_map->register_object("enm", "^enm$", new EFFECT_NOISEGATE());
  eca_chain_operator_map->register_object("epp", "^epp$", new EFFECT_NORMAL_PAN());
  eca_chain_operator_map->register_object("erc", "^erc$", new EFFECT_CHANNEL_COPY());
  eca_chain_operator_map->register_object("erm", "^erm$", new EFFECT_MIX_TO_CHANNEL());
  eca_chain_operator_map->register_object("etc", "^etc$", new EFFECT_CHORUS());
  eca_chain_operator_map->register_object("etd", "^etd$", new EFFECT_DELAY());
  eca_chain_operator_map->register_object("ete", "^ete$", new ADVANCED_REVERB());
  eca_chain_operator_map->register_object("etf", "^etf$", new EFFECT_FAKE_STEREO());
  eca_chain_operator_map->register_object("etl", "^etl$", new EFFECT_FLANGER());
  eca_chain_operator_map->register_object("etm", "^etm$", new EFFECT_MULTITAP_DELAY());
  eca_chain_operator_map->register_object("etp", "^etp$", new EFFECT_PHASER());
  eca_chain_operator_map->register_object("etr", "^etr$", new EFFECT_REVERB());
  eca_chain_operator_map->register_object("ev", "^ev$", new EFFECT_ANALYZE());
  eca_chain_operator_map->register_object("ezf", "^ezf$", new EFFECT_DCFIND());
  eca_chain_operator_map->register_object("ezx", "^ezx$", new EFFECT_DCFIX());
  eca_chain_operator_map->register_object("gc", "^gc$", new TIME_CROP_GATE());
  eca_chain_operator_map->register_object("ge", "^ge$", new THRESHOLD_GATE());
}

static void register_default_controllers(void) {
  eca_controller_map->register_object("kf", "^kf$", new GENERIC_CONTROLLER(new GENERIC_OSCILLATOR_FILE()));
  eca_controller_map->register_object("kog", "^kog$", new GENERIC_CONTROLLER(new GENERIC_OSCILLATOR()));
  eca_controller_map->register_object("kl", "^kl$", new GENERIC_CONTROLLER(new LINEAR_ENVELOPE()));
  eca_controller_map->register_object("kl2", "^kl2$", new GENERIC_CONTROLLER(new TWO_STAGE_LINEAR_ENVELOPE()));
  eca_controller_map->register_object("klg", "^klg$", new GENERIC_CONTROLLER(new GENERIC_LINEAR_ENVELOPE()));
  eca_controller_map->register_object("km", "^km$", new GENERIC_CONTROLLER(new MIDI_CONTROLLER()));
  eca_controller_map->register_object("kos", "^kos$", new GENERIC_CONTROLLER(new SINE_OSCILLATOR()));
  eca_controller_map->register_object("ksv", "^ksv$", new GENERIC_CONTROLLER(new VOLUME_ANALYZE_CONTROLLER()));
}

//  static void register_default_presets(void) { }

static void register_internal_audioio_plugin(const string& libdir,
					     const string& filename) {

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
	    eca_audio_object_map->register_object(plugin_keyword(), plugin_keyword_regex(), aobj);
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
    std::cerr << "(eca-static-object-maps) dlopen() failed; " << file;
    std::cerr << ": \"" << dlerror() << "\"." << std::endl;
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

/**
 * Registers internal audio plugins to the object
 * maps.
 */
static void register_internal_audioio_plugins(void) {
  ECA_RESOURCES ecarc;
  string libdir = ecarc.resource("internal-plugin-directory");

  struct stat fbuf;
  if (stat(libdir.c_str(), &fbuf) < 0) {
    ECA_LOG_MSG(ECA_LOGGER::info, "(eca-static-object-maps) Internal-plugin directory not found. Check your ~/.ecasoundrc!");
    return;
  }

  register_internal_audioio_plugin(libdir, "libaudioio_af.so");
  register_internal_audioio_plugin(libdir, "libaudioio_alsa.so");
  register_internal_audioio_plugin(libdir, "libaudioio_alsa_named.so");
  register_internal_audioio_plugin(libdir, "libaudioio_arts.so");
  register_internal_audioio_plugin(libdir, "libaudioio_jack.so");

  const ECA_OBJECT* aobj = 0;

  aobj = eca_audio_object_map->object("alsahw_09");
  if (aobj != 0) {
    eca_audio_object_map->register_object("alsahw", "^alsahw$", const_cast<ECA_OBJECT*>(aobj));
    eca_audio_object_map->register_object("alsaplugin", "^alsaplugin$", const_cast<ECA_OBJECT*>(aobj));
  }

  aobj = eca_audio_object_map->object("alsa_09");
  if (aobj != 0) {
    eca_audio_object_map->register_object("alsa", "^alsa$", const_cast<ECA_OBJECT*>(aobj));
  }
}

static void register_default_midi_devices(void) {
  eca_midi_device_map->register_object("rawmidi", "^rawmidi$", new MIDI_IO_RAW());
}

static void register_ladspa_plugins(void) {
  DIR *dp;

  vector<string> dir_names;
  char* env = std::getenv("LADSPA_PATH");
  if (env != 0) 
    dir_names = kvu_string_to_vector(string(), ':');
  ECA_RESOURCES ecarc;
  string add_file = ecarc.resource("ladspa-plugin-directory");
  if (find(dir_names.begin(), dir_names.end(), add_file) == dir_names.end()) dir_names.push_back(add_file);

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
	    ladspa_plugins = create_plugins(*p + "/" + entry_name);
	}
	catch(ECA_ERROR& e) {  }
	for(unsigned int n = 0; n < ladspa_plugins.size(); n++) {
	  eca_ladspa_plugin_map->register_object(ladspa_plugins[n]->unique(), "^" + ladspa_plugins[n]->unique() + "$", ladspa_plugins[n]);
	  eca_ladspa_plugin_id_map->register_object(kvu_numtostr(ladspa_plugins[n]->unique_number()),
						    kvu_numtostr(ladspa_plugins[n]->unique_number()), 
						    ladspa_plugins[n]);
	}
	entry = readdir(dp);
      }
    }
    ++p;
  }
}

vector<EFFECT_LADSPA*> create_plugins(const string& fname) { 
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
