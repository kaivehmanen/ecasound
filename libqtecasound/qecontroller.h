#ifndef _QECONTROLLER_H
#define _QECONTROLLER_H

#include <qwidget.h>
#include "qelibraryobject.h"
#include "ctrl-source.h"

class QEController : public QWidget,
		     public QELibraryObject,
		     public CONTROLLER_SOURCE {
  Q_OBJECT

 public:

  /**
   * Class constructor
   */
  QEController (QWidget *parent = 0, const char *name = 0) 
    : QWidget(parent, name) { }
};

#endif
