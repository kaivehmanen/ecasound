#ifndef INCLUDED_LOCKS_H
#define INCLUDED_LOCKS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_ASM_ATOMIC
#include <asm/atomic.h>
#endif
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
#ifdef USE_ASM_ATOMIC
    return(atomic_read(&value_rep));
#else
    int temp;
    pthread_mutex_lock(&mutex_rep);
    temp = value_rep;
    pthread_mutex_unlock(&mutex_rep);
    return(temp);
#endif
  }

  void set(int value) {
#ifdef USE_ASM_ATOMIC
    atomic_set(&value_rep, value);
#else
    pthread_mutex_lock(&mutex_rep);
    value_rep = value;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  void add(int value) {
#ifdef USE_ASM_ATOMIC
    atomic_add(value, &value_rep);
#else
    pthread_mutex_lock(&mutex_rep);
    value_rep += value;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  void subtract(int value) {
#ifdef USE_ASM_ATOMIC
    atomic_sub(value, &value_rep);
#else
    pthread_mutex_lock(&mutex_rep);
    value_rep -= value;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  void increment(void) {
#ifdef USE_ASM_ATOMIC
    atomic_inc(&value_rep);
#else
    pthread_mutex_lock(&mutex_rep);
    ++value_rep;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  void decrement(void) {
#ifdef USE_ASM_ATOMIC
    atomic_dec(&value_rep);
#else
    pthread_mutex_lock(&mutex_rep);
    --value_rep;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  ATOMIC_INTEGER(int value = 0) {
#ifdef USE_ASM_ATOMIC
    atomic_set(&value_rep, value);
#else
    pthread_mutex_init(&mutex_rep, NULL);
    set(value);
#endif
  }
 
  ~ATOMIC_INTEGER(void) {
#ifdef USE_ASM_ATOMIC
#else
    pthread_mutex_destroy(&mutex_rep);
#endif
  }

 private:

#ifdef USE_ASM_ATOMIC
  atomic_t value_rep;
#else
  mutable pthread_mutex_t mutex_rep;
  int value_rep;
#endif
};

#endif
