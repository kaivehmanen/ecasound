#ifndef _ECA_AUDIO_OBJECT_MAP_H
#define _ECA_AUDIO_OBJECT_MAP_H

#include <string>
#include <map>

#include "audioio.h"

/**
 * Dynamic register for audio objects their id-substrings
 *
 * @author Kai Vehmanen
 */
class ECA_AUDIO_OBJECT_MAP {

 public:

  /**
   * Register a new effect.
   */
  static void register_object(const string& id_string, AUDIO_IO* object);

  /**
   * List of registered objects (keywords).
   */
  static const map<string,string>& registered_objects(void);

  /**
   * Return the first object that matches with 'keyword'. If 
   * 'use_regex', regular expression matching is used.
   */
  static AUDIO_IO* object(const string& keyword, bool use_regex = true);

  /**
   * Return the matching keyword for 'object'.
   */
  static string object_identifier(const AUDIO_IO* object);
};

#endif

