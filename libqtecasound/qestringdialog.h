#ifndef _QESTRINGDIALOG_H
#define _QESTRINGDIALOG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qlineedit.h>

class QEStringDialog : public QDialog {
  Q_OBJECT
 public:

  /**
   * Class constructor
   */
  QEStringDialog (const QString& prompt, QWidget *parent = 0, const char *name = 0);

  /**
   * Returns the result string
   */
  QString result_string(void) const { return(input_text); }

public slots:

  /** 
   * Fetch result data from widgets
   */
  virtual void update_results(void);

 protected:

 void keyPressEvent(QKeyEvent*  kevent);

 private:

  /**
   * Creates and initializes keyboard short-cuts
   */
  void init_shortcuts(void); 

  QLineEdit* tekstirivi;
  QString input_text;
};

#endif
