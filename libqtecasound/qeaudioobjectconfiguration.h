#ifndef _QEAUDIOOBJECTCONFIGURATION_H
#define _QEAUDIOOBJECTCONFIGURATION_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include <ecasound/audioio.h>

#include "qeinput.h"

class QLabel;
class QLineEdit;
class QGroupBox;
class QGrid;

/**
 * Input widget for configuring ecasound audio objects
 */
class QEAudioObjectConfiguration : public QEInput {
  Q_OBJECT
 public:

  QEAudioObjectConfiguration (AUDIO_IO* op, QWidget *parent = 0, const char *name = 0);
  AUDIO_IO* result(void) const { return(object_rep); }

public slots:

  virtual void update_results(void);
  void change_object(AUDIO_IO* op);

 private:

  AUDIO_IO* object_rep;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist;

  QGrid* paramgrid;
  QLabel* obj_desc;
  QLabel* obj_name;
  QGroupBox* paramgroup;

  void init_layout(void);

 public:

  virtual bool class_invariant(void) const { return(object_rep != 0); }
};

#endif
