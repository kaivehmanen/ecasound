#ifndef _COMMAND_QUEUE_H
#define _COMMAND_QUEUE_H

#include <pthread.h>
#include <string>
#include <deque>

extern pthread_mutex_t _cq_lock;

class COMMAND_QUEUE {
    
private:
           // mutex ensuring exclusive access to buffer

    deque<string> cmds;

public:
    void push_back(const string& cmd);
    void pop_front(void);
    string front(void) const;
    bool cmds_available(void);
    void flush(void);

    COMMAND_QUEUE(void) { }
};

#endif






