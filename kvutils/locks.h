#ifndef INCLUDED_LOCKS_H
#define INCLUDED_LOCKS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ASM_ATOMIC_H
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
#ifdef HAVE_ASM_ATOMIC_H
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
#ifdef HAVE_ASM_ATOMIC_H
    atomic_set(&value_rep, value);
#else
    pthread_mutex_lock(&mutex_rep);
    value_rep = value;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  void add(int value) {
#ifdef HAVE_ASM_ATOMIC_H
    atomic_add(value, &value_rep);
#else
    pthread_mutex_lock(&mutex_rep);
    value_rep += value;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  void subtract(int value) {
#ifdef HAVE_ASM_ATOMIC_H
    atomic_sub(value, &value_rep);
#else
    pthread_mutex_lock(&mutex_rep);
    value_rep -= value;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  void increment(void) {
#ifdef HAVE_ASM_ATOMIC_H
    atomic_inc(&value_rep);
#else
    pthread_mutex_lock(&mutex_rep);
    ++value_rep;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  void decrement(void) {
#ifdef HAVE_ASM_ATOMIC_H
    atomic_dec(&value_rep);
#else
    pthread_mutex_lock(&mutex_rep);
    --value_rep;
    pthread_mutex_unlock(&mutex_rep);
#endif
  }

  ATOMIC_INTEGER(int value = 0) {
#ifdef HAVE_ASM_ATOMIC_H
    atomic_set(&value_rep, value);
#else
    pthread_mutex_init(&mutex_rep, NULL);
    set(value);
#endif
  }
 
  ~ATOMIC_INTEGER(void) {
#ifdef HAVE_ASM_ATOMIC_H
#else
    pthread_mutex_destroy(&mutex_rep);
#endif
  }

 private:

#ifdef HAVE_ASM_ATOMIC_H
  atomic_t value_rep;
#else
  mutable pthread_mutex_t mutex_rep;
  int value_rep;
#endif
};

#endif
