#ifndef INCLUDED_MIDI_SERVER_H
#define INCLUDED_MIDI_SERVER_H

#include <deque>
#include <vector>
#include <string>
#include <map>

#include <pthread.h>
#include <kvutils/locks.h>
#include "midiio.h"

/**
 * MIDI i/o engine.
 *
 * @author Kai Vehmanen
 */
class MIDI_SERVER {

  friend void* start_midi_server_io_thread(void *ptr);

 public:

  static const unsigned int max_queue_size_rep;

  static bool is_voice_category_status_byte(unsigned char byte);
  static bool is_system_common_category_status_byte(unsigned char byte);
  static bool is_realtime_category_status_byte(unsigned char byte);  
  static bool is_status_byte(unsigned char byte);

 private:

  deque<unsigned char> buffer_rep;
  mutable map<pair<int,int>,int> controller_values_rep;
  unsigned char running_status_rep;
  int current_ctrl_channel_rep;
  int current_ctrl_number;

  vector<MIDI_IO*> clients_rep;
  pthread_t io_thread_rep;
  bool thread_running_rep;
  int schedpriority_rep;
  ATOMIC_INTEGER exit_request_rep;
  ATOMIC_INTEGER stop_request_rep;
  ATOMIC_INTEGER running_rep;

  MIDI_SERVER& operator=(const MIDI_SERVER& x) { return *this; }
  MIDI_SERVER (const MIDI_SERVER& x) { }

  void io_thread(void);
  void parse_receive_queue(void);

 public:

  bool is_running(void) const;

  void start(void);
  void stop(void);

  void set_schedpriority(int v) { schedpriority_rep = v; }

  void register_client(MIDI_IO* mobject);
  void unregister_client(MIDI_IO* mobject);

  void add_controller_trace(int channel, int ctrl);
  void remove_controller_trace(int channel, int ctrl);
  int last_controller_value(int channel, int ctrl) const;

  MIDI_SERVER (void);
  ~MIDI_SERVER(void);
};

#endif
