#ifndef _QEDIALOG_H
#define _QEDIALOG_H

#include <vector>
#include <qdialog.h>

#include "qeinput.h"
#include "qeokcancelinput.h"

class QWidget;

/**
 * A base class for libqtecasound dialog widgets
 */
class QEDialog : public QDialog {
  Q_OBJECT

public slots:

  /** 
   * Fetch result data from widgets
   */
  virtual void update_results(void) = 0;

 public:

  /**
   * Class constructor
   */
  QEDialog (QWidget *parent = 0, const char *name = 0) : QDialog(parent, name, true) { }
};

#endif
