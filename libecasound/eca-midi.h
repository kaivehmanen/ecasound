#ifndef INCLUDE_ECA_MIDI_H
#define INCLUDE_ECA_MIDI_H

#define MIDI_IN_QUEUE_SIZE 1024

#include <pthread.h>
#include <vector>

class ECA_ERROR;

void init_midi_queues(void) throw(ECA_ERROR&);

/**
 * Routines for accessing raw MIDI -devices (OSS or ALSA).
 */
class MIDI_IN_QUEUE {

private:

  pthread_mutex_t midi_in_lock_rep;     // mutex ensuring exclusive access to MIDI-buffer
  pthread_cond_t midi_in_cond_rep;
  bool midi_in_locked_rep;

  vector<char> buffer;
  bool right;

  size_t current_put, current_get;
  size_t bufsize;
  double controller_value;
  
  bool is_status_byte(char byte) const;
  bool forth_get(void);
  
 public:
  
  void update_midi_queues(void);
  void put(char byte);  
  double last_controller_value(void) const;
  bool update_controller_value(double controller, double channel);
  MIDI_IN_QUEUE(void);
};

extern MIDI_IN_QUEUE midi_in_queue;

#endif
