#ifndef _QEINPUT_H
#define _QEINPUT_H

#include <qwidget.h>
#include <kvutils/definition_by_contract.h>

/**
 * A base class for libqtecasound input dialogs
 */
class QEInput : public QWidget,
		public DEFINITION_BY_CONTRACT {
  Q_OBJECT
public:

  /**
   * Class constructor
   */
  QEInput (QWidget *parent = 0, const char *name = 0) : QWidget(parent, name) { }

public slots:


  /**
   * Enables/activates the inputs
   */
  virtual void enable(void) { setEnabled(true); }
 
  /**
   * Disables the inputs
   */
  virtual void disable(void) { setEnabled(false); }

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
