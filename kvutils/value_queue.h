#ifndef _VALUE_QUEUE_H
#define _VALUE_QUEUE_H

#include <pthread.h>
#include <utility>
#include <deque>

/**
 * A thread-safe way to transmit int-double pairs.
 * @author Kai Vehmanen
 */
class VALUE_QUEUE {
    
 private:
  pthread_mutex_t lock;     // mutex ensuring exclusive access to buffer
  
  deque<pair<int,double> > cmds;

public:
  /**
   * Add a new item to the end of the queue.
   */
  void push_back(int key, double value);

  /**
   * Remove the first item.
   *
   * require:
   *   is_empty() == false
   */
  void pop_front(void);

  /**
   * Return the first item.
   *
   * require:
   *   is_empty() == false
   */
  const pair<int,double>& front(void);

  /**
   * Is queue empty?
   *
   * require:
   *   is_empty() == false
   */
  bool is_empty(void) const;
  
  VALUE_QUEUE(void) { 
    pthread_mutex_init(&lock, NULL);
  }
};

#endif

