#ifndef INCLUDE_ECA_MIDI_H
#define INCLUDE_ECA_MIDI_H

#include <pthread.h>
#include <deque>

#include "eca-error.h"

void init_midi_queues(void) throw(ECA_ERROR&);

/**
 * Routines for accessing raw MIDI -devices (OSS or ALSA).
 */
class MIDI_IN_QUEUE {

public:

  static const unsigned int max_queue_size_rep;

private:

  pthread_mutex_t midi_in_lock_rep;     // mutex ensuring exclusive access to MIDI-buffer
  pthread_cond_t midi_in_cond_rep;
  bool midi_in_locked_rep;

  deque<unsigned char> buffer_rep;
  int controller_value_rep;
  unsigned char running_status_rep;
  int current_ctrl_channel;

  bool is_voice_category_status_byte(unsigned char byte) const;
  bool is_system_common_category_status_byte(unsigned char byte) const;
  bool is_realtime_category_status_byte(unsigned char byte) const;  
  bool is_status_byte(unsigned char byte) const;
  
 public:

  void update_midi_queues(void);
  void put(unsigned char byte);  
  int last_controller_value(void) const;
  bool update_controller_value(int controller, int channel);
  MIDI_IN_QUEUE(void);
};

extern MIDI_IN_QUEUE midi_in_queue;

#endif
