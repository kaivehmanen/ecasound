#ifndef INCLUDED_AUDIOFX_H
#define INCLUDED_AUDIOFX_H

#include "eca-chainop.h"

/**
 * Virtual base for all audio effect classes.
 *
 * @author Kai Vehmanen
 */
class EFFECT_BASE : public CHAIN_OPERATOR {

 private:

  long int srate_rep;
  int channels_rep;

 protected:

  long int samples_per_second(void) const;
  int channels(void) const;

  void set_samples_per_second(long int v);
  void set_channels(int v);

 public:

  EFFECT_BASE(void);
  virtual ~EFFECT_BASE(void);
};

#endif
