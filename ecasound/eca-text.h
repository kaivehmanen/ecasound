#ifndef _ECA_TEXT_H
#define _ECA_TEXT_H

#include <string>
#include <algorithm>

#include <eca-session.h>

string print_header(void);
void signal_handler(int signum);

void start_iactive(ECA_SESSION* param);
void start_iactive_readline(ECA_SESSION* param);

void init_readline_support(void);
char** ecasound_completion (char *text, int start, int end);
char* command_generator (char* text, int state);
char* ecasound_command_generator (char* text, int state);

#endif
