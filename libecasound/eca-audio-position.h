#ifndef INCLUDED_ECA_AUDIO_POSITION_H
#define INCLUDED_ECA_AUDIO_POSITION_H

#include "sample-specs.h"
#include "eca-audio-format.h"

/**
 * Position cursor for a finite length audio stream
 */
class ECA_AUDIO_POSITION : public ECA_AUDIO_FORMAT {

 private:

  SAMPLE_SPECS::sample_pos_t position_in_samples_rep;
  SAMPLE_SPECS::sample_pos_t length_in_samples_rep;

 public:

  // --
  // Get length
  // --
  virtual SAMPLE_SPECS::sample_pos_t length_in_samples(void) const;
  int length_in_seconds(void) const;
  double length_in_seconds_exact(void) const;

 protected:

  // --
  // Set length
  // --
  virtual void length_in_samples(SAMPLE_SPECS::sample_pos_t pos);
  void length_in_seconds(int pos_in_seconds);
  void length_in_seconds(double pos_in_seconds);

 public:

  // --
  // Get position
  // --
  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const;
  int position_in_seconds(void) const;
  double position_in_seconds_exact(void) const;

 protected:

  // --
  // Set position
  // --
  virtual void position_in_samples(SAMPLE_SPECS::sample_pos_t pos);
  void position_in_samples_advance(SAMPLE_SPECS::sample_pos_t pos);
  void position_in_seconds(int pos_in_seconds);
  void position_in_seconds(double pos_in_seconds);

 public:

  // ===================================================================
  // Seek position

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

  // --
  // Utilities
  // --

  /**
   * True if current position is beyond the end position or
   * smaller than zero.
   */
  inline bool out_position(void) const { return((( position_in_samples_rep < 0) &&
				    (position_in_samples_rep > length_in_samples_rep)) ? true : false); }
  /**
   * If current position is beyond the end position, 
   * set end position according to current positin.
   *
   * ensure:
   *  position_in_samples() == length_in_samples()
   */
  void extend_position(void) { length_in_samples_rep = 
			       (position_in_samples_rep > length_in_samples_rep)
			       ? position_in_samples_rep : length_in_samples_rep; }

  ECA_AUDIO_POSITION(const ECA_AUDIO_FORMAT& fmt);
  virtual ~ECA_AUDIO_POSITION(void);
};

#endif
