#ifndef _QEDIALOG_H
#define _QEDIALOG_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>

class QBoxLayout;

/**
 * Widget representing pair of 'ok' and 'cancel' buttons
 */
class QEAcceptInput : public QWidget {
  Q_OBJECT

public slots:

  void accept(void);
  void reject(void);
   
signals:

  void ok(void);
  void cancel(void);

 public:

  QEAcceptInput (QWidget *parent=0, const char *name=0);

  QSize sizeHint(void) const;
  
 private:

  QBoxLayout* buttons;
};

#endif
