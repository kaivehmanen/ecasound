#ifndef _QECHAINOPINPUT_H
#define _QECHAINOPINPUT_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include <ecasound/eca-chainop-map.h>
#include <ecasound/eca-chainop.h>

class QLabel;

/**
 * Input widget selecting an libecasound chain operator
 */
class QEChainopInput : public QWidget {
  Q_OBJECT
 public:

  QEChainopInput (QWidget *parent = 0, const char *name = 0);

  CHAIN_OPERATOR* result(void) const { return(chainop_rep); }
  CHAIN_OPERATOR* clone_result(void);

public slots:

  void update_chainop(int index);
  void set_parameters(void);

 private:

  CHAIN_OPERATOR* chainop_rep;
  QLabel* cop_desc;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist; 
  int selected_index;

  void init_layout(void);
};

#endif
