#ifndef _ECA_QT_SESSION_H
#define _ECA_QT_SESSION_H

#include <qwidget.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qdir.h>
#include <qstring.h>

#include "eca-controller.h"

// #include "eca-qtchainsetup.h"

class QEChainsetup;

class QESession : public QWidget
{
  Q_OBJECT
public:
  QESession (ECA_CONTROLLER* econtrol, const ECA_SESSION* esession, QWidget *parent=0, const char *name=0);
  
public slots:
 void update_chainsetuplist(void);
 void button_load(void);
 void button_save(void);
 void button_new(void);
 void button_del(void);
 void button_toggle_connected(void);
 void button_open_chainsetup(void);
 void button_edit_chainsetup(void);
 void button_chainsetup_clicked(QListViewItem* i);

private slots:

signals:
 void session_closed(void);

protected:

  void timerEvent( QTimerEvent * );
  void closeEvent( QCloseEvent * );
//  void keyPressEvent(QKeyEvent*  kevent);
//  void paintEvent( QPaintEvent * );

 private:
  
  bool is_chainsetup_highlighted(void) const;
  void select_highlighted_chainsetup(void);

  void init_shortcuts(void);
  void init_chainsetuplist(void);
  void init_buttons(QBoxLayout* buttons);

  int timer_id;

  QListView* chainsetupview;
  QString current_dir;

  ECA_CONTROLLER* ctrl;
  const ECA_SESSION* ecasession;
};

#endif




