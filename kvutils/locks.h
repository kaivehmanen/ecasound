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

  int get(void) const;
  void set(int value);
  void add(int value);
  void subtract(int value);
  void increment(void);
  void decrement(void);

  ATOMIC_INTEGER(int value = 0);
  ~ATOMIC_INTEGER(void);

 private:

  void* value_repp;
  mutable void* mutex_repp;
  int value_rep;
};

#endif
