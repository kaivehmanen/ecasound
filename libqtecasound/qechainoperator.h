#ifndef _QECHAINOPERATOR_H
#define _QECHAINOPERATOR_H

#include <qwidget.h>
#include "eca-chainop.h"
#include "samplebuffer.h"
#include "qelibraryobject.h"

/**
 * Ecasound chain operator with a graphical interface
 */
class QEChainOperator : public QWidget,
			public QELibraryObject,
			public CHAIN_OPERATOR {
 public:

  QEChainOperator (QWidget *parent = 0, const char *name = 0) 
    : QWidget(parent, name) { }

 private:
};

#endif
