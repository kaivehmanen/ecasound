#ifndef INCLUDED_QE_INTE_H
#define INCLUDED_QE_INTE_H

#include <qapplication.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qstatusbar.h>

#include "eca-session.h"
#include "eca-control.h"

class QERuntimePosition;
class QEChainsetup;
class QEButtonRow;

class QEInterface : public QWidget
{
  Q_OBJECT
public:
  QEInterface(ECA_CONTROL* control, QWidget *parent=0, const char *name=0);

public slots:
  void emsg_general(void);
  void emsg_exec(void);
  void emsg_quit(void);
  void emsg_start(void);
  void emsg_stop(void);
  void emsg_forward(void);
  void emsg_rewind(void);
  void emsg_rw_begin(void);
  void emsg_status(void);
  void emsg_csstatus(void);
  void emsg_ctrlstatus(void);
  void emsg_estatus(void);
  void emsg_fstatus(void);
  void emsg_cstatus(void);
  void emsg_setpos(double pos_seconds);
  void get_focus(void);

private slots:
  void update_statusbar(void);
  void update_runtimebar(void);
  void not_implemented(void);

signals:
  void is_finished(void);
  void clear_textinput(void);
  void update_signallevel(int);
  void mute_signallevels(void);
  void focus_to_session();

private:
  QLineEdit* tekstirivi_repp;
  QStatusBar* statusbar_repp;
  QEButtonRow* buttonrow_repp;
  QERuntimePosition* rpos_repp;
  QEChainsetup* session_repp;

  ECA_CONTROL* ctrl_repp;

  void init_layout(void);
  void init_statusbar(void);
  void init_buttons();
  void init_tabwidget(QBoxLayout* debugout);
  void init_runtimebar(QBoxLayout* buttons);
  void init_bottomrow(QBoxLayout* bottomrow);
  void init_textinput(QBoxLayout* textinput);
};

#endif
