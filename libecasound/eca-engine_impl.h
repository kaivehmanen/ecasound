#ifndef INCLUDED_ECA_ENGINE_IMPL_H
#define INCLUDED_ECA_ENGINE_IMPL_H

#include <ctime>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include <kvutils/value_queue.h>
#include <kvutils/procedure_timer.h>

#include "eca-chainsetup.h"

/**
 * Private class used in ECA_ENGINE 
 * implementation.
 */
class ECA_ENGINE_impl {

  friend ECA_ENGINE;

 private:


  PROCEDURE_TIMER looptimer_faster_than_rt_rep;
  PROCEDURE_TIMER looptimer_slower_than_rt_rep;
  PROCEDURE_TIMER looptimer_exceeds_buffering_rep;

  VALUE_QUEUE command_queue_rep;

  pthread_cond_t ecasound_stop_cond_repp;
  pthread_mutex_t ecasound_stop_mutex_repp;

  struct timeval multitrack_input_stamp_rep;

  ECA_CHAINSETUP::Mix_mode mixmode_rep;
};

#endif /* INCLUDED_ECA_ENGINE_IMPL_H */
