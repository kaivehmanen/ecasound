#ifndef _AUDIOFX_LADSPA_H
#define _AUDIOFX_LADSPA_H

#include <vector>

#include "samplebuffer.h"
#include "audiofx.h"
#include "ladspa.h"

/**
 * Wrapper class for LADSPA plugins
 * @author Kai Vehmanen
 */
class EFFECT_LADSPA : public EFFECT_BASE {

private:

  SAMPLE_BUFFER* buffer;
  
  const LADSPA_Descriptor *plugin_desc;
  vector<LADSPA_Handle> plugins;

  unsigned long port_count_rep;
  int in_audio_ports;
  int out_audio_ports;
  long unique_number_rep;
  string label_rep, unique_rep, param_names_rep;
  vector<LADSPA_Data> params;

public:

  virtual string name(void) const { return("LADSPA/" + label_rep); }
  virtual string description(void) const { return("Wrapper for LADSPA plugins."); }

  virtual string parameter_names(void) const { return(param_names_rep); }

  /**
   * This identifier can be used as a unique, case-sensitive
   * identifier for the plugin type within the plugin file. 
   * Labels must not contain white-space characters. 
   */
  string unique(void) const { return(unique_rep); }

  /**
   * This numeric identifier indicates the plugin type
   * uniquely. Plugin programmers may reserve ranges of IDs from a
   * central body to avoid clashes. Hosts may assume that IDs are
   * below 0x1000000. 
   */
  long int unique_number(void) const { return(unique_number_rep); }

  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

  EFFECT_LADSPA* clone(void);
  EFFECT_LADSPA* new_expr(void)  { return new EFFECT_LADSPA(plugin_desc); }
  EFFECT_LADSPA (const LADSPA_Descriptor *plugin_desc = 0) throw(ECA_ERROR*);
  ~EFFECT_LADSPA (void);

 private:

  EFFECT_LADSPA (const EFFECT_LADSPA& x) { }
  EFFECT_LADSPA& operator=(const EFFECT_LADSPA& x) { return *this; }
};

#endif
