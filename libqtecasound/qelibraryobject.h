#ifndef _QELIBRARYOBJECT_H
#define _QELIBRARYOBJECT_H

#include <pthread.h>

/**
 * Base class for combined ecasound and Qt library objects
 */
class QELibraryObject {

 protected:

  /**
   * Locks the object from access by other threads
   *
   * require:
   *  object_not_locked
   *
   * ensure:
   *  object_locked
   */
  void lock_object(void) {
    pthread_mutex_lock(&thread_lock);
  }

  /**
   * Tries to lock to object
   *
   * require:
   *  object_not_locked
   *
   * ensure:
   *  result == true implies object_locked
   *  result == false implies object_not_locked
   */
  bool try_lock_object(void) {
    return(pthread_mutex_trylock(&thread_lock));
  }

  /**
   * Unlocks the object
   *
   * require:
   *  object_locked
   *
   * ensure:
   *  object_not_locked
   */
  void unlock_object(void) {
    pthread_mutex_unlock(&thread_lock);
  }

 public:

  /**
   * Contructor
   */
  QELibraryObject(void) {
    pthread_mutex_init(&thread_lock, 0);
  }

 private:

  pthread_mutex_t thread_lock;
};

#endif
