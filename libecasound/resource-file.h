#ifndef INCLUDED_RESOURCE_FILE_H
#define INCLUDED_RESOURCE_FILE_H

#include <vector>
#include <map>
#include <string>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef USE_CXX_STD_NAMESPACE
using namespace std;
#endif

/**
 * Generic resource file class
 */
class RESOURCE_FILE {

  string resfile_rep;
  mutable map<string,string> resmap_rep;
  vector<string> lines_rep;
  bool modified_rep;

 public:

  /**
   * Returns a vector of registered presets
   */
  vector<string> keywords(void) const;

  /**
   * Returns current resource file name.
   */
  const string& resource_file(void) const { return(resfile_rep); }

  /**
   * Set resource file name.
   */
  void resource_file(const string& v) { resfile_rep = v; }

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

  /**
   * Load/restore resources from file
   */
  void load(void);

  /**
   * Save/store resources to file saving
   */
  void save(void);

  /**
   * Constructor. Resource values are read, if
   * filename argument is given.
   */
  RESOURCE_FILE(const string& resource_file = "");
  virtual ~RESOURCE_FILE(void);
};

#endif
