#ifndef INCLUDED_ECA_QTDEBUG_H
#define INCLUDED_ECA_QTDEBUG_H

#include <qwidget.h>
#include <qstring.h>

class QTextView;

class QEDebug : public QWidget
{
  Q_OBJECT
public:
  QEDebug(QWidget *parent = 0, const char *name = 0);
  QSize sizeHint(void) const;

protected:
  void	timerEvent(QTimerEvent *);
  void resizeEvent(QResizeEvent *);

private:
  QString t_rep;
  QTextView* tview_repp;
};

#endif
