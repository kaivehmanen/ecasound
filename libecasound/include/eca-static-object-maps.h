#ifndef _ECA_STATIC_OBJECT_MAPS_H
#define _ECA_STATIC_OBJECT_MAPS_H

#include "eca-audio-object-map.h"
#include "eca-chainop-map.h"
#include "eca-controller-map.h"
#include "eca-preset-map.h"

extern ECA_AUDIO_OBJECT_MAP eca_audio_object_map;
extern ECA_AUDIO_OBJECT_MAP eca_audio_device_map;
extern ECA_CHAIN_OPERATOR_MAP eca_chain_operator_map;
extern ECA_CONTROLLER_MAP eca_controller_map;
extern ECA_PRESET_MAP eca_preset_map;

void register_default_objects(void);

#endif
