#ifndef _QEAUDIOOBJECTCONFIGURATION_H
#define _QEAUDIOOBJECTCONFIGURATION_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include "audioio.h"
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

  /**
   * Class constructor
   */
  QEAudioObjectConfiguration (AUDIO_IO* op, QWidget *parent = 0, const char *name = 0);

  /**
   * Returns the resulting audio object instance
   */
  AUDIO_IO* result(void) const { return(object_rep); }

public slots:

  virtual void update_results(void);

  /** 
   * Initilizes a new audio object
   */
  void change_object(AUDIO_IO* op);

 private:

  AUDIO_IO* object_rep;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist;

  QGrid* paramgrid;
  QLabel* obj_desc;
  QLabel* obj_name;
  QGroupBox* paramgroup;

  /**
   * Creates and initializes widget layout
   */
  void init_layout(void);

 public:

  virtual bool class_invariant(void) const { return(object_rep != 0); }
};

#endif
