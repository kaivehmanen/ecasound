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
    
  /**
   * Accepts the current result and exit the modal event loop
   */
  void accept(void);

  /**
   * Rejects the current result and exit the modal event loop
   */
  void reject(void);

  /** 
   * Fetch result data from widgets
   */
  virtual void update_results(void) { }
   
signals:

  /**
   * Emitted when result is accepted
   */
  void ok(void);

  /**
   * Emitted when result is rejected
   */
  void cancel(void);

 public:

  QEOkCancelInput (QWidget *parent = 0, const char *name = 0);
  QSize sizeHint(void) const;
  
 private:

  QBoxLayout* buttons;
};

#endif
