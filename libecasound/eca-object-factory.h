#ifndef INCLUDED_ECA_OBJECT_FACTORY_H
#define INCLUDED_ECA_OBJECT_FACTORY_H

#include <string>

class EFFECT_LADSPA;
class GENERIC_CONTROLLER;
class CHAIN_OPERATOR;
class AUDIO_IO;
class MIDI_IO;

/**
 * Class for creating various ecasound objects.
 * @author Kai Vehmanen
 */
class ECA_OBJECT_FACTORY {

 public:

  static EFFECT_LADSPA* ladspa_map_object(const string& keyword);
  static EFFECT_LADSPA* ladspa_map_object(long int number);
  static GENERIC_CONTROLLER* controller_map_object(const string& keyword);
  static CHAIN_OPERATOR* chain_operator_map_object(const string& keyword);
  static AUDIO_IO* audio_io_map_object(const string& keyword, bool use_regex = true);

  static AUDIO_IO* create_audio_object(const string& arg);
  static MIDI_IO* create_midi_device(const string& arg);
};

#endif
