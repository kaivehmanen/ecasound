#ifndef _ECA_LADSPA_PLUGIN_MAP_H
#define _ECA_LADSPA_PLUGIN_MAP_H

#include <string>
#include <map>

#include <kvutils/kvu_numtostr.h>
#include "audiofx_ladspa.h"
#include "eca-static-object-maps.h"

/**
 * Dynamic register for chain operators and their id-strings
 *
 * @author Kai Vehmanen
 */
class ECA_LADSPA_PLUGIN_MAP {

 public:

  /**
   * Register a new effect
   */
  static void register_object(const string& id_string, EFFECT_LADSPA* object) { eca_ladspa_plugin_map.register_object(id_string,object); }

  /**
   * Register a new effect
   */
  static void register_object(long int id_number, EFFECT_LADSPA* object) { eca_ladspa_plugin_id_map.register_object(kvu_numtostr(id_number),object); }

  /**
   * List of registered effects ('effect name'-'keyword' map).
   */
  static const map<string,string>& registered_objects(void) { return(eca_ladspa_plugin_map.registered_objects()); }

  /**
   * Return the first effect that matches with 'keyword'
   */
  static EFFECT_LADSPA* object(const string& keyword) { return(dynamic_cast<EFFECT_LADSPA*>(eca_ladspa_plugin_map.object(keyword))); }

  /**
   * Return the first effect that matches with 'number'
   */
  static EFFECT_LADSPA* object(long int number) { return(dynamic_cast<EFFECT_LADSPA*>(eca_ladspa_plugin_id_map.object(kvu_numtostr(number)))); }

  /**
   * Return the matching keyword for 'object'.
   */
  static string object_identifier(const EFFECT_LADSPA* object) { return("el"); }
};

#endif
