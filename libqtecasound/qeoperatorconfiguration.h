#ifndef _QEOPERATORCONFIGURATION_H
#define _QEOPERATORCONFIGURATION_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include "eca-operator.h"

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

  /**
   * Class constructor
   */
  QEOperatorConfiguration (OPERATOR* op, QWidget *parent = 0, const char *name = 0);

  /**
   * Returns the resulting operator object instance
   */
  OPERATOR* result(void) const { return(operator_rep); }

public slots:

  virtual void update_results(void);

  /** 
   * Initilizes a new operator object
   */
  void change_operator(OPERATOR* op);

 private:

  OPERATOR* operator_rep;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist;

  QGrid* paramgrid;
  QLabel* obj_desc;
  QLabel* obj_name;
  QGroupBox* paramgroup;

  /**
   * Creates and initializes widget layout
   */
  void init_layout(void);

 public:

  virtual bool class_invariant(void) const { return(operator_rep != 0); }
};

#endif
