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
  string label_rep, unique_rep, param_names_rep;
  vector<LADSPA_Data> params;

public:

  string name(void) const { return("LADSPA/" + label_rep); }
  string parameter_names(void) const { return(param_names_rep); }
  string unique(void) const { return(unique_rep); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_LADSPA* clone(void);
  EFFECT_LADSPA* new_expr(void)  { return new EFFECT_LADSPA(plugin_desc); }
  EFFECT_LADSPA (const LADSPA_Descriptor *plugin_desc = 0) throw(ECA_ERROR*);
  ~EFFECT_LADSPA (void);

 private:

  EFFECT_LADSPA (const EFFECT_LADSPA& x) { }
  EFFECT_LADSPA& operator=(const EFFECT_LADSPA& x) { return *this; }
};

#endif
