#ifndef INCLUDED_AUDIOFX_LV2_H
#define INCLUDED_AUDIOFX_LV2_H

#include <vector>
#include <string>

#include "audiofx.h"

#if ECA_USE_LIBLILV

#include <lilv/lilvmm.hpp>

class SAMPLE_BUFFER;

/**
 * Wrapper class for LV2 plugins
 * @author Jeremy Salwen
 */
class EFFECT_LV2 : public EFFECT_BASE {

public:

  EFFECT_LV2 (Lilv::Plugin plugin_d) throw(ECA_ERROR&);
  virtual ~EFFECT_LV2(void);

  EFFECT_LV2* clone(void) const;
  EFFECT_LV2* new_expr(void) const { return new EFFECT_LV2(plugin_desc); }

  virtual std::string name(void) const { return(name_rep); }
  virtual std::string description(void) const;
  virtual std::string parameter_names(void) const { return(param_names_rep); }

  /**
   * This identifier can be used as a unique, case-sensitive
   * identifier for the plugin type within the plugin file. 
   * Labels must not contain white-space characters. 
   */
  std::string unique(void) const { return(unique_rep); }

  virtual int output_channels(int i_channels) const;

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd) const;
  virtual void set_parameter(int param, parameter_t value);
  virtual parameter_t get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void release(void);
  virtual void process(void);

 private:

  EFFECT_LV2(const EFFECT_LV2& x):plugin_desc(x.plugin_desc) { }
  EFFECT_LV2& operator=(const EFFECT_LV2& x) { return *this; }

private:

  SAMPLE_BUFFER* buffer_repp;
  
  Lilv::Plugin plugin_desc;
  std::vector<Lilv::Instance> plugins_rep;

  unsigned long port_count_rep;
  int in_audio_ports;
  int out_audio_ports;
  std::string name_rep, maker_rep, unique_rep, param_names_rep;
  std::vector<float> params;
  std::vector<struct PARAM_DESCRIPTION> param_descs_rep;

  void init_ports(void) throw(ECA_ERROR&);
  void parse_parameter_hint_information(const Lilv::Plugin  plugin, Lilv::Port p, struct PARAM_DESCRIPTION *pd);
};

#endif /* ECA_USE_LIBLILV */
#endif
