#ifndef _QEOKCANCEL_H
#define _QEOKCANCEL_H

#include "qeinput.h"

class QBoxLayout;
class QWidget;

/**
 * 'ok' - 'cancel' input buttons
 */
class QEOkCancelInput : public QEInput {
  Q_OBJECT

public slots:

  void accept(void);
  void reject(void);

  /** 
   * Fetch result data from widgets
   */
  virtual void update_results(void) { }
   
signals:

  void ok(void);
  void cancel(void);

 public:

  QEOkCancelInput (QWidget *parent = 0, const char *name = 0);
  QSize sizeHint(void) const;
  
 private:

  QBoxLayout* buttons;
};

#endif
