#ifndef _QESAVEFILEDIALOG_H
#define _QESAVEFILEDIALOG_H

#include <qdialog.h>
#include <qlayout.h>

#include <vector>
#include <string>

#include "qeacceptinput.h"
#include "qefilenameinput.h"

class QVGroupBox;

/**
 * Dialog for selecting audio file, format and various 
 * related options.
 */
class QESaveFileDialog : public QDialog {
  Q_OBJECT
 public:

  QESaveFileDialog (QWidget *parent=0, const char *name=0);

 public:

  string result_filename(void) const { return(fname->result_filename()); }
  const string& current_dir(void) const { return(fname->current_dir()); }

 private:

  QEFilenameInput* fname; 
  QEAcceptInput* okcancel;
};

#endif
