#ifndef QEWAVEFORM_H
#define QEWAVEFORM_H

#include <string>
#include <vector>

#include <qwidget.h>
#include <qevent.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kvutils/definition_by_contract.h>
#include <ecasound/audioio-types.h>

/**
 * One block of graphical wave data
 */
class QEWaveBlock {
 public:
  int16_t min;
  int16_t max;
};

/**
 * Waveform widget
 *
 * This class represents a one-channel waveform display.
 */
class QEWaveForm : public QWidget, public DEFINITION_BY_CONTRACT {
  Q_OBJECT

public slots:

  /**
   * Initialize view with new waveform data
   */
  void update_wave_blocks(const vector<QEWaveBlock>* block);

  /**
   * Set current position in as xcoord relative to current widget width
   */
  void current_position_relative(int xcoord);

  /**
   * Set current position in blocks
   */
  void current_position(long int blocks);

  /**
   * Repaint the current pointer
   */
  void repaint_current_position(void);

  /**
   * Set marked area start position in sample blocks
   */
  void marked_area_begin(long int v);

  /***
   * Set marked are end position in sample blocks
   */
  void marked_area_end(long int v);

  /**
   * Set marked area using relative x-coordinates
   */
  void mark_area_relative(int from, int to);

  /***
   * Toggle whether marking is enabled
   */
  void toggle_marking(bool v) { marked_rep = v; }

  /**
   * Set visible area start and end positions in sample blocks
   */
  void visible_area(long int start, long int end);

  /***
   * Zoom to marked area
   */
  void zoom_to_marked(void);

  // ------
  
  /**
   * Set color for drawing the wave form
   */
  void set_wave_color(const QColor& value) { wave_color = value; }

  /**
   * Set color of waveform background
   */
  void set_background_color(const QColor& value) { background_color = value; }

  /**
   * Set position marker color
   */
  void set_position_color(const QColor& value) { position_color = value; }

  /**
   * Set color for drawing marked areas
   */
  void set_marked_color(const QColor& value) { marked_color = value; }

  /**
   * Set background color for drawing marked areas
   */
  void set_marked_background_color(const QColor& value) { marked_background_color = value; }

  /**
   * Set position marked color for marked areas
   */
  void set_marked_position_color(const QColor& value) { marked_position_color = value; }

  /**
   * Set color for minimum and maximum sample value lines
   */
  void set_minmax_color(const QColor& value) { minmax_color = value; }

  /**
   * Set color of the silent/zero line
   */
  void set_zeroline_color(const QColor& value) { zeroline_color = value; }

 public:

  /**
   * Current position in sample blocks
   */
  long int current_position(void) const { return(current_position_rep); }

  /**
   * Marked area start position in sample blocks
   */
  long int marked_area_begin(void) const { return(marked_area_begin_rep); }

  /***
   * Marked are end position in sample blocks
   */
  long int marked_area_end(void) const { return(marked_area_end_rep); }

  /***
   * True, if some area is marked
   */
  bool is_marked(void) const { return(marked_rep); }

  /**
   * Visible area start position in sample blocks
   */
  long int visible_area_begin(void) const { return(visible_area_begin_rep); }

  /***
   * Visible are end position in sample blocks
   */
  long int visible_area_end(void) const { return(visible_area_end_rep); }

  // ------

  void paintEvent(QPaintEvent* e);

 private:

  int waveblock_minimum(double from, double step);
  int waveblock_maximum(double from, double step);

  QColor calculate_wave_color_value(int value);

  // ------

  int channel_rep;

  long int current_position_rep;  // position in blocks
  long int marked_area_begin_rep,
           marked_area_end_rep,
           visible_area_begin_rep,
           visible_area_end_rep;

  const vector<QEWaveBlock>* waveblock;
  vector<QEWaveBlock> empty_blocks;

  double step;
 
  int prev_xpos_minimum, prev_xpos_maximum;
  int xpos, prev_xpos;
  bool prev_inside_marked;

  bool marked_rep;

  QColor wave_color, 
         background_color,
         position_color,
         minmax_color,
         marked_color,
         marked_background_color,
         marked_position_color,
         zeroline_color;

 public:

  QSize sizeHint(void) const;
  QEWaveForm (int channel,
	      QWidget *parent = 0, 
	      const char *name = 0);
};

#endif
