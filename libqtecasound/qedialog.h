#ifndef _QEDIALOG_H
#define _QEDIALOG_H

#include <vector>
#include <qdialog.h>

#include "qeinput.h"
#include "qeokcancelinput.h"

class QWidget;

class QEDialog : public QDialog {
  Q_OBJECT
 public:

  QEDialog (QWidget *parent = 0, const char *name = 0);
};

#endif
