#ifndef _AUDIOFX_MIXING_H
#define _AUDIOFX_MIXING_H

#include <vector>

#include "samplebuffer_iterators.h"
#include "audiofx.h"

/**
 * Virtual base class for channel mixing and routing effects
 * @author Kai Vehmanen
 */
class EFFECT_MIXING : public EFFECT_BASE {
 public:
  typedef std::vector<parameter_type>::size_type ch_type;

  virtual ~EFFECT_MIXING(void) { }
};

/**
 * Channel copy (one-to-one copy)
 * @author Kai Vehmanen
 */
class EFFECT_CHANNEL_COPY : public EFFECT_MIXING {

private:

  ch_type from_channel, to_channel;
  SAMPLE_ITERATOR_CHANNEL f_iter, t_iter;

public:

  virtual std::string name(void) const { return("Channel copy"); }
  virtual std::string parameter_names(void) const { return("from-channel,to-channel"); }

  int output_channels(int i_channels) const;

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd);
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_CHANNEL_COPY* clone(void) const { return new EFFECT_CHANNEL_COPY(*this); }
  EFFECT_CHANNEL_COPY* new_expr(void) const { return new EFFECT_CHANNEL_COPY(); }
  EFFECT_CHANNEL_COPY (parameter_type from_channel = 1.0, parameter_type to_channel = 1.0);
};

/**
 * Channel copy (one-to-one copy)
 * @author Kai Vehmanen
 */
class EFFECT_MIX_TO_CHANNEL : public EFFECT_MIXING {

private:

  typedef std::vector<parameter_type>::size_type ch_type;

  int channels;
  ch_type to_channel;
  parameter_type sum;

  SAMPLE_ITERATOR_CHANNEL t_iter;
  SAMPLE_ITERATOR_INTERLEAVED i;

public:

  virtual std::string name(void) const { return("Mix to channel"); }
  virtual std::string parameter_names(void) const { return("to-channel"); }

  int output_channels(int i_channels) const;

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd);
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_MIX_TO_CHANNEL* clone(void) const { return new EFFECT_MIX_TO_CHANNEL(*this); }
  EFFECT_MIX_TO_CHANNEL* new_expr(void) const { return new EFFECT_MIX_TO_CHANNEL(); }
  EFFECT_MIX_TO_CHANNEL (parameter_type to_channel = 1.0);
};

#endif
