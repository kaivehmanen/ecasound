#ifndef _ECA_QT_CHAIN_H
#define _ECA_QT_CHAIN_H

#include <qwidget.h>
#include <qlayout.h>
#include <qlistview.h>

#include <eca-chain.h>
#include <eca-control.h>

class QEButtonRow;

class QEChain : public QWidget
{
  Q_OBJECT
public:
  QEChain (ECA_CONTROL* econtrol, const CHAIN* chain, QWidget *parent=0, const char *name=0);
  
public slots:
 void update_chainlist(void);
 void update_chainlist_clean(void);

private slots:
  void not_implemented(void);

signals:
 void widget_closed(void);

protected:

  void closeEvent( QCloseEvent * );
  void timerEvent( QTimerEvent * );

 private:

  void init_chainlist(void);
  void init_buttons(void);

  QEButtonRow* buttons;

  ECA_CONTROL* ctrl;
  const CHAIN* chain;

  QListView* chainview;
  QListViewItem* newitem;
};

#endif
