#ifndef _QEOPENFILEDIALOG_H
#define _QEOPENFILEDIALOG_H

#include <qdialog.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include <vector>
#include <string>

#include <ecasound/qeaudioformatinput.h>
#include <ecasound/qefilenameinput.h>

#include "qeacceptinput.h"

class QVGroupBox;

/**
 * Dialog for selecting audio file, format and various 
 * related options.
 */
class QEOpenFileDialog : public QDialog {
  Q_OBJECT
 public:

  QEOpenFileDialog (QWidget *parent=0, const char *name=0);

private slots:

  void format_test(void);
  void update_refresh_toggle(bool v);
  void update_wcache_toggle(bool v);

 public:

  string result_filename(void) const { return(fname->result_filename()); }
  const string& current_dir(void) const { return(fname->current_dir()); }

  int result_bits(void) const { return(aformat->bits()); }
  int result_channels(void) const { return(aformat->channels()); }
  int result_srate(void) const { return(aformat->samples_per_second()); }

  bool result_wave_cache_toggle(void) const { return(wcache_toggle->isChecked()); }
  bool result_cache_refresh_toggle(void) const { return(refresh_toggle->isChecked()); }
  bool result_direct_mode_toggle(void) const { return(direct_toggle->isChecked()); }

 private:

  QBoxLayout* init_toggles(void);
  QCheckBox* wcache_toggle;
  QCheckBox* refresh_toggle;
  QCheckBox* direct_toggle;

  QEFilenameInput* fname; 
  QEAudioFormatInput* aformat;
  QEAcceptInput* okcancel;
};

#endif
