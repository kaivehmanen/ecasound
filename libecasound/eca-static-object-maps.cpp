// ------------------------------------------------------------------------
// eca-static-object-maps.h: Static object map instances
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <algorithm>

#include <kvutils/kvu_numtostr.h>
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

extern "C" {
#include "ladspa.h"

#ifdef FEELING_EXPERIMENTAL
#include "audiofx_vst.h"
#endif
}

#include "eca-static-object-maps.h"
#include "generic-controller.h"
#include "ctrl-source.h"
#include "midi-cc.h"
#include "osc-gen.h"
#include "osc-gen-file.h"
#include "osc-sine.h"
#include "linear-envelope.h"
#include "two-stage-linear-envelope.h"
#include "stamp-ctrl.h"

#include "audioio-plugin.h"
#include "audioio-types.h"
#include "audioio-cdr.h"
#include "audioio-wave.h"
#include "audioio-oss.h"
#include "audioio-ewf.h"
#include "audioio-mp3.h"
#include "audioio-ogg.h"
#include "audioio-mikmod.h"
#include "audioio-timidity.h"
#include "audioio-raw.h"
#include "audioio-null.h"
#include "audioio-rtnull.h"

#include "eca-resources.h"
#include "eca-error.h"

// FIXME: get rid of these static objects (make them pointer or something)
ECA_OBJECT_MAP eca_audio_object_map;
ECA_OBJECT_MAP eca_audio_device_map;
ECA_OBJECT_MAP eca_chain_operator_map;
ECA_OBJECT_MAP eca_ladspa_plugin_map;
ECA_OBJECT_MAP eca_ladspa_plugin_id_map;
ECA_OBJECT_MAP eca_controller_map;
ECA_PRESET_MAP eca_preset_map;

#ifdef FEELING_EXPERIMENTAL
ECA_VST_PLUGIN_MAP eca_vst_plugin_map;
#endif

void register_default_audio_objects(void);
void register_default_controllers(void);
void register_default_chainops(void);
void register_default_presets(void);
void register_ladspa_plugins(void);
void register_internal_plugins(void);

vector<EFFECT_LADSPA*> create_plugins(const string& fname) throw(ECA_ERROR&);

void register_default_objects(void) {
  static bool defaults_registered = false;
  if (defaults_registered == true) return;
  defaults_registered = true;

  register_default_controllers();
  register_default_chainops();
  register_default_audio_objects();
  register_internal_plugins();
  register_ladspa_plugins();
}

void register_default_audio_objects(void) {
  eca_audio_object_map.register_object("\\.wav$", new WAVEFILE());
  eca_audio_object_map.register_object("\\.ewf$", new EWFFILE());
  eca_audio_object_map.register_object("\\.cdr$", new CDRFILE());

  AUDIO_IO* raw = new RAWFILE();
  eca_audio_object_map.register_object("\\.raw$", raw);

  AUDIO_IO* mp3 = new MP3FILE();
  eca_audio_object_map.register_object("\\.mp3$", mp3);
  eca_audio_object_map.register_object("\\.mp2$", mp3);

  AUDIO_IO* ogg = new OGG_VORBIS_INTERFACE();
  eca_audio_object_map.register_object("\\.ogg$", ogg);

  AUDIO_IO* mikmod = new MIKMOD_INTERFACE();
  eca_audio_object_map.register_object("\\.669$", mikmod);
  eca_audio_object_map.register_object("\\.amf$", mikmod);
  eca_audio_object_map.register_object("\\.dsm$", mikmod);
  eca_audio_object_map.register_object("\\.far$", mikmod);
  eca_audio_object_map.register_object("\\.gdm$", mikmod);
  eca_audio_object_map.register_object("\\.imf$", mikmod);
  eca_audio_object_map.register_object("\\.it$", mikmod);
  eca_audio_object_map.register_object("\\.m15$", mikmod);
  eca_audio_object_map.register_object("\\.ed$", mikmod);
  eca_audio_object_map.register_object("\\.mod$", mikmod);
  eca_audio_object_map.register_object("\\.mtm$", mikmod);
  eca_audio_object_map.register_object("\\.s3m$", mikmod);
  eca_audio_object_map.register_object("\\.stm$", mikmod);
  eca_audio_object_map.register_object("\\.stx$", mikmod);
  eca_audio_object_map.register_object("\\.ult$", mikmod);
  eca_audio_object_map.register_object("\\.uni$", mikmod);
  eca_audio_object_map.register_object("\\.xm$", mikmod);

  AUDIO_IO* timidity = new TIMIDITY_INTERFACE();
  eca_audio_object_map.register_object("\\.mid$", timidity);
  eca_audio_object_map.register_object("\\.midi$", timidity);

  AUDIO_IO* device = 0;  
#ifdef COMPILE_OSS
  device = new OSSDEVICE();
  eca_audio_object_map.register_object("/dev/dsp[0-9]*", device);
  eca_audio_device_map.register_object("/dev/dsp[0-9]*", device);
#endif

  device = new REALTIME_NULL();
  eca_audio_object_map.register_object("^rtnull$", device);
  eca_audio_device_map.register_object("^rtnull$", device);

  eca_audio_object_map.register_object("^-$", raw);
  eca_audio_object_map.register_object("^stdin$", raw);
  eca_audio_object_map.register_object("^stdout$", raw);
  eca_audio_object_map.register_object("^null$", new NULLFILE());
}

