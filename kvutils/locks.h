#ifndef INCLUDED_LOCKS_H
#define INCLUDED_LOCKS_H

#include <pthread.h>

int pthread_mutex_spinlock (pthread_mutex_t *mp, long int spinlimit);

/**
 * Simple class providing atomic read and write access
 * to a single integer value. Implementation may be 
 * based on direct atomic operations or traditional 
 * locking, depending on the underlying platform.
 */
class ATOMIC_INTEGER {

 public:

  int get(void) const {
    int temp;
    pthread_mutex_lock(&mutex_rep);
    temp = value_rep;
    pthread_mutex_unlock(&mutex_rep);
    return(temp);
  }

  void set(int value) {
    pthread_mutex_lock(&mutex_rep);
    value_rep = value;
    pthread_mutex_unlock(&mutex_rep);
  }

  void add(int value) {
    pthread_mutex_lock(&mutex_rep);
    value_rep += value;
    pthread_mutex_unlock(&mutex_rep);
  }

  void subtract(int value) {
    pthread_mutex_lock(&mutex_rep);
    value_rep -= value;
    pthread_mutex_unlock(&mutex_rep);
  }

  void increment(void) {
    pthread_mutex_lock(&mutex_rep);
    ++value_rep;
    pthread_mutex_unlock(&mutex_rep);
  }

  void decrement(void) {
    pthread_mutex_lock(&mutex_rep);
    --value_rep;
    pthread_mutex_unlock(&mutex_rep);
  }

  ATOMIC_INTEGER(int value = 0) {
    pthread_mutex_init(&mutex_rep, NULL);
    set(value);
  }
 
  ~ATOMIC_INTEGER(void) {
    pthread_mutex_destroy(&mutex_rep);
  }

 private:

  mutable pthread_mutex_t mutex_rep;
  int value_rep;
};

#endif
