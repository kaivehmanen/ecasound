#ifndef QEBUTTONROW_H
#define QEBUTTONROW_H

#include <vector>

#include <qwidget.h>
#include <qlayout.h>
#include <qaccel.h>
#include <qfont.h>

#include <kvutils/definition_by_contract.h>

/**
 * User-interface widget for representing a button-row. Supports
 * fonts and keyboard shortcuts.
 */
class QEButtonRow : public QWidget, public DEFINITION_BY_CONTRACT {
  Q_OBJECT

 public:

  /**
   * Sets the default font for all QEButtonRow instances
   */
  static void set_default_font(const QFont&);

 private:

  static QFont default_font;
  
 public:

  /**
   * Set default font used for all button labels. If not set, default font is used.
   */
  void set_font(const QFont& v);

  /**
   * Add a new button and associate short-cut key 'key' to it.
   */
  void add_button(QButton*, int key);

  /**
   * Add a new button. Short-cut key 'key' and 'receiver - member' pair 
   * are connected to the added button.
   *
   * @param member You must use SLOT() macro for determinating this
   */
  void add_button(QButton*, int key, const QObject * receiver, const char * member);

  /**
   * Returns the most recently added button. This should be used
   * for connecting added buttons. 
   */
  QButton* last_button(void) const { return(buttons.back()); }

  /**
   * Constructor that takes a comma-separated list of button labels as its 
   * argument.
   */
  QEButtonRow (QWidget *parent = 0, const char *name = 0);

 private:

  QFont font_rep;
  QHBoxLayout* box;
  QAccel *accel;
  vector<QButton*> buttons;
};

#endif
