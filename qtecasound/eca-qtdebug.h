#ifndef _ECA_DEBUG_H
#define _ECA_DEBUG_H

#include <qwidget.h>
#include <qmultilinedit.h>

class QEDebug : public QWidget
{
  Q_OBJECT
public:
  QEDebug( QWidget *parent=0, const char *name=0 );
  QSize sizeHint(void) const;

signals:
  void append(const QString&);

protected:
  void	timerEvent( QTimerEvent * );
  void resizeEvent( QResizeEvent * );


private:
  QMultiLineEdit* mle;  
};

#endif