void register_default_chainops(void) {
  eca_chain_operator_map.register_object("eS", new EFFECT_AUDIO_STAMP());
  eca_chain_operator_map.register_object("ea", new EFFECT_AMPLIFY());
  eca_chain_operator_map.register_object("eac", new EFFECT_AMPLIFY_CHANNEL());
  eca_chain_operator_map.register_object("eal", new EFFECT_LIMITER());
  eca_chain_operator_map.register_object("eaw", new EFFECT_AMPLIFY_CLIPCOUNT());
  eca_chain_operator_map.register_object("ec", new EFFECT_COMPRESS());
  eca_chain_operator_map.register_object("eca", new ADVANCED_COMPRESSOR());
  eca_chain_operator_map.register_object("eemb", new EFFECT_PULSE_GATE_BPM());
  eca_chain_operator_map.register_object("eemp", new EFFECT_PULSE_GATE());
  eca_chain_operator_map.register_object("eemt", new EFFECT_TREMOLO());
  eca_chain_operator_map.register_object("ef1", new EFFECT_RESONANT_BANDPASS());
  eca_chain_operator_map.register_object("ef3", new EFFECT_RESONANT_LOWPASS());
  eca_chain_operator_map.register_object("ef4", new EFFECT_RC_LOWPASS_FILTER());
  eca_chain_operator_map.register_object("efa", new EFFECT_ALLPASS_FILTER());
  eca_chain_operator_map.register_object("efb", new EFFECT_BANDPASS());
  eca_chain_operator_map.register_object("efc", new EFFECT_COMB_FILTER());
  eca_chain_operator_map.register_object("efh", new EFFECT_HIGHPASS());
  eca_chain_operator_map.register_object("efi", new EFFECT_INVERSE_COMB_FILTER());
  eca_chain_operator_map.register_object("efl", new EFFECT_LOWPASS());
  eca_chain_operator_map.register_object("efr", new EFFECT_BANDREJECT());
  eca_chain_operator_map.register_object("efs", new EFFECT_RESONATOR());
  eca_chain_operator_map.register_object("ei", new EFFECT_PITCH_SHIFT());
  eca_chain_operator_map.register_object("enm", new EFFECT_NOISEGATE());
  eca_chain_operator_map.register_object("epp", new EFFECT_NORMAL_PAN());
  eca_chain_operator_map.register_object("erc", new EFFECT_CHANNEL_COPY());
  eca_chain_operator_map.register_object("erm", new EFFECT_MIX_TO_CHANNEL());
  eca_chain_operator_map.register_object("etc", new EFFECT_CHORUS());
  eca_chain_operator_map.register_object("etd", new EFFECT_DELAY());
  eca_chain_operator_map.register_object("ete", new ADVANCED_REVERB());
  eca_chain_operator_map.register_object("etf", new EFFECT_FAKE_STEREO());
  eca_chain_operator_map.register_object("etl", new EFFECT_FLANGER());
  eca_chain_operator_map.register_object("etm", new EFFECT_MULTITAP_DELAY());
  eca_chain_operator_map.register_object("etp", new EFFECT_PHASER());
  eca_chain_operator_map.register_object("etr", new EFFECT_REVERB());
  eca_chain_operator_map.register_object("ev", new EFFECT_ANALYZE());
  eca_chain_operator_map.register_object("ezf", new EFFECT_DCFIND());
  eca_chain_operator_map.register_object("ezx", new EFFECT_DCFIX());
  eca_chain_operator_map.register_object("gc", new TIME_CROP_GATE());
  eca_chain_operator_map.register_object("ge", new THRESHOLD_GATE());
}

