#ifndef INCLUDED_ECA_OBJECT_FACTORY_H
#define INCLUDED_ECA_OBJECT_FACTORY_H

#include <string>

class AUDIO_IO;
class MIDI_IO;

/**
 * Class for creating various ecasound objects.
 * @author Kai Vehmanen
 */
class ECA_OBJECT_FACTORY {

 public:

  /**
   * Create a new audio object based on the formatted argument string.
   *
   * @param arg a formatted string describing an audio object, see ecasound 
   *            manuals for detailed info
   * @return the created object or 0 if an invalid format string was given 
   *         as the argument
   *
   * require:
   *  arg.empty() != true
   */
  static AUDIO_IO* create_audio_object(const string& arg);

  /**
   * Create a new MIDI-device object based on the formatted argument string.
   *
   * @param arg a formatted string describing a MIDI-device object, see ecasound 
   *            manuals for detailed info
   * @return the created object or 0 if an invalid format string was given 
   *         as the argument
   *
   * require:
   *  arg.empty() != true
   */
  static MIDI_IO* create_midi_device(const string& arg);

};

#endif
