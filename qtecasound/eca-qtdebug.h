#ifndef _ECA_QTDEBUG_H
#define _ECA_QTDEBUG_H

#include <qwidget.h>
#include <qtextview.h>
#include <qstring.h>

class QEDebug : public QWidget
{
  Q_OBJECT
public:
  QEDebug( QWidget *parent=0, const char *name=0 );
  QSize sizeHint(void) const;

protected:
  void	timerEvent( QTimerEvent * );
  void resizeEvent( QResizeEvent * );

private:
  QString t;
  QTextView* tview;
};

#endif
