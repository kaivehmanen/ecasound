#ifndef _FILE_PRESET_H
#define _FILE_PRESET_H

#include "preset.h"

/**
 * File based effect preset
 *
 * @author Kai Vehmanen
 */
class FILE_PRESET : public PRESET {

  string filename_rep;

 public:

  string filename(void) const { return(filename_rep); }
  void set_filename(const string& v) { filename_rep = v; }

  virtual FILE_PRESET* clone(void) { return(new FILE_PRESET(*this)); }
  virtual FILE_PRESET* new_expr(void) { return(new FILE_PRESET()); }
  virtual ~FILE_PRESET (void) { }

  FILE_PRESET(const string& file_name = "");
};

#endif
