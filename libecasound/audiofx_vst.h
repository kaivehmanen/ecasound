#ifndef INCLUDE_AUDIOFX_VST_H
#define INCLUDE_AUDIOFX_VST_H

#include <string>
#include <vector>

#include "samplebuffer.h"
#include "audiofx.h"

#ifdef FEELING_EXPERIMENTAL
#include "aeffectx.h"

typedef AEffect* (*VST_plugin_descriptor)(audioMasterCallback audioMaster);

long vst_audiomaster(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

/**
 * Wrapper class for VST1.0/2.0 plugins
 * @author Kai Vehmanen
 */
class EFFECT_VST : public EFFECT_BASE {

public:

  EFFECT_VST (const std::string& fname) throw(ECA_ERROR&);
  virtual ~EFFECT_VST (void);

  EFFECT_VST* clone(void)  { return new EFFECT_VST(library_file_rep); }
  EFFECT_VST* new_expr(void)  { return new EFFECT_VST(library_file_rep); }

  virtual virtual std::string name(void) const { return("VST/" + label_rep); }
  virtual std::string description(void) const { return("Wrapper for VST-plugins."); }
  virtual virtual std::string parameter_names(void) const { return(param_names_rep); }

  std::string unique(void) const { return(unique_rep); }

  virtual void set_parameter(int param, parameter_t value);
  virtual parameter_t get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

 private:

  EFFECT_VST (const EFFECT_VST& x) { }
  EFFECT_VST& operator=(const EFFECT_VST& x) { return *this; }

private:

  SAMPLE_BUFFER* buffer;

  VST_plugin_descriptor master_func;
  std::vector<AEffect*> vst_handles;
  std::string label_rep, unique_rep, param_names_rep, library_file_rep;
};

#endif // experimental
#endif
