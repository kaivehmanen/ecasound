#ifndef _ECA_QT_WAVEDATA_H
#define _ECA_QT_WAVEDATA_H

#include <vector>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qvgroupbox.h>

#include <sys/types.h>

#include "samplebuffer.h"
#include "audioio-types.h"

class QEWaveBlock {
 public:
  int16_t min;
  int16_t max;
};

class QEWaveData : public QWidget
{
public:
  QEWaveData(AUDIO_IO_FILE* ioobject, QWidget *parent=0, const char *name=0 );

  void updateWaveData(bool force = false);
  bool is_valid(void) { return(valid_ioobject); }

  void set_buffersize(int value) { buffersize = value; } 

  void set_wave_color(const QColor& value) { wave_color = value; }
  void set_background_color(const QColor& value) { background_color = value; }
  void set_position_color(const QColor& value) { position_color = value; }
  void set_minmax_color(const QColor& value) { position_color = value; }
  void set_zeroline_color(const QColor& value) { position_color = value; }

protected:

  void paintEvent( QPaintEvent * );
  void timerEvent( QTimerEvent * );

private:

  int waveblock_maximum(int channel, double from, double step);
  int waveblock_minimum(int channel, double from, double step);
  void calculate_buffersize(void);

  void load_ews_data(void);
  void save_ews_data(bool forced = false);

  void load_ews_data_ascii(void);

  QRect ur;
  QPixmap pix;
  QPainter p;

  QColor wave_color, 
         background_color,
         position_color,
         minmax_color,
         zeroline_color;

  AUDIO_IO_FILE* ioobject;
  int buffersize;

  int samples_per_pixel;
  int xposcoord, newxposcoord;
  double step;
  double bufindex;
  //  vector<QEWaveBlock>::size_type t;
  int xcoord;
  int ycoord;
  int waveheight;
  int buffersize_save;
  int channels;

  bool valid_ioobject;
  vector<vector<QEWaveBlock> > waveblocks; 
};

#endif


