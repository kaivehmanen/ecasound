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
class ECA_AUDIO_OBJECT_MAP {

  ECA_OBJECT_MAP omap;

 public:

  /**
   * Register a new effect.
   */
  void register_object(const string& id_string, AUDIO_IO* object);

  /**
   * List of registered objects (keywords).
   */
  const map<string,string>& registered_objects(void) const;

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

#endif

