#ifndef _ECA_QT_IODEVDIALOG_H
#define _ECA_QT_IODEVDIALOG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qlayout.h>

#include <vector>
#include <string>

#include "qeaudioformatinput.h"
#include "qefilenameinput.h"

class ECA_CHAINSETUP;

class QEIodevDialog : public QDialog {
  Q_OBJECT
 public:

  QEIodevDialog (const ECA_CHAINSETUP* csetup, QWidget *parent=0, const char *name=0);

  enum DIRECTION { input, output };

  ECA_AUDIO_FORMAT result_audio_format(void) const { return(r_aformat); }
  vector<string> result_chains(void) const { return(r_chains); }
  string result_filename(void) const { return(r_filename); }
  bool result_explicit_format(void) const { return(r_explicit); }
  DIRECTION result_direction(void) const { return(r_dir); }

  const string& current_dir(void) const { return(current_dir_rep); }

public slots:

  void set_direction(DIRECTION newdir) { r_dir = newdir; }
  void set_filename(const string& name) { r_filename = name; }
  void set_audio_format(const ECA_AUDIO_FORMAT& value) { r_aformat = value; }
  void set_chains(const vector<string>& newchains) { r_chains = newchains; }
  void set_explicit_format(bool enabled) { r_explicit = enabled; }
  void set_current_dir(const string& newpath) { current_dir_rep = newpath; }

protected slots:
  void inputGiven(void);

private slots:
  void set_input_mode(void) { b_dir->setButton(0); }
  void set_output_mode(void) { b_dir->setButton(1); }

 private:

  void init_shortcuts(void);
  void init_filename(void);
  void init_inout(void);
  void init_chains(void);

  ECA_AUDIO_FORMAT r_aformat;
  vector<string> r_chains;
  string r_filename;
  DIRECTION r_dir;
  bool r_explicit;

  string current_dir_rep;
  const ECA_CHAINSETUP* chainsetup;

  QEAudioFormatInput* aformat;
  QEFilenameInput* fnameinput;

  QButtonGroup* b_dir;
  QButtonGroup* b_override;
  QLineEdit* filenameinput;
  QPushButton* filenamebrowse;
  QListBox* chaininput;

  QBoxLayout* filename;
  QBoxLayout* inout;
  QBoxLayout* chains;
};

#endif
