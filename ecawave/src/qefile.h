#ifndef QEFILE_H
#define QEFILE_H

#include <string>
#include <vector>

#include <qwidget.h>
#include <qlayout.h>

#include <kvutils/definition_by_contract.h>
#include <ecasound/eca-audio-format.h>
#include <ecasound/eca-audio-time.h>

#include "qewaveform.h"

class AUDIO_IO;

/**
 * Single audio file
 *
 * This class provides the user interface and functionally 
 * for single audio file.
 */
class QEFile : public QWidget, public DEFINITION_BY_CONTRACT {
  Q_OBJECT

public slots:

  void update_wave_form_data(void);

  /**
   * Set the default audio format used when updating wave form data
   */
  void set_audio_format(const ECA_AUDIO_FORMAT& afrm) { aformat = afrm; }

  void current_position(long int samples);
  void visible_area(long int startpos_samples, long int endpos_samples);
  void mark_area_relative(int from, int to);

  /**
   * Unmark file
   */
  void unmark(void);

  /**
   * Zoom to marked area - visible area is the same as the marked area
   */
  void zoom_to_marked(void);

  /**
   * Zoom-out visible area to whole audio file length
   */
  void zoom_out(void);

signals:
  
  /**
   * Emitted when marked area is changed
   */
  void selection_changed(void);

  /**
   * Emitted when current position is changed
   */
  void current_position_changed(ECA_AUDIO_TIME curpos);

  /** 
   * Emitted when visible area is changed
   */
  void visible_area_changed(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end);

  /** 
   * Emitted when marked area is changed
   */
  void marked_area_changed(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end);

 public:

  static const int max_buffer_size = 512;

  /**
   * Returns current position in samples
   */
  long int current_position(void) const;

  /**
   * Returns selection length in samples
   */
  long int selection_length(void) const;

  /**
   * Returns audio file length in samples
   */
  long int length(void) const { return(length_rep); }

  /**
   * Returns samples per second
   */
  long int samples_per_second(void) const { return(sample_rate_rep); }

  /**
   * Whether file is ready for use
   */
  bool is_valid(void) const;

  /**
   * Whether to use wave form cache
   */
  void toggle_wave_cache(bool v) { wcache_toggle_rep = v; }

  /**
   * Whether to force wave form cache refresh
   */
  void toggle_cache_refresh(bool v) { refresh_toggle_rep = v; }

  const string& filename(void) const { return(filename_rep); }

 private:

  void open_io_object(void);
  void calculate_buffersize(void);

  bool check_ews_data(void);
  void load_ews_data(void);
  void save_ews_data(bool forced);

  void init_layout(void);
  long int coord_to_samples(int coord);
  long int blocks_to_samples(long int blocks);

  AUDIO_IO* io_object;
  int waveform_count;
  int buffersize_rep;
  string filename_rep;
  ECA_AUDIO_FORMAT aformat;
  long int length_rep;
  long int channels_rep;
  long int sample_rate_rep;
  
  bool refresh_toggle_rep, wcache_toggle_rep;

  int last_mouse_xpos;
  int last_mousemove_xpos;
  bool marking_rep;

  vector<vector<QEWaveBlock> > waveblocks;
  vector<QEWaveForm*> waveforms;

 public:

  bool eventFilter(QObject *, QEvent *e);
  QSize sizeHint(void) const;

  bool class_invariant(void) const { return(io_object != 0); }
  
  /**
   * Open a new file and update waveform data
   */
  void open(const string& filename);

  QEFile (const string& filename, bool use_wave_cache, bool force_refresh, QWidget *parent = 0, const char *name = 0);
  QEFile (QWidget *parent = 0, const char *name = 0) : QWidget(parent,name) { }
};

#endif
