#ifndef _QEOPERATORCONFIGURATION_H
#define _QEOPERATORCONFIGURATION_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include <ecasound/eca-operator.h>

#include "qeinput.h"

class QLabel;
class QLineEdit;
class QGroupBox;
class QGrid;

/**
 * Input widget for configuring ecasound operators
 */
class QEOperatorConfiguration : public QEInput {
  Q_OBJECT
 public:

  QEOperatorConfiguration (OPERATOR* op, QWidget *parent = 0, const char *name = 0);

  OPERATOR* result(void) const { return(operator_rep); }

public slots:

  virtual void update_results(void);
  void change_operator(OPERATOR* op);

 private:

  OPERATOR* operator_rep;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist;

  QGrid* paramgrid;
  QLabel* obj_desc;
  QLabel* obj_name;
  QGroupBox* paramgroup;

  void init_layout(void);

 public:

  virtual bool class_invariant(void) const { return(operator_rep != 0); }
};

#endif
