#ifndef _ECA_QT_DEBUGTAB_H
#define _ECA_QT_DEBUGTAB_H

#include <qwidget.h>
#include "eca-qtinte.h"

class QEDebugTab : public QWidget
{
  Q_OBJECT
public:
  QEDebugTab (QEInterface *interface = 0, QWidget* parent = 0, const char *name = 0);
};

#endif
