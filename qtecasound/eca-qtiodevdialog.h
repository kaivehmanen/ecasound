#ifndef _ECA_QT_IODEVDIALOG_H
#define _ECA_QT_IODEVDIALOG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qlayout.h>

#include <vector>
#include <string>

class ECA_CHAINSETUP;

class QEIodevDialog : public QDialog {
  Q_OBJECT
 public:

  QEIodevDialog (const ECA_CHAINSETUP* csetup, QWidget *parent=0, const char *name=0);

  enum DIRECTION { input, output };

  int result_bits(void) const { return(r_bits); }
  int result_channels(void) const { return(r_channels); }
  int result_srate(void) const { return(r_srate); }
  vector<string> result_chains(void) const { return(r_chains); }
  string result_filename(void) const { return(r_filename); }
  bool result_explicit_format(void) { return(r_explicit); }
  DIRECTION result_direction(void) { return(r_dir); }

  const string& current_dir(void) const { return(current_dir_rep); }

public slots:

  void set_direction(DIRECTION newdir) { r_dir = newdir; }
  void set_filename(const string& name) { r_filename = name; }
  void set_bits(int value) { r_bits = value; }
  void set_channels(int value) { r_channels = value; }
  void set_srate(int value) { r_srate = value; }
  void set_chains(const vector<string>& newchains) { r_chains = newchains; }
  void set_explicit_format(bool enabled) { r_explicit = enabled; }
  void set_current_dir(const string& newpath) { current_dir_rep = newpath; }

protected slots:

  void inputGiven(void);
  void button_browse(void);
  void handle_key(int c);

private slots:
  void handle_key_6(void) { handle_key((int)'6'); }
  void handle_key_8(void) { handle_key((int)'8'); }
  void handle_key_i(void) { handle_key((int)'i'); }
  void handle_key_u(void) { handle_key((int)'u'); }

 protected:

 void keyPressEvent(QKeyEvent*  kevent);

 private:

  void init_shortcuts(void);
  void init_filename(void);
  void init_inout(void);
  void init_format(void);
  void init_chains(void);

  int r_bits, r_channels, r_srate;
  vector<string> r_chains;
  string r_filename;
  DIRECTION r_dir;
  bool r_explicit;

  string current_dir_rep;

  const ECA_CHAINSETUP* chainsetup;

  QLineEdit* filenameinput;
  QPushButton* filenamebrowse;
  QSpinBox* srateinput;
  QListBox* chaininput;

  QButtonGroup* b_channels;
  QButtonGroup* b_bits;
  QButtonGroup* b_dir;
  QButtonGroup* b_override;

  QBoxLayout* filename;
  QBoxLayout* inout;
  QBoxLayout* format;
  QBoxLayout* chains;
};

#endif



