#ifndef INCLUDED_ECA_TEXT_H
#define INCLUDED_ECA_TEXT_H

#if RL_READLINE_VERSION >= 0x0402
char** ecasound_completion (const char *text, int start, int end);
char* ecasound_command_generator (const char* text, int state);
#else 
char** ecasound_completion (char *text, int start, int end);
char* ecasound_command_generator (char* text, int state);
#endif

#endif
