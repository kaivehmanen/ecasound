#ifndef _ECA_QT_SIGNALLEVEL_H
#define _ECA_QT_SIGNALLEVEL_H

#include <vector>
#include <qwidget.h>

#include "audioio.h"
#include "samplebuffer.h"
#include "eca-qtlevelmeter.h"

class QESignalLevel : public QWidget
{
  Q_OBJECT
public:
  QESignalLevel(vector<SAMPLE_BUFFER>* c, QWidget *parent=0, const char *name=0 );

public slots:
  void update(int);
  void mute(void);

private:

    vector<QELevelMeter*> levelmeters;
    vector<SAMPLE_BUFFER>* inputs;
};

#endif






