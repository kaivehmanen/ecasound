#ifndef INCLUDED_AUDIOFX_H
#define INCLUDED_AUDIOFX_H

#include "eca-chainop.h"
#include "eca-samplerate-aware.h"

/**
 * Virtual base for all audio effect classes.
 *
 * @author Kai Vehmanen
 */
class EFFECT_BASE : public CHAIN_OPERATOR, 
		    public ECA_SAMPLERATE_AWARE
{

public:

  /** @name Constructors and destructors */
  /*@{*/

  EFFECT_BASE(void);
  virtual ~EFFECT_BASE(void);

  /*@}*/

 protected:

  /** @name Protected functions for storing/retrieving channel count */
  /*@{*/

  int channels(void) const;
  void set_channels(int v);

  /*@}*/

 public:

  /** @name Public virtual functions to notify about changes 
   *        (Reimplemented from ECA_RESAMPLE_AWARE) */
  /*@{*/

  virtual void set_samples_per_second(SAMPLE_SPECS::sample_rate_t v);

  /*@}*/

  /** @name Public virtual functions for initialization
   *        (Reimplemented from CHAIN_OPERATOR) */
  /*@{*/

  virtual void init(SAMPLE_BUFFER* sbuf);

  /*@}*/

 private:

  int channels_rep;
};

#endif
