#ifndef _DYNAMIC_OBJECT_H
#define _DYNAMIC_OBJECT_H

#include <string>

#include "dynamic-parameters.h"
#include "eca-object.h"

/**
 * Virtual class for objects supporting dynamic parameter
 * control.
 *
 * @author Kai Vehmanen
 */
template<class T>
class DYNAMIC_OBJECT : public DYNAMIC_PARAMETERS<T>,
                       public ECA_OBJECT {

 public:

  /**
   * Virtual method that clones the current object and returns 
   * a pointer to it. This must be implemented by all subclasses!
   */
  virtual DYNAMIC_OBJECT<T>* clone(void) = 0;

  /**
   * Virtual method that creates a new object of current type.
   * This must be implemented by all subclasses!
   */
  virtual DYNAMIC_OBJECT<T>* new_expr(void) = 0;

  virtual ~DYNAMIC_OBJECT (void) { }
};

#endif