void register_default_controllers(void) {
  eca_controller_map.register_object("kf", new GENERIC_CONTROLLER(new GENERIC_OSCILLATOR_FILE()));
  eca_controller_map.register_object("kog", new GENERIC_CONTROLLER(new GENERIC_OSCILLATOR()));
  eca_controller_map.register_object("kl", new GENERIC_CONTROLLER(new LINEAR_ENVELOPE()));
  eca_controller_map.register_object("kl2", new GENERIC_CONTROLLER(new TWO_STAGE_LINEAR_ENVELOPE()));
  eca_controller_map.register_object("km", new GENERIC_CONTROLLER(new MIDI_CONTROLLER()));
  eca_controller_map.register_object("kos", new GENERIC_CONTROLLER(new SINE_OSCILLATOR()));
  eca_controller_map.register_object("ksv", new GENERIC_CONTROLLER(new VOLUME_ANALYZE_CONTROLLER()));
}

void register_default_presets(void) { }

static AUDIO_IO* register_internal_plugin(const string& libdir,
					       const string& filename) {
  string file = libdir + string("/") + filename;
  audio_io_descriptor desc_func = 0;
  void *plugin_handle = dlopen(file.c_str(), RTLD_NOW);
  if (plugin_handle != 0) {
    audio_io_interface_version plugin_version;
    plugin_version = (audio_io_interface_version)dlsym(plugin_handle, "audio_io_interface_version");
    if (plugin_version != 0) {
      int version = plugin_version();
      if (version < ecasound_library_version_current -
	  ecasound_library_version_age ||
	  version > ecasound_library_version_current) {
	ecadebug->msg(ECA_DEBUG::info, 
		      "(eca-static-object-maps) Opening internal plugin file \"" + 
		      file + 
		      "\" failed. Plugin version " + 
		      kvu_numtostr(version) +
		      " doesn't match libecasound version " +
		      kvu_numtostr(ecasound_library_version_current) + "." +
		      kvu_numtostr(ecasound_library_version_revision) + "." +
		      kvu_numtostr(ecasound_library_version_age) + ".");
	return(0);
      }
      else {
	desc_func = (audio_io_descriptor)dlsym(plugin_handle, "audio_io_descriptor");
	if (desc_func != 0) {
	  return(desc_func());
	}
      }
    }
  }
  if (plugin_handle == 0 ||
      desc_func == 0) {
    ecadebug->msg(ECA_DEBUG::user_objects, 
		  "(eca-static-object-maps) Opening internal plugin file \"" + 
		  file + 
		  "\" failed, because: \"" + 
		  dlerror() +
		  "\"");
  }
    
  return(0);
}

