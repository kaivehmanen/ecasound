#ifndef INCLUDED_CHAINOP_H
#define INCLUDED_CHAINOP_H

#include <map>
#include <string>

#include "eca-operator.h"
#include "eca-audio-format.h"
#include "samplebuffer.h"

/**
 * Virtual base class for chain operators. 
 * @author Kai Vehmanen
 */
class CHAIN_OPERATOR : public OPERATOR {

 public:

  typedef SAMPLE_SPECS::sample_type parameter_type;

  /**
   * Prepare chain operator for processing. All following 
   * calls will use the sample buffer pointed by 'sbuf'.
   * It's important that buffer parameters are not changed
   * after this call (especially channel count and sample rate).
   *
   * @param sbuf pointer to a sample buffer object
   */
  virtual void init(SAMPLE_BUFFER* sbuf) = 0;

  /**
   * Process sample data
   */
  virtual void process(void) = 0;

  /**
   * Optional status info.
   * @param single_sample pointer to a single sample
   */
  virtual std::string status(void) const { return(""); }

  /** 
   * If chain operator regularly adds or removes samples from 
   * the input data stream, this function should be 
   * reimplemented.
   */
  virtual long int output_samples(long int i_samples) const { return(i_samples); }

  /** 
   * If sample buffer used for initializing has 'i_channels' audio 
   * channels, this routine returns the number of output channels
   * produced. Must be reimplemented if channel count changes 
   * during processing.
   */
  virtual int output_channels(int i_channels) const { return(i_channels); }

  /**
   * Virtual destructor.
   */
  virtual ~CHAIN_OPERATOR (void) { }
};

#endif
