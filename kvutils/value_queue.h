#ifndef INCLUDE_VALUE_QUEUE_H
#define INCLUDE_VALUE_QUEUE_H

#include <pthread.h>
#include <utility>
#include <deque>

using std::pair;
using std::deque;

/**
 * A thread-safe way to transmit int-double pairs.
 * @author Kai Vehmanen
 */
class VALUE_QUEUE {
    
 private:

  mutable pthread_mutex_t lock_rep;     // mutex ensuring exclusive access to buffer
  mutable pthread_cond_t cond_rep;

  pair<int,double> empty_rep;
  deque<pair<int,double> > cmds_rep;

public:
  /**
   * Adds a new item to the end of the queue.
   */
  void push_back(int key, double value);

  /**
   * Removes the first item.
   *
   * require:
   *   is_empty() == false
   */
  void pop_front(void);

  /**
   * Returns the first item.
   *
   * require:
   *   is_empty() == false
   */
  const pair<int,double>& front(void);

  /**
   * Blocks until 'is_empty() != true'. 'timeout_sec' and
   * 'timeout_usec' specify the upper time limit for blocking.
   */
  void poll(int timeout_sec, long int timeout_usec);

  /**
   * Is queue empty?
   *
   * require:
   *   is_empty() == false
   */
  bool is_empty(void) const;
  
  VALUE_QUEUE(void);
};

#endif
