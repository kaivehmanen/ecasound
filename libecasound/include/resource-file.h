#ifndef _RESOURCE_FILE_H
#define _RESOURCE_FILE_H

#include <vector>
#include <string>

/**
 * Generic resource file class
 */
class RESOURCE_FILE {

  string res_file;

 public:

  /**
   * Returns a vector of registered presets
   */
  vector<string> keywords(void) const;

  /**
   * Returns current resource file name.
   */
  const string& resource_file(void) const { return(res_file); }

  /**
   * Returns value of resource 'tag'.
   */
  string resource(const string& tag) const;

  /**
   * Set resource 'tag' value to 'value'. If value wasn't 
   * previously defined, it's added.
   */
  void resource(const string& tag, const string& value);

  /**
   * Returns true if resource 'tag' is 'true', otherwise false
   */
  bool boolean_resource(const string& tag) const;
  
  /**
   * Whether resource 'tag' is specified in the resource file
   */
  bool has(const string& tag) const;

  RESOURCE_FILE(const string& resource_file) : res_file(resource_file) { }
  ~RESOURCE_FILE(void);
};

#endif




