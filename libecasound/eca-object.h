#ifndef INCLUDED_ECA_OBJECT_H
#define INCLUDED_ECA_OBJECT_H

#include <string>

using std::string;

/**
 * Virtual class for ecasound objects
 *
 * @author Kai Vehmanen
 */
class ECA_OBJECT {

 public:

  /**
   * Object name used to identify the object type. In most 
   * cases, object name is same for all class instances.
   * Must be implemented in all subclasses.
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
