#ifndef INCLUDED_ECA_TEXT_H
#define INCLUDED_ECA_TEXT_H

#include <string>
#include <algorithm>
#include <iostream>

#include <eca-session.h>

class COMMAND_LINE;

void print_header(ostream* dostream);
void parse_command_line(COMMAND_LINE& cline);

void start_iactive(ECA_SESSION* param);
void start_iactive_readline(ECA_SESSION* param);

void init_readline_support(void);
char** ecasound_completion (char *text, int start, int end);
char* command_generator (char* text, int state);
char* ecasound_command_generator (char* text, int state);

#endif
