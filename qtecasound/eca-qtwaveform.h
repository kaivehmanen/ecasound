#ifndef _ECA_QT_WAVEFORM_H
#define _ECA_QT_WAVEFORM_H

#include <vector>
#include <qwidget.h>
#include <qlayout.h>

#include "eca-qtwavedata.h"
#include "audioio.h"
#include "samplebuffer.h"

class QEWaveForm : public QWidget
{
  Q_OBJECT
public:
  QEWaveForm(AUDIO_IO_FILE* iodevice, QWidget *parent=0, const char *name=0 );

signals:
  void setTotalSteps (int totalSteps);
  void setProgress (int progress);

public slots:
  void updateWaveData(bool force = false);
  void updateWaveView(void);

private slots:
  void forcedUpdateWaveData(void) { updateWaveData(true); }

protected:

  void init_buttons(); 
  void init_shortcuts();
  void closeEvent( QCloseEvent *e );

private:



  QBoxLayout* topLayout;
  QBoxLayout* buttons;

  QEWaveData* wdata;
  bool valid_iodevice;
};


#endif




