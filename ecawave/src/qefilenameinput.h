#ifndef _QEFILENAMEINPUT_H
#define _QEFILENAMEINPUT_H

#include <string>

#include <qwidget.h>
#include <qlineedit.h>
#include <qlayout.h>

class QLineEdit;

/**
 * Widget for filename selection
 */
class QEFilenameInput : public QWidget {
  Q_OBJECT

 public:

  enum open_mode {
    file_open,
    file_save
  };

  QEFilenameInput (open_mode omode, QWidget *parent=0, const char *name=0);

  string result_filename(void) const { return(string(filenameinput->text().latin1())); }
  const string& current_dir(void) const { return(current_dir_rep); }

  void set_filename(const string& name) { filenameinput->setText(name.c_str()); }
  void set_current_dir(const string& newpath) { current_dir_rep = newpath; }

public slots:

  void button_browse(void);

signals:

  void file_selected(void);

 public:

  QSize sizeHint(void) const;

 private:

  void init_layout(void);

  QBoxLayout* filename;
  QLineEdit* filenameinput;
  string r_filename;
  string current_dir_rep;
  open_mode omode;
};

#endif




