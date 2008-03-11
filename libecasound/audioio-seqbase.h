#ifndef INCLUDED_AUDIOIO_SEQUENCER_BASE_H
#define INCLUDED_AUDIOIO_SEQUENCER_BASE_H

#include <string>

#include "audioio-proxy.h"
#include "samplebuffer.h"
#include "eca-audio-time.h"

/**
 * Base class for audio sequencer objects. 
 *
 * Audio sequencer objects open one or more child objects
 * and alter the sequence of audio that is read from them.
 * Common operations are changing the position (inserting
 * silence at start, slicing), and looping of segments.
 *
 * The child objects also implement the AUDIO_IO interface,
 * or one of the more specialized subclasses.
 *
 * Related design patterns:
 *     - Proxy (GoF207)
 *
 * @author Kai Vehmanen
 */
class AUDIO_SEQUENCER_BASE : public AUDIO_IO_PROXY {

 public:

  /** @name Public functions */
  /*@{*/

  AUDIO_SEQUENCER_BASE ();
  virtual ~AUDIO_SEQUENCER_BASE(void);

  /*@}*/
  
  /** @name Reimplemented functions from ECA_OBJECT */
  /*@{*/
 
  /* Pure virtual class, not implemented */

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_PARAMETERS<string> */
  /*@{*/

  /* none */

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_OBJECT<string> */
  /*@{*/

  virtual AUDIO_SEQUENCER_BASE* clone(void) const;
  virtual AUDIO_SEQUENCER_BASE* new_expr(void) const { return new AUDIO_SEQUENCER_BASE(); }

  /*@}*/

  /** @name Reimplemented functions from ECA_AUDIO_POSITION */
  /*@{*/

  virtual void seek_position(void);

  /*@}*/

  /** @name Reimplemented functions from AUDIO_IO */
  /*@{*/

  virtual bool finite_length_stream(void) const { return(!child_looping_rep); }
  virtual bool finished(void) const;

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf);

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);

  /*@}*/

  /** @name New functions */
  /*@{*/

  /**
   * Sets the child object to open. Argument 'v' should 
   * be a string suitable for passing to ECA_OBJECT_MAP::object(),
   * i.e. a Ecasound Option Syntax (EOS) string.
   */
  void set_child_object_string(const std::string& v);

  const std::string& child_object_string(void) const { return child_name_rep; }

  /**
   * Set start offset for child object
   */
  void set_child_offset(const ECA_AUDIO_TIME& v);

  const ECA_AUDIO_TIME& child_offset(void) const { return child_offset_rep; }

  /**
   * Set start position inside child object.
   */
  void set_child_start_position(const ECA_AUDIO_TIME& v);

  const ECA_AUDIO_TIME& child_start_position(void) const { return child_start_pos_rep; }

  /**
   * Set child length. If not set, defaults to the total length. 
   */
  void set_child_length(const ECA_AUDIO_TIME& v);

  const ECA_AUDIO_TIME& child_length(void) const { return child_length_rep; }

  /**
   * Toggle whether child object data is looped.
   */
  void toggle_looping(bool v) { child_looping_rep = v; }

  bool child_looping(void) const { return child_looping_rep; }    

  /*@}*/

protected:

  void dump_child_debug(void);    
  SAMPLE_SPECS::sample_pos_t priv_public_to_child_pos(SAMPLE_SPECS::sample_pos_t pubpos) const;

private:

  SAMPLE_BUFFER tmp_buffer;

  bool child_looping_rep;
  ECA_AUDIO_TIME child_offset_rep,
                 child_start_pos_rep,
                 child_length_rep;
  std::string child_name_rep;
  long int buffersize_rep;
  bool child_write_started;
  bool init_rep;
  
  AUDIO_SEQUENCER_BASE& operator=(const AUDIO_SEQUENCER_BASE& x) { return *this; }
  AUDIO_SEQUENCER_BASE (const AUDIO_SEQUENCER_BASE& x) { }

};

#endif
