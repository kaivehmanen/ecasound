#ifndef INCLUDED_ECA_TEXT_H
#define INCLUDED_ECA_TEXT_H

char** ecasound_completion (char *text, int start, int end);
char* ecasound_command_generator (char* text, int state);

#endif
