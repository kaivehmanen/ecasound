#ifndef _COMMAND_QUEUE_H
#define _COMMAND_QUEUE_H

#include <pthread.h>
#include <string>
#include <deque>

extern pthread_mutex_t _cq_lock;

/**
 * Thread-safe way to transmit string objects.
 */

class COMMAND_QUEUE {
    
private:

    deque<string> cmds;

public:
    
    /**
     * Inserts 'cmd' into the queue
     */
    void push_back(const string& cmd);

    /**
     * Pops the first item
     */
    void pop_front(void);

    /**
     * Returns the first item
     */
    string front(void) const;

    /**
     * Returns true if at least one command is available
     */
    bool cmds_available(void);
    
    /**
     * Flush all items
     */
    void flush(void);

    COMMAND_QUEUE(void) { }
};

#endif






