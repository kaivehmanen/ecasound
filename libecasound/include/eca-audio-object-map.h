#ifndef _ECA_AUDIO_OBJECT_MAP_H
#define _ECA_AUDIO_OBJECT_MAP_H

#include <string>
#include <map>

#include "audioio.h"
#include "eca-object-map.h"

/**
 * Dynamic register for audio objects their id-substrings
 *
 * @author Kai Vehmanen
 */
class ECA_AUDIO_OBJECT_MAP : public ECA_OBJECT_MAP<AUDIO_IO> {

  mutable map<string, AUDIO_IO*> object_map;
  mutable map<string, string> object_prefix_map;
  vector<string> object_names;

 public:

  /**
   * Register a new effect.
   */
  void register_object(const string& id_string, AUDIO_IO* object);

  /**
   * List of registered objects (keywords).
   */
  const vector<string>& registered_objects(void) const;

  /**
   * Return the first object that matches with 'keyword'
   */
  AUDIO_IO* object(const string& keyword) const;

  /**
   * Return the matching keyword for 'object'.
   */
  string object_identifier(const AUDIO_IO* object) const;

  virtual ~ECA_AUDIO_OBJECT_MAP(void) { }  
};

/**
 * Dynamic register for audio file objects
 *
 * @author Kai Vehmanen
 */
class ECA_AUDIO_FILE_MAP : public ECA_AUDIO_OBJECT_MAP {

  bool defaults_registered;

 public:

  /**
   * Register default chain operators
   */
  void register_default_objects(void);

  ECA_AUDIO_FILE_MAP(void) : defaults_registered(false) { }
};

/**
 * Dynamic register for audio device objects
 *
 * @author Kai Vehmanen
 */
class ECA_AUDIO_DEVICE_MAP : public ECA_AUDIO_OBJECT_MAP {

  bool defaults_registered;

 public:

  /**
   * Register default chain operators
   */
  void register_default_objects(void);

  ECA_AUDIO_DEVICE_MAP(void) : defaults_registered(false) { }
};

#endif

