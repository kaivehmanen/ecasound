#ifndef _QEOBJECTINPUT_H
#define _QEOBJECTINPUT_H

#include "qeinput.h"

class QLabel;
class QWidget;
class ECA_OBJECT;
class ECA_OBJECT_MAP;

/**
 * Object map input widget
 */
class QEObjectMap : public QEInput {
  Q_OBJECT
 public:

  /**
   * Class constructor
   */
  QEObjectMap (ECA_OBJECT_MAP* omap, QWidget *parent = 0, const char *name = 0);

  /**
   * Returns the currently selected object
   */
  ECA_OBJECT* result(void) const { return(object_rep); }

  virtual bool class_invariant(void) const { return(omap_rep != 0); }

public slots:

  virtual void update_results(void);

private slots:

  /**
   * Update object matching 'index'
   *
   * require:
   *  index >= 0
   * ensure:
   *  object_rep != 0
   */
  void update_object(int index);

 private:

  ECA_OBJECT_MAP* omap_rep;
  ECA_OBJECT* object_rep;
  int selected_index;
  bool empty_rep;

  /**
   * Creates and initializes widget layout
   */
  void init_layout(void);
};

#endif
