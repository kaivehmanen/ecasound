#ifndef _QESTRINGPARAMETERINPUT_H
#define _QESTRINGPARAMETERINPUT_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include <ecasound/dynamic-object.h>

#include "qeinput.h"

class QLabel;
class QLineEdit;

/**
 * Input widget for dynamic objects with string parameters
 */
class QEStringParameterInput : public QEInput {
  Q_OBJECT
 public:

  QEStringParameterInput (OPERATOR* op, QWidget *parent = 0, const char *name = 0);

  DYNAMIC_OBJECT<string>* result(void) const { return(operator_rep); }

  virtual bool class_invariant(void) const { return(operator_rep != 0); }

public slots:

  virtual void update_results(void);

 private:

  DYNAMIC_OBJECT<string>* operator_rep;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist;

  void init_layout(void);
};

#endif
