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

#include <config.h>

#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <algorithm>

#include "eca-chainop.h"
#include "audiofx.h"
#include "audiofx_amplitude.h"
#include "audiofx_analysis.h"
#include "audiofx_filter.h"
#include "audiofx_rcfilter.h"
#include "audiofx_mixing.h"
#include "audiofx_timebased.h"
#include "audiogate.h"
#include "audiofx_ladspa.h"
#include "ladspa.h"

#include "eca-static-object-maps.h"
#include "generic-controller.h"
#include "ctrl-source.h"
#include "midi-cc.h"
#include "osc-gen.h"
#include "osc-sine.h"
#include "linear-envelope.h"
#include "two-stage-linear-envelope.h"

#include "audioio-types.h"
#include "audioio-cdr.h"
#include "audioio-wave.h"
#include "audioio-oss.h"
#include "audioio-ewf.h"
#include "audioio-mp3.h"
#include "audioio-mikmod.h"
#include "audioio-alsa.h"
#include "audioio-alsa2.h"
#include "audioio-alsalb.h"
#include "audioio-af.h"
#include "audioio-raw.h"
#include "audioio-null.h"
#include "audioio-rtnull.h"

#include "eca-resources.h"
#include "eca-error.h"

ECA_AUDIO_OBJECT_MAP eca_audio_object_map;
ECA_AUDIO_OBJECT_MAP eca_audio_device_map;
ECA_CHAIN_OPERATOR_MAP eca_chain_operator_map;
ECA_LADSPA_PLUGIN_MAP eca_ladspa_plugin_map;
ECA_CONTROLLER_MAP eca_controller_map;
ECA_PRESET_MAP eca_preset_map;

void register_default_audio_objects(void);
void register_default_controllers(void);
void register_default_chainops(void);
void register_default_presets(void);
void register_ladspa_plugins(void);
vector<EFFECT_LADSPA*> create_plugins(const string& fname) throw(ECA_ERROR*);

void register_default_objects(void) {
  static bool defaults_registered = false;
  if (defaults_registered == true) return;
  defaults_registered = true;

  register_default_controllers();
  register_default_chainops();
  register_default_audio_objects();
  register_ladspa_plugins();
}

void register_default_audio_objects(void) {
  eca_audio_object_map.register_object(".wav", new WAVEFILE());
  eca_audio_object_map.register_object(".ewf", new EWFFILE());
  eca_audio_object_map.register_object(".cdr", new CDRFILE());

  AUDIO_IO* raw = new RAWFILE();
  eca_audio_object_map.register_object(".raw", raw);

  AUDIO_IO* mp3 = new MP3FILE();
  eca_audio_object_map.register_object(".mp3", mp3);
  eca_audio_object_map.register_object(".mp2", mp3);

  AUDIO_IO* mikmod = new MIKMOD_INTERFACE();
  eca_audio_object_map.register_object(".669", mikmod);
  eca_audio_object_map.register_object(".amf", mikmod);
  eca_audio_object_map.register_object(".dsm", mikmod);
  eca_audio_object_map.register_object(".far", mikmod);
  eca_audio_object_map.register_object(".gdm", mikmod);
  eca_audio_object_map.register_object(".imf", mikmod);
  eca_audio_object_map.register_object(".it", mikmod);
  eca_audio_object_map.register_object(".m15", mikmod);
  eca_audio_object_map.register_object(".med", mikmod);
  eca_audio_object_map.register_object(".mod", mikmod);
  eca_audio_object_map.register_object(".mtm", mikmod);
  eca_audio_object_map.register_object(".s3m", mikmod);
  eca_audio_object_map.register_object(".stm", mikmod);
  eca_audio_object_map.register_object(".stx", mikmod);
  eca_audio_object_map.register_object(".ult", mikmod);
  eca_audio_object_map.register_object(".uni", mikmod);
  eca_audio_object_map.register_object(".xm", mikmod);

#ifdef COMPILE_AF
  AUDIO_IO* af = new AUDIOFILE_INTERFACE();
  eca_audio_object_map.register_object(".aif", af);
  eca_audio_object_map.register_object(".au", af);
  eca_audio_object_map.register_object(".snd", af);
#endif
  
  AUDIO_IO* device = new OSSDEVICE();
  eca_audio_object_map.register_object("/dev/dsp", device);
  eca_audio_device_map.register_object("/dev/dsp", device);

#ifdef COMPILE_ALSA
  device = new ALSA_LOOPBACK_DEVICE();
  eca_audio_object_map.register_object("alsalb", device);
  eca_audio_device_map.register_object("alsalb", device);
#endif

#ifdef ALSALIB_050
  device = new ALSA_PCM2_DEVICE();
  eca_audio_object_map.register_object("alsa", device);
  eca_audio_device_map.register_object("alsa", device);
#endif
#ifdef ALSALIB_032
  device = new ALSA_PCM_DEVICE();
  eca_audio_object_map.register_object("alsa", device);      
  eca_audio_device_map.register_object("alsa", device);
#endif

  device = new REALTIME_NULL();
  eca_audio_object_map.register_object("rtnull", device);
  eca_audio_device_map.register_object("rtnull", device);

  eca_audio_object_map.register_object("stdin", raw);
  eca_audio_object_map.register_object("stdout", raw);
  eca_audio_object_map.register_object("null", new NULLFILE());
}

