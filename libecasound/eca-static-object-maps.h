#ifndef INCLUDED_ECA_STATIC_OBJECT_MAPS_H
#define INCLUDED_ECA_STATIC_OBJECT_MAPS_H

class ECA_FACTORY_MAP;
class ECA_OBJECT_MAP;
class ECA_PRESET_MAP;

/**
 * A private classed used by ECA_OBJECT_FACTORY
 * to access object maps.
 */
class ECA_STATIC_OBJECT_MAPS {

 public:

  friend class ECA_OBJECT_FACTORY;

 private:

  static ECA_OBJECT_MAP* audio_object_map(void);
  static ECA_OBJECT_MAP* chain_operator_map(void);
  static ECA_OBJECT_MAP* ladspa_plugin_map(void);
  static ECA_OBJECT_MAP* ladspa_plugin_id_map(void);
  static ECA_OBJECT_MAP* controller_map(void);
  static ECA_OBJECT_MAP* midi_device_map(void);
  static ECA_PRESET_MAP* preset_map(void);

  static bool default_objects_registered(void);
  static void register_default_objects(void);
  static void unregister_default_objects(void);
};

#endif
