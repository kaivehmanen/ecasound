#ifndef _QECONTROLLER_H
#define _QECONTROLLER_H

#include <qwidget.h>

#include "eca-chainop.h"
#include "samplebuffer.h"

class QEController : public QWidget,
		     public QElibraryObject,
		     public CONTROLLER_SOURCE {
  Q_OBJECT
 public:

  QEController (QWidget *parent = 0, const char *name = 0);
};

#endif
