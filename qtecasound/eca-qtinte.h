#ifndef _ECA_QTINTE_H
#define _ECA_QTINTE_H

#include <qapplication.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qstatusbar.h>

#include "eca-session.h"
#include "eca-controller.h"

// #include "eca-qtrtposition.h"
// #include "eca-qtsession.h"
class QERuntimePosition;
class QESession;
class QEButtonRow;

class QEInterface : public QWidget
{
  Q_OBJECT
public:
  QEInterface(ECA_CONTROLLER* control, const ECA_SESSION* session, QWidget *parent=0, const char *name=0 );

protected:
  void focusInEvent ( QFocusEvent * );
  void focusOutEvent ( QFocusEvent * );

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
  void emsg_estatus(void);
  void emsg_fstatus(void);
  void emsg_cstatus(void);
  void emsg_setpos(double pos_seconds);
  void get_focus(void);

private slots:
  void init_sessionsetup(void);
  void update_statusbar(void);
  void update_runtimebar(void);
  void not_implemented(void);
  void sessionsetup_closed(void);

signals:
  void is_finished(void);
  void clear_textinput(void);
  void update_signallevel(int);
  void mute_signallevels(void);
  void focus_to_session();

private:
  QLineEdit* tekstirivi;
  QStatusBar* statusbar;

  QEButtonRow* buttonrow;

  QERuntimePosition* rpos;
  QESession* child_setup;

  ECA_CONTROLLER* ctrl;
  const ECA_SESSION* ecaparams;

  void init_statusbar(void);
  void init_buttons();
  void init_runtimebar(QBoxLayout* buttons);
  void init_bottomrow(QBoxLayout* bottomrow);
  void init_textinput(QBoxLayout* textinput);
  void init_debugout(QBoxLayout* debugout);
};

#endif



