#ifndef _ECA_LADSPA_PLUGIN_MAP_H
#define _ECA_LADSPA_PLUGIN_MAP_H

#include <string>
#include <map>

#include "audiofx_ladspa.h"
#include "eca-object-map.h"

/**
 * Dynamic register for chain operators and their id-strings
 *
 * @author Kai Vehmanen
 */
class ECA_LADSPA_PLUGIN_MAP {

  ECA_OBJECT_MAP omap;

 public:

  /**
   * Register a new effect
   */
  void register_object(const string& id_string, EFFECT_LADSPA* object) { omap.register_object(id_string,object); }

  /**
   * List of registered effects ('effect name'-'keyword' map).
   */
  const map<string,string>& registered_objects(void) const { return(omap.registered_objects()); }

  /**
   * Return the first effect that matches with 'keyword'
   */
  EFFECT_LADSPA* object(const string& keyword) const  { return(dynamic_cast<EFFECT_LADSPA*>(omap.object(keyword))); }

  /**
   * Return the matching keyword for 'object'.
   */
  string object_identifier(const EFFECT_LADSPA* object) const { return("el"); }

  virtual ~ECA_LADSPA_PLUGIN_MAP(void) { }
};

#endif
