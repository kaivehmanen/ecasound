#ifndef INCLUDED_ECA_OBJECT_H
#define INCLUDED_ECA_OBJECT_H

#include <string>

/**
 * Virtual class for ecasound objects
 *
 * @author Kai Vehmanen
 */
class ECA_OBJECT {

 public:

  /**
   * Object name used to identify the object type.
   * Must be implemented in subclasses.
   */
  virtual string name(void) const = 0;

  /**
   * Object description. Description should be short, informative
   * and unformatted.
   */
  virtual string description(void) const { return(name()); }

  virtual ~ECA_OBJECT (void) { }
};

#endif