void register_default_chainops(void) {
  eca_chain_operator_map.register_object("ea", new EFFECT_AMPLIFY());
  eca_chain_operator_map.register_object("eac", new EFFECT_AMPLIFY_CHANNEL());
  eca_chain_operator_map.register_object("eaw", new EFFECT_AMPLIFY_CLIPCOUNT());
  eca_chain_operator_map.register_object("ec", new EFFECT_COMPRESS());
  eca_chain_operator_map.register_object("eca", new ADVANCED_COMPRESSOR());
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
  eca_controller_map.register_object("kf", new GENERIC_CONTROLLER(new GENERIC_OSCILLATOR()));
  eca_controller_map.register_object("kl", new GENERIC_CONTROLLER(new LINEAR_ENVELOPE()));
  eca_controller_map.register_object("kl2", new GENERIC_CONTROLLER(new TWO_STAGE_LINEAR_ENVELOPE()));
  eca_controller_map.register_object("km", new GENERIC_CONTROLLER(new MIDI_CONTROLLER()));
  eca_controller_map.register_object("kos", new GENERIC_CONTROLLER(new SINE_OSCILLATOR()));
}

void register_default_presets(void) { }

void register_ladspa_plugins(void) {
  DIR *dp;

  ECA_RESOURCES ecarc;
  string dir_name = ecarc.resource("ladspa-plugin-directory");
  
  struct stat statbuf;
  dp = opendir(dir_name.c_str());
  if (dp != 0) {
    struct dirent *entry = readdir(dp);
    while(entry != 0) {
      lstat(entry->d_name, &statbuf);
      if (string(entry->d_name).find(".so") != string::npos) {
	vector<EFFECT_LADSPA*> ladspa_plugins;
	try {
	  ladspa_plugins = create_plugins(dir_name + "/" + string(entry->d_name));
	}
	catch(ECA_ERROR *e) { cerr << e->error_msg() << endl; }
	for(unsigned int n = 0; n < ladspa_plugins.size(); n++) {
	  eca_ladspa_plugin_map.register_object(ladspa_plugins[n]->unique(), ladspa_plugins[n]);
	}
      }
      entry = readdir(dp);
    }
  }
}

vector<EFFECT_LADSPA*> create_plugins(const string& fname) throw(ECA_ERROR*) { 
  vector<EFFECT_LADSPA*> plugins;

  void *plugin_handle = dlopen(fname.c_str(), RTLD_NOW);
  if (plugin_handle == 0) 
    throw(new ECA_ERROR("ECA_STATIC_OBJECT_MAPS", "Unable to open plugin file."));

  vector<LADSPA_Descriptor_Function> desc_funcs;
  desc_funcs.push_back(reinterpret_cast<LADSPA_Descriptor_Function>(dlsym(plugin_handle, "descriptor")));
  if (desc_funcs[0] == 0)
    throw(new ECA_ERROR("ECA_STATIC_OBJECT_MAPS", "Unable find plugin LADSPA-descriptor."));

  struct LADSPA_Descriptor *plugin_desc = 0;
  for (int i = 0;; i++) {
    plugin_desc = desc_funcs[0](i);
    if (plugin_desc == 0) break;
    try {
      plugins.push_back(new EFFECT_LADSPA(plugin_desc));
    }
    catch (ECA_ERROR*) { }
    plugin_desc = 0;
  }

  //  if (plugin_handle != 0) dlclose(plugin_handle);
  return(plugins);
}
