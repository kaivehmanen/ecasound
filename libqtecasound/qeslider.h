#ifndef _QESLIDER_H
#define _QESLIDER_H

#include <qslider.h>
#include "qecontroller.h"

/**
 * Graphical slider for controlling effect parameters
 */
class QESlider : public QEController {
  Q_OBJECT

public:

  virtual string name(void) const { return("Graphical slider"); }
  virtual string parameter_names(void) const { return(""); }
  virtual parameter_type get_parameter(int param) const { return(0.f); }
  virtual void set_parameter(int param, parameter_type value) { }

  /**
   * Returns the current parameter value
   */
  virtual parameter_type value(void); 

  virtual CONTROLLER_SOURCE* clone(void) { return(new QESlider); }
  virtual CONTROLLER_SOURCE* new_expr(void) { return(new QESlider); }

  /**
   * Class constructor
   */
  QESlider(QWidget *parent = 0, const char *name = 0);

 private slots:

  void update_value(int v);

 private:

  QSlider* slider_rep;
  parameter_type value_rep;

  void init_layout(void);
};

#endif
