#include <iostream>
#include <string>
#include <iomanip>

#include <eca-debug.h>

#include "textdebug.h"

void TEXTDEBUG::stream(ostream* dos) {
  dostream = dos;
}

ostream* TEXTDEBUG::stream(void) { return(dostream); }

void TEXTDEBUG::flush(void) {
  dostream->flush();
}

void TEXTDEBUG::set_debug_level(int level) {
  debug_level = level;
}

void TEXTDEBUG::control_flow(const string& part) {
  if (is_enabled() == false) return;
  
  *dostream << "- [[1m " << part << "[0m ] ";
  for (unsigned char n = 0; n < (69 - part.size()); n++)
    *dostream << '-';
  *dostream << "\n";
}

void TEXTDEBUG::msg(const string& info) {
  if (is_enabled() == false) return;
  *dostream << info << "\n";
}

void TEXTDEBUG::msg(int level, const string& info) {
  if (is_enabled() == false) return;
  if (debug_level < level) return;
  if (debug_level != 0) *dostream << "DEBUG: ";
  *dostream << info << "\n";
}

TEXTDEBUG::TEXTDEBUG(void) {
  dostream = &cout;
  debug_level = 0;
  enable();
}















