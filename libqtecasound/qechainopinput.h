#ifndef _QECHAINOPINPUT_H
#define _QECHAINOPINPUT_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include <ecasound/eca-static-object-maps.h>
#include <ecasound/eca-chainop.h>

#include "qeinput.h"

class QLabel;

/**
 * Chain operator input widget
 */
class QEChainopInput : public QEInput {
  Q_OBJECT
 public:

  QEChainopInput (QWidget *parent = 0, const char *name = 0);

  CHAIN_OPERATOR* result(void) const { return(chainop_rep); }

public slots:

  virtual void update_results(void);

private slots:

  void update_chainop(int index);

 private:

  CHAIN_OPERATOR* chainop_rep;
  QLabel* cop_desc;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist; 
  int selected_index;

  void init_layout(void);
};

#endif
