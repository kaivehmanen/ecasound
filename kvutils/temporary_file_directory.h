#ifndef INCLUDED_TEMPORARY_FILE_DIRECTORY
#define INCLUDED_TEMPORARY_FILE_DIRECTORY

#include <string>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef USE_CXX_STD_NAMESPACE
using std::string;
#endif

/**
 * Provides services for allocating and reserving 
 * secure, temporary directories.
 * 
 * @author Kai Vehmanen
 */
class TEMPORARY_FILE_DIRECTORY {

 public:

  static const int max_temp_files = 512;

  TEMPORARY_FILE_DIRECTORY(void);
  TEMPORARY_FILE_DIRECTORY(const string& dir);
  ~TEMPORARY_FILE_DIRECTORY(void);

  void set_directory_prefix(const string& dir);
  void reserve_directory(const string& nspace);
  void release_directory(void);

  string get_directory_prefix(void) const;
  string get_reserved_directory(void) const;
  string create_filename(const string& prefix, const string& postfix);
  bool is_valid(void) const;

 private:

  void check_validity(void);

  string tdir_rep;
  string dirprefix_rep;
  int tmp_index_rep;
  bool valid_rep;
};

#endif
