#include "../config.h"
#include <iostream>
#include <string>

#include <eca-debug.h>

#include "textdebug.h"

#include <curses.h>
#include <term.h>

void TEXTDEBUG::stream(ostream* dos) {
  dostream = dos;
}

ostream* TEXTDEBUG::stream(void) { return(dostream); }

void TEXTDEBUG::flush(void) {
  dostream->flush();
}

void TEXTDEBUG::control_flow(const string& part) {
  if ((get_debug_level() & ECA_DEBUG::module_flow) != ECA_DEBUG::module_flow) return;

#ifdef USE_NCURSES
  *dostream << "- [ ";
  putp(tigetstr("bold"));
  *dostream << part;
  putp(tigetstr("rmso"));
  *dostream << " ] ";
#else
  *dostream << "- [ " << part << " ] ";
#endif
  for (unsigned char n = 0; n < (69 - part.size()); n++) *dostream << "-";
  *dostream << "\n";
}

void TEXTDEBUG::msg(int level, const string& info) {
  if ((get_debug_level() & level) != level) return;
  *dostream << info << "\n";
}

TEXTDEBUG::TEXTDEBUG(void) {
  dostream = &cout;
}

TEXTDEBUG::~TEXTDEBUG(void) {
  flush();
}
