#ifndef _TEXTDEBUG_H
#define _TEXTDEBUG_H

#include <string>
#include <iostream.h>

#include <eca-debug.h>

class TEXTDEBUG : public MAINDEBUG {
private:
    
    ostream* dostream;

    int debug_level;
    
    void stream(ostream* dos);
    ostream* stream(void);
    
public:

    void flush(void);
    void set_debug_level(int level);
    int get_debug_level(void) { return(debug_level); }

    TEXTDEBUG(void);

    void control_flow(const string& part);
    void msg(const string& info);
    void msg(int level, const string& info);
};

extern TEXTDEBUG textdebug;

#endif














