#ifndef _QECHAINOPERATORINPUT_H
#define _QECHAINOPERATORINPUT_H

#include <vector>

#include <qwidget.h>
#include <qhbox.h>
#include <qlineedit.h>

#include "eca-static-object-maps.h"
#include "eca-chainop.h"

#include "qeinput.h"

class QLabel;
class QTabWidget;
class QEObjectMap;
class QEOperatorConfiguration;

/**
 * Chain operator input widget
 */
class QEChainOperatorInput : public QEInput {
  Q_OBJECT
 public:

  QEChainOperatorInput (QWidget *parent = 0, const char *name = 0);

  /**
   * Returns the resulting chain operator object instance
   */
  CHAIN_OPERATOR* result(void) const { return(chainop_rep); }

public slots:

  virtual void update_results(void);

private slots:

 void operator_updated(void); 
 void operator_updated(const QString& a); 

 private:

  CHAIN_OPERATOR* chainop_rep;
  QEOperatorConfiguration* opconf;
  QTabWidget* maptab_rep;
  vector<QEObjectMap*> omap_inputs;
  QLabel* cop_desc;
  vector<QLabel*> paramlist;
  vector<QLineEdit*> inputlist; 
  int selected_index;

  /**
   * Creates and initializes widget layout
   */
  void init_layout(void);
};

#endif
