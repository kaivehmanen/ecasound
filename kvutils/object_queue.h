#ifndef _OBJECT_QUEUE_H
#define _OBJECT_QUEUE_H

#include <pthread.h>
#include <string>
#include <deque>

extern pthread_mutex_t _cq_lock;

/**
 * Thread-safe way to transmit generic objects
 */
template<class C>
class OBJECT_QUEUE {
    
private:

    deque<C> objs;

public:
    
    /**
     * Inserts 'cmd' into the queue
     */
    void push_back(const C& obj) {
      pthread_mutex_lock(&_cq_lock);
      objs.push_back(obj);
      pthread_mutex_unlock(&_cq_lock);
    }

    /**
     * Pops the first item
     */
    void pop_front(void) {
      pthread_mutex_lock(&_cq_lock);
      objs.pop_front();
      pthread_mutex_unlock(&_cq_lock);
    }

    /**
     * Returns the first item
     */
    C& front(void) const {
      assert(objs.size() > 0);
      return(objs.front());
    }

    /**
     * Returns true if at least one command is available
     */
    bool objs_available(void) const {
      if (pthread_mutex_trylock(&_cq_lock) != 0)
        return(false);
      bool temp = false;
      if (objs.size() > 0) temp = true;
      pthread_mutex_unlock(&_cq_lock);
      return(temp);
    }
    
    /**
     * Flush all items
     */
    void flush(void) {
      int n = pthread_mutex_trylock(&_cq_lock);
      if (n != 0) return;
      while (objs.size() > 0) objs.pop_front();
      pthread_mutex_unlock(&_cq_lock);
    }

    OBJECT_QUEUE(void) { _cq_lock = PTHREAD_MUTEX_INITIALIZER; }
};

#endif
