#ifndef _RESOURCE_FILE_H
#define _RESOURCE_FILE_H

#include <map>
#include <string>

/**
 * Class that represents a generic resource file
 */
class RESOURCE_FILE {

  map<string,string> rcmap;
  map<int,string> rc_comments;

  string res_file;
  bool changed_rep;
  bool use_equal_sign;
  int loaded_resource_values;

 public:

  bool is_modified(void) { return(changed_rep); }
  void set_modified_state(bool value) { changed_rep = value; }

  void load(void);
  void save(void);

  const string& get_resource_file(void) { return(res_file); }
  void set_resource_file(const string& file) { res_file = file; }

  const string& resource(const string& tag);
  void resource(const string& tag, const string& value);

  bool boolean_resource(const string& tag);

  RESOURCE_FILE(void) : changed_rep(false), use_equal_sign(true), loaded_resource_values(-1) { }
  ~RESOURCE_FILE(void);
};

#endif




