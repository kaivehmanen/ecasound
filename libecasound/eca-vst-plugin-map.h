#ifndef _ECA_VST_PLUGIN_MAP_H
#define _ECA_VST_PLUGIN_MAP_H

#include <config.h>
#ifdef FEELING_EXPERIMENTAL

#include <string>
#include <vector>
#include <map>

#include "audiofx_vst.h"
#include "eca-object-map.h"

/**
 * Dynamic register for storing VST1.0/2.0 plugins
 *
 * @author Kai Vehmanen
 */
class ECA_VST_PLUGIN_MAP : public ECA_OBJECT_MAP {

 private:

  mutable map<string,string> object_map;
  mutable map<string,string> object_keyword_map;

 public:

  /**
   * Register a new VST plugin
   */
  void register_object(const string& id_string, EFFECT_VST* object);

  /**
   * List of registered EFFECT_VSTs ('EFFECT_VST name'-'keyword' map).
   */
  const map<string,string>& registered_objects(void) const;

  /**
   * Return the first object that matches with 'keyword'
   */
  ECA_OBJECT* object(const string& keyword) const;

  /**
   * Return the matching keyword for 'object'.
   */
  string object_identifier(const EFFECT_VST* object) const;

  ECA_VST_PLUGIN_MAP(void);
  virtual ~ECA_VST_PLUGIN_MAP(void) { }
};

#endif
#endif
