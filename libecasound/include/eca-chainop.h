#ifndef _CHAINOP_H
#define _CHAINOP_H

#include <map>
#include <string>

#include "dynamic-object.h"
#include "samplebuffer.h"

/**
 * Virtual base class for chain operators. 
 * @author Kai Vehmanen
 */
class CHAIN_OPERATOR : public DYNAMIC_OBJECT {

 public:

  /**
   * Prepare chain operator for processing.
   */
  virtual void init(SAMPLE_BUFFER* sbuf) = 0;

  /**
   * Process sample data
   * @param sbuf pointer to a sample buffer object
   */
  virtual void process(void) = 0;

  /**
   * Optional status info.
   * @param single_sample pointer to a single sample
   */
  virtual string status(void) const { return(""); }

  /** 
   * If sample buffer used for initializing has 'i_channels' audio 
   * channels, this routine returns the number of output channels
   * produced. Must be reimplemented if channel count changes 
   * during processing.
   */
  virtual int output_channels(int i_channels) const { return(i_channels); }

  /**
   * Virtual method that clones the current object and returns 
   * a pointer to it. This must be implemented by all subclasses!
   */
  virtual CHAIN_OPERATOR* clone(void) = 0;

  /**
   * Virtual destructor.
   */
  virtual ~CHAIN_OPERATOR (void) { }
};

#endif
