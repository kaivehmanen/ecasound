#ifndef _ECA_QTMAIN_H
#define _ECA_QTMAIN_H

#include "eca-session.h"

class COMMAND_LINE;

int main(int argc, char **argv);

void parse_command_line(COMMAND_LINE& cline);
void signal_handler(int signum);

#endif
