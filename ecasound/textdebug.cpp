#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <string>

#include <eca-debug.h>
#include "textdebug.h"

#if defined ECA_USE_NCURSES || defined ECA_USE_TERMCAP
#ifdef ECA_HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#include <ncurses/term.h>
#else
#include <curses.h>
#include <term.h>
#endif
#endif

void TEXTDEBUG::stream(std::ostream* dos) {
  dostream = dos;
}

std::ostream* TEXTDEBUG::stream(void) { return(dostream); }

void TEXTDEBUG::flush(void) {
  dostream->flush();
}

void TEXTDEBUG::control_flow(const std::string& part) {
  if ((get_debug_level() & ECA_DEBUG::module_flow) != ECA_DEBUG::module_flow) return;

#if defined ECA_USE_NCURSES || defined ECA_USE_TERMCAP
  *dostream << "- [ ";
  putp(tigetstr("bold"));
  *dostream << part;
  putp(tigetstr("sgr0"));
  *dostream << " ] ";
#else
  *dostream << "- [ " << part << " ] ";
#endif
  if (part.size() < 70)
    for (unsigned char n = 0; n < (69 - part.size()); n++) *dostream << "-";
  *dostream << "\n";
}

void TEXTDEBUG::msg(int level, const std::string& info) {
  if ((get_debug_level() & level) != level) return;
  *dostream << info << "\n";
}

TEXTDEBUG::TEXTDEBUG(void) {
  dostream = &std::cout;
}

TEXTDEBUG::~TEXTDEBUG(void) {
  flush();
}
