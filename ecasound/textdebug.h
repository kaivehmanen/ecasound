#ifndef _TEXTDEBUG_H
#define _TEXTDEBUG_H

#include <string>
#include <iostream.h>

#include <eca-debug.h>

class TEXTDEBUG : public ECA_DEBUG {
private:
    
    ostream* dostream;
    int debug_level;
    
    void stream(ostream* dos);
    ostream* stream(void);
    
public:

    void flush(void);
    void control_flow(const string& part);
    void msg(int level, const string& info);

    TEXTDEBUG(void);
};

extern TEXTDEBUG textdebug;

#endif