void register_internal_plugins(void) {
  ECA_RESOURCES ecarc;
  string libdir = ecarc.resource("internal-plugin-directory");

  struct stat fbuf;
  if (stat(libdir.c_str(), &fbuf) < 0) {
    ecadebug->msg(ECA_DEBUG::info, "(eca-static-object-maps) Internal-plugin directory not found. Check your ~/.ecasoundrc!");
    return;
  }

  AUDIO_IO* aobj;
  aobj = register_internal_plugin(libdir, "libaudioio_af.so");
  if (aobj != 0) {
#ifdef COMPILE_AF
    eca_audio_object_map.register_object("\\.aif*", aobj);
    eca_audio_object_map.register_object("\\.au$", aobj);
    eca_audio_object_map.register_object("\\.snd$", aobj);
#endif
  }

  aobj = register_internal_plugin(libdir, "libaudioio_alsa.so");
  if (aobj != 0) {
    eca_audio_object_map.register_object("^alsa_03$", aobj);
    eca_audio_device_map.register_object("^alsa_03$", aobj);
#ifdef ALSALIB_032
    eca_audio_object_map.register_object("^alsa$", aobj);
    eca_audio_device_map.register_object("^alsa$", aobj);
#endif
  }

  aobj = register_internal_plugin(libdir, "libaudioio_alsalb.so");
  if (aobj != 0) {
#if (defined ALSALIB_032 || defined ALSALIB_050)
    eca_audio_object_map.register_object("^alsalb$", aobj);
    eca_audio_device_map.register_object("^alsalb$", aobj);
#endif
  }

  aobj = register_internal_plugin(libdir, "libaudioio_alsa2_plugin.so");
  if (aobj != 0) {
    eca_audio_object_map.register_object("^alsaplugin_05$", aobj);
    eca_audio_device_map.register_object("^alsaplugin_05$", aobj);
#ifdef ALSALIB_050
    eca_audio_object_map.register_object("^alsaplugin$", aobj);
    eca_audio_device_map.register_object("^alsaplugin$", aobj);
#endif
  }

  aobj = register_internal_plugin(libdir, "libaudioio_alsa2.so");
  if (aobj != 0) {
    eca_audio_object_map.register_object("^alsa_05$", aobj);
    eca_audio_device_map.register_object("^alsa_05$", aobj);
#ifdef ALSALIB_050
    eca_audio_object_map.register_object("^alsa$", aobj);
    eca_audio_device_map.register_object("^alsa$", aobj);
#endif
  }

  aobj = register_internal_plugin(libdir, "libaudioio_alsa3.so");
  if (aobj != 0) {
    eca_audio_object_map.register_object("^alsa_06$", aobj);
    eca_audio_device_map.register_object("^alsa_06$", aobj);
    eca_audio_object_map.register_object("^alsaplugin_06$", aobj);
    eca_audio_device_map.register_object("^alsaplugin_06$", aobj);
#ifdef ALSALIB_060
    eca_audio_object_map.register_object("^alsa$", aobj);
    eca_audio_device_map.register_object("^alsa$", aobj);
    eca_audio_object_map.register_object("^alsaplugin$", aobj);
    eca_audio_device_map.register_object("^alsaplugin$", aobj);
#endif
  }

  aobj = register_internal_plugin(libdir, "libaudioio_arts.so");
  if (aobj != 0) {
#ifdef COMPILE_ARTS
    eca_audio_object_map.register_object("^arts$", aobj);
    eca_audio_device_map.register_object("^arts$", aobj);
#endif
  }
}

void register_ladspa_plugins(void) {
  DIR *dp;

  vector<string> dir_names;
  char* env = getenv("LADSPA_PATH");
  if (env != 0) 
    dir_names = string_to_vector(string(), ':');
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
	catch(ECA_ERROR& e) { cerr << e.error_message() << endl; }
	for(unsigned int n = 0; n < ladspa_plugins.size(); n++) {
	  eca_ladspa_plugin_map.register_object(ladspa_plugins[n]->unique(), ladspa_plugins[n]);
	  eca_ladspa_plugin_id_map.register_object(kvu_numtostr(ladspa_plugins[n]->unique_number()), ladspa_plugins[n]);
	}
	entry = readdir(dp);
      }
    }
    ++p;
  }
}

vector<EFFECT_LADSPA*> create_plugins(const string& fname) throw(ECA_ERROR&) { 
  vector<EFFECT_LADSPA*> plugins;

  void *plugin_handle = dlopen(fname.c_str(), RTLD_NOW);
  if (plugin_handle == 0) 
    throw(ECA_ERROR("ECA_STATIC_OBJECT_MAPS", string("Unable to open
 plugin file \"") + fname + "\"."));

  LADSPA_Descriptor_Function desc_func;
  
  desc_func = (LADSPA_Descriptor_Function)dlsym(plugin_handle, "ladspa_descriptor");
  if (desc_func == 0)
    throw(ECA_ERROR("ECA_STATIC_OBJECT_MAPS", "Unable find plugin LADSPA-descriptor."));

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

  //  if (plugin_handle != 0) dlclose(plugin_handle);
  return(plugins);
}
