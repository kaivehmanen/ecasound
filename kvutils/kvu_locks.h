#ifndef INCLUDED_KVU_LOCKS_H
#define INCLUDED_KVU_LOCKS_H

#include <pthread.h>

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

/**
 * A simple guarded lock wrapper for pthread_mutex_lock
 * and pthread_mutex_unlock. Lock is acquired 
 * upon object creating and released during destruction.
 */
class KVU_GUARD_LOCK {

 public:

  KVU_GUARD_LOCK(pthread_mutex_t* lock_arg);
  ~KVU_GUARD_LOCK(void);

 private:

  pthread_mutex_t* lock_repp;

  KVU_GUARD_LOCK(void) {}
  KVU_GUARD_LOCK(const KVU_GUARD_LOCK&) {}
  KVU_GUARD_LOCK& operator=(const KVU_GUARD_LOCK&) { return *this; }
};

#endif
