#ifndef INCLUDED_ECA_AUDIO_POSITION_H
#define INCLUDED_ECA_AUDIO_POSITION_H

#include "sample-specs.h"
#include "eca-audio-format.h"

/**
 * Position cursor for a finite length audio stream
 */
class ECA_AUDIO_POSITION : public ECA_AUDIO_FORMAT {


 public:

  /** @name Constructors and destructors */
  /*@{*/

  ECA_AUDIO_POSITION(const ECA_AUDIO_FORMAT& fmt);
  virtual ~ECA_AUDIO_POSITION(void);

  /*@}*/

  /** @name Public functions for getting length information */
  /*@{*/

  virtual SAMPLE_SPECS::sample_pos_t length_in_samples(void) const;
  int length_in_seconds(void) const;
  double length_in_seconds_exact(void) const;

  /*@}*/

 protected:

  /** @name Protected functions for setting length */
  /*@{*/

  virtual void length_in_samples(SAMPLE_SPECS::sample_pos_t pos);
  void length_in_seconds(int pos_in_seconds);
  void length_in_seconds(double pos_in_seconds);

  /**
   * If current position is beyond the end position, 
   * set end position according to current positin.
   *
   * ensure:
   *  position_in_samples() == length_in_samples()
   */
  void extend_position(void) { 
    length_in_samples_rep = 
      (position_in_samples_rep > length_in_samples_rep)
      ? position_in_samples_rep : length_in_samples_rep; }

  /*@}*/

 public:

  /** @name Public functions for getting position information */
  /*@{*/

  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const;
  int position_in_seconds(void) const;
  double position_in_seconds_exact(void) const;

  /*@}*/

 protected:

  /** @name Protected functions for setting length */
  /*@{*/

  virtual void position_in_samples(SAMPLE_SPECS::sample_pos_t pos);
  void position_in_samples_advance(SAMPLE_SPECS::sample_pos_t pos);
  void position_in_seconds(int pos_in_seconds);
  void position_in_seconds(double pos_in_seconds);

  /*@}*/

 public:

  /** @name Public functions for seeking to new positions */
  /*@{*/

  /**
   * Seek to current position. It's guaranteed that current position
   * is valid (between [0,length_in_samples()]).
   *
   * require:
   *  out_position() != true
   */
  virtual void seek_position(void) = 0;

  void seek_position_in_samples(SAMPLE_SPECS::sample_pos_t pos_in_samples);
  void seek_position_in_samples_advance(SAMPLE_SPECS::sample_pos_t pos_in_samples);
  void seek_position_in_seconds(double pos_in_seconds);
  void seek_first(void);
  void seek_last(void);

  /*@}*/

  /** @name Public utility functions */
  /*@{*/

  /**
   * True if current position is beyond the end position or
   * smaller than zero.
   */
  inline bool out_position(void) const { 
    return((( position_in_samples_rep < 0) &&
	    (position_in_samples_rep > length_in_samples_rep)) ? true : false); }

  /*@}*/

 protected:

  /** @name Functions reimplemented from ECA_AUDIO_FORMAT */
  /*@{*/

  virtual void samples_per_second_changed(SAMPLE_SPECS::sample_rate_t old_value);

  /*@}*/

 private:

  SAMPLE_SPECS::sample_pos_t position_in_samples_rep;
  SAMPLE_SPECS::sample_pos_t length_in_samples_rep;

};

#endif
