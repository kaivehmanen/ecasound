#ifndef _ECA_STATIC_OBJECT_MAPS_H
#define _ECA_STATIC_OBJECT_MAPS_H

#include <config.h>
#include <string>

#include "eca-object-map.h"
#include "eca-preset-map.h"
#include "eca-vst-plugin-map.h"

#include "eca-chainop.h"
#include "ctrl-source.h"

extern ECA_OBJECT_MAP eca_audio_object_map;
extern ECA_OBJECT_MAP eca_audio_device_map;
extern ECA_OBJECT_MAP eca_chain_operator_map;
extern ECA_OBJECT_MAP eca_ladspa_plugin_map;
extern ECA_OBJECT_MAP eca_controller_map;
extern ECA_PRESET_MAP eca_preset_map;

#ifdef FEELING_EXPERIMENTAL
extern ECA_VST_PLUGIN_MAP eca_vst_plugin_map;
#endif

void register_default_objects(void);

#endif

