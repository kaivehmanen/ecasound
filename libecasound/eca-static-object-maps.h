#ifndef INCLUDED_ECA_STATIC_OBJECT_MAPS_H
#define INCLUDED_ECA_STATIC_OBJECT_MAPS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class ECA_OBJECT_MAP;
class ECA_PRESET_MAP;

/*
 * When possible, use of the class ECA_OBJECT_FACTORY 
 * should be preferred to directed access to map objects.
 */

extern ECA_OBJECT_MAP* eca_audio_object_map;
extern ECA_OBJECT_MAP* eca_chain_operator_map;
extern ECA_OBJECT_MAP* eca_ladspa_plugin_map;
extern ECA_OBJECT_MAP* eca_ladspa_plugin_id_map;
extern ECA_OBJECT_MAP* eca_controller_map;
extern ECA_OBJECT_MAP* eca_midi_device_map;
extern ECA_PRESET_MAP* eca_preset_map;

void register_default_objects(void);
void unregister_default_objects(void);

#endif
