#ifndef _QT_STRING_DIALOG_H
#define _QT_STRING_DIALOG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qlineedit.h>

class QEStringDialog : public QDialog {
  Q_OBJECT
 public:

  QEStringDialog (const QString& prompt, QWidget *parent=0, const char *name=0);

  QString resultString(void) { return(input_text); }

protected slots:

  void inputGiven(void);

 protected:

 void keyPressEvent(QKeyEvent*  kevent);

 private:

  void init_shortcuts(void); 

  QLineEdit* tekstirivi;
  QString input_text;
};

#endif
