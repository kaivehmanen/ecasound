#ifndef INCLUDED_KVU_THREADS_H
#define INCLUDED_KVU_THREADS_H

#include <string>
#include <pthread.h>

using std::string;

int kvu_pthread_mutex_spinlock (pthread_mutex_t *mp, long int spinlimit);
int kvu_pthread_timed_wait(pthread_mutex_t* mutex, pthread_cond_t* cond, long int seconds);
string kvu_pthread_timed_wait_result(int result, const string& prefix);

#endif
