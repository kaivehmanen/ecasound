#ifndef INCLUDED_ECA_OBJECT_FACTORY_H
#define INCLUDED_ECA_OBJECT_FACTORY_H

#include <map>
#include <string>

class EFFECT_LADSPA;
class GENERIC_CONTROLLER;
class CHAIN_OPERATOR;
class AUDIO_IO;
class LOOP_DEVICE;
class MIDI_IO;

/**
 * Class for creating various ecasound objects.
 * @author Kai Vehmanen
 */
class ECA_OBJECT_FACTORY {

 public:

  static EFFECT_LADSPA* ladspa_map_object(const std::string& keyword);
  static EFFECT_LADSPA* ladspa_map_object(long int number);
  static GENERIC_CONTROLLER* controller_map_object(const std::string& keyword);
  static CHAIN_OPERATOR* chain_operator_map_object(const std::string& keyword);
  static AUDIO_IO* audio_io_map_object(const std::string& keyword, bool use_regex = true);

  static AUDIO_IO* create_audio_object(const std::string& arg);
  static MIDI_IO* create_midi_device(const std::string& arg);
  static AUDIO_IO* create_loop_output(const std::string& argu, std::map<int,LOOP_DEVICE*>* loop_map);
  static AUDIO_IO* create_loop_input(const std::string& argu, std::map<int,LOOP_DEVICE*>* loop_map);
  static CHAIN_OPERATOR* create_chain_operator (const std::string& arg);
  static CHAIN_OPERATOR* create_ladspa_plugin (const std::string& arg);
  static CHAIN_OPERATOR* create_vst_plugin (const std::string& arg);
  static GENERIC_CONTROLLER* create_controller (const std::string& arg);

};

#endif
