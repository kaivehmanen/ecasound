#ifndef _QEOPERATORINPUT_H
#define _QEOPERATORINPUT_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include <ecasound/eca-operator.h>

#include "qeinput.h"

class QLabel;
class QLineEdit;

/**
 * Input widget for dynamic objects with numeric parameters
 */
class QEOperatorInput : public QEInput {
  Q_OBJECT
 public:

  QEOperatorInput (OPERATOR* op, QWidget *parent = 0, const char *name = 0);

  OPERATOR* result(void) const { return(operator_rep); }

  virtual bool class_invariant(void) const { return(operator_rep != 0); }

public slots:

  virtual void update_results(void);

 private:

  OPERATOR* operator_rep;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist;

  void init_layout(void);
};

#endif
