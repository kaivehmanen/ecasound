#ifndef INCLUDED_QE_MAIN_H
#define INCLUDED_QE_MAIN_H

#include "eca-session.h"

class COMMAND_LINE;

int main(int argc, char **argv);

void parse_command_line(COMMAND_LINE& cline);
void signal_handler(int signum);

#endif
