#ifndef INCLUDED_TEXTDEBUG_H
#define INCLUDED_TEXTDEBUG_H

#include <string>
#include <iostream>

#include <eca-debug.h>

class TEXTDEBUG : public ECA_DEBUG {
private:
    
    std::ostream* dostream;
    int debug_level;
    
    void stream(std::ostream* dos);
    std::ostream* stream(void);
    
public:

    void flush(void);
    void control_flow(const std::string& part);
    void msg(int level, const std::string& infoarg);

    TEXTDEBUG(void);
    ~TEXTDEBUG(void);
};

extern TEXTDEBUG textdebug;

#endif
