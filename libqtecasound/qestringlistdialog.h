#ifndef INCLUDED_QESTRINGLISTDIALOG_H
#define INCLUDED_QESTRINGLISTDIALOG_H

#include <vector>
#include <string>
#include <qdialog.h>

class QListBox;

class QEStringListDialog : public QDialog {
  Q_OBJECT
 public:

  /**
   * Class constructor
   */
  QEStringListDialog (const QString& prompt, const vector<string> items, QWidget *parent = 0, const char *name = 0);

  /**
   * Returns the result string
   */
  vector<string> result(void) const { return(items_rep); }

public slots:

  /** 
   * Fetch result data from widgets
   */
  virtual void update_results(void);

 private:

  void init_shortcuts(void);
  void init_layout(void);

  vector<string> items_rep, alternatives_rep;
  QListBox* item_input_repp;
};

#endif
