#ifndef _QEFILENAMEINPUT_H
#define _QEFILENAMEINPUT_H

#include <string>
#include <qlineedit.h>

#include "qeinput.h"

class QBoxLayout;
class QLineEdit;

/**
 * Filename input with browsing
 */
class QEFilenameInput : public QEInput {
  Q_OBJECT

 public:

  /**
   * File browser modes. 'browse_existing' only allows to select
   * existing files, while in 'browse_new' mode, all filenames are
   * accepted.
   */
  enum {
    browse_existing,
    browse_any
  };

  QEFilenameInput (int mode, QWidget* parent = 0, const char *name = 0);

  /**
   * Current filename
   */
  const string& result_filename(void) const;

  /**
   * Get current browsing directory 
   */
  const string& current_dir(void) const { return(current_dir_rep); }

  /**
   * Sets the initial file name
   */
  void set_filename(const string& name) { filenameinput->setText(name.c_str()); }

  /**
   * Sets the starting directory for browsing
   */
  void set_current_dir(const string& newpath) { current_dir_rep = newpath; }

public slots:

  void button_browse(void);

  /** 
   * Fetches result data from widgets
   */
 virtual void update_results(void) { }

signals:

  /** 
   * Emitted when some file has been selected
   */
  void file_selected(void);

 public:

  QSize sizeHint(void) const;

 private:

  /**
   * Creates and initializes widget layout
   */
  void init_layout(void);

  QBoxLayout* filename;
  QLineEdit* filenameinput;
  QAccel *accel;
  int mode_rep;
  mutable string filename_rep;
  string current_dir_rep;
};

#endif
