#ifndef INCLUDED_ECA_OBJECT_FACTORY_H
#define INCLUDED_ECA_OBJECT_FACTORY_H

#include <map>
#include <list>
#include <string>

class AUDIO_IO;
class CHAIN_OPERATOR;
class ECA_OBJECT;
class ECA_OBJECT_MAP;
class EFFECT_LADSPA;
class GENERIC_CONTROLLER;
class LOOP_DEVICE;
class MIDI_IO;
class PRESET;

/**
 * Class for creating various ecasound objects.
 * @author Kai Vehmanen
 */
class ECA_OBJECT_FACTORY {

 public:

  /** @name Functions for initialization and cleanup. */
  /*@{*/

  static void reserve_factory(void);
  static void free_factory(void);

  /*@}*/

  /** @name Functions that return a list of all registered object types (keywords). */
  /*@{*/

  static const std::list<std::string>& audio_io_list(void);
  static const std::list<std::string>& chain_operator_list(void);
  static const std::list<std::string>& preset_list(void);
  static const std::list<std::string>& ladspa_list(void);
  static const std::list<std::string>& controller_list(void);

  /*@}*/

  /** @name Functions for creating objects based on keyword argument. */
  /*@{*/

  static const AUDIO_IO* audio_io_map_object(const std::string& keyword);
  static const CHAIN_OPERATOR* chain_operator_map_object(const std::string& keyword);
  static const EFFECT_LADSPA* ladspa_map_object(const std::string& keyword);
  static const EFFECT_LADSPA* ladspa_map_object(long int number);
  static const PRESET* preset_object(const std::string& keyword);
  static const GENERIC_CONTROLLER* controller_map_object(const std::string& keyword);

  /*@}*/

  /** @name Functions for creating objects based on EOS (Ecasound Option Syntax) strings. */
  /*@{*/

  static AUDIO_IO* create_audio_object(const std::string& arg);
  static MIDI_IO* create_midi_device(const std::string& arg);
  static AUDIO_IO* create_loop_output(const std::string& argu, std::map<int,LOOP_DEVICE*>* loop_map);
  static AUDIO_IO* create_loop_input(const std::string& argu, std::map<int,LOOP_DEVICE*>* loop_map);
  static CHAIN_OPERATOR* create_chain_operator (const std::string& arg);
  static CHAIN_OPERATOR* create_ladspa_plugin (const std::string& arg);
  static CHAIN_OPERATOR* create_vst_plugin (const std::string& arg);
  static GENERIC_CONTROLLER* create_controller (const std::string& arg);

  /*@}*/

  /** @name Functions for querying object map contents. */
  /*@{*/

  static std::string object_identifier(const ECA_OBJECT* obj);

  /*@}*/

  /** @name Functions for direct (const-only) access to object maps. */
  /*@{*/

  static const ECA_OBJECT_MAP* audio_io_map(void);
  static const ECA_OBJECT_MAP* chain_operator_map(void);
  static const ECA_OBJECT_MAP* ladspa_map(void);
  static const ECA_OBJECT_MAP* preset_map(void);
  static const ECA_OBJECT_MAP* controller_map(void);

  /*@}*/

  /** @name Functions adding new object types. */
  /*@{*/

  static void register_chain_operator(const std::string& keyword, const std::string& expr, CHAIN_OPERATOR* object);
  static void register_controller(const std::string& keyword, const std::string& expr, GENERIC_CONTROLLER* object);

  /*@}*/

};

#endif
