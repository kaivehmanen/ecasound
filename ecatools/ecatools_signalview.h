#ifndef INCLUDED_ECATOOLS_SIGNALVIEW_H
#define INCLUDED_ECATOOLS_SIGNALVIEW_H

int main(int argc, char *argv[]);
void parse_command_line(int argc, char *argv[]);
void print_usage(void);
void signal_handler(int signum);

const string ecatools_signalview_version = "20001124";

#endif
