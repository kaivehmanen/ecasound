#ifndef _QEINPUT_H
#define _QEINPUT_H

#include <qwidget.h>
#include <kvutils/definition_by_contract.h>

class QEInput : public QWidget,
		public DEFINITION_BY_CONTRACT {
  Q_OBJECT
public:

  QEInput (QWidget *parent = 0, const char *name = 0);

public slots:


  /**
   * Enables/activates the inputs
   */
  virtual void enable(void) { }
 
  /**
   * Disables the inputs
   */
  virtual void disable(void) { }

  /** 
   * Fetch result data from widgets
   */
  virtual void update_results(void) = 0;

signals:

  /**
    * Emitted when input values are changed
    */
  void changed(void);
};

#endif
