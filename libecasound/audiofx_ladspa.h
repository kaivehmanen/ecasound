#ifndef INCLUDED_AUDIOFX_LADSPA_H
#define INCLUDED_AUDIOFX_LADSPA_H

#include <vector>
#include <string>

#include "audiofx.h"

#ifdef HAVE_LADSPA_H
#include <ladspa.h>
#else
#include "ladspa.h"
#endif

class SAMPLE_BUFFER;

/**
 * Wrapper class for LADSPA plugins
 * @author Kai Vehmanen
 */
class EFFECT_LADSPA : public EFFECT_BASE {

private:

  SAMPLE_BUFFER* buffer_repp;
  
  const LADSPA_Descriptor *plugin_desc;
  std::vector<LADSPA_Handle> plugins_rep;

  unsigned long port_count_rep;
  int in_audio_ports;
  int out_audio_ports;
  long unique_number_rep;
  std::string name_rep, unique_rep, param_names_rep;
  std::vector<LADSPA_Data> params;

  void init_ports(void);

public:

  virtual std::string name(void) const { return(name_rep); }
  virtual std::string parameter_names(void) const { return(param_names_rep); }

  /**
   * This identifier can be used as a unique, case-sensitive
   * identifier for the plugin type within the plugin file. 
   * Labels must not contain white-space characters. 
   */
  std::string unique(void) const { return(unique_rep); }

  /**
   * This numeric identifier indicates the plugin type
   * uniquely. Plugin programmers may reserve ranges of IDs from a
   * central body to avoid clashes. Hosts may assume that IDs are
   * below 0x1000000. 
   */
  long int unique_number(void) const { return(unique_number_rep); }

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd);
  virtual void set_parameter(int param, parameter_t value);
  virtual parameter_t get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

  EFFECT_LADSPA* clone(void) const;
  EFFECT_LADSPA* new_expr(void) const { return new EFFECT_LADSPA(plugin_desc); }
  EFFECT_LADSPA (const LADSPA_Descriptor *plugin_desc = 0) throw(ECA_ERROR&);
  ~EFFECT_LADSPA (void);

 private:

  EFFECT_LADSPA (const EFFECT_LADSPA& x) { }
  EFFECT_LADSPA& operator=(const EFFECT_LADSPA& x) { return *this; }
};

#endif
