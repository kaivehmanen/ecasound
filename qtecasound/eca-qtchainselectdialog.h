#ifndef _ECA_QT_CHAINSELECTDIALOG_H
#define _ECA_QT_CHAINSELECTDIALOG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlayout.h>

#include <vector>
#include <string>

#include <eca-chainsetup.h>

class QEChainselectDialog : public QDialog {
  Q_OBJECT
 public:

  QEChainselectDialog (const ECA_CHAINSETUP* csetup, QWidget *parent=0, const char *name=0);

  vector<string> result_chains(void) const { return(r_chains); }

public slots:

  void set_chains(const vector<string>& newchains) { r_chains = newchains; }

protected slots:

  void inputGiven(void);
  void handle_key(int c);

 protected:

 void keyPressEvent(QKeyEvent*  kevent);

 private:

  void init_shortcuts(void);
  void init_chains(void);

  vector<string> r_chains;

  const ECA_CHAINSETUP* chainsetup;

  QListBox* chaininput;

  QBoxLayout* chains;
};

#endif



