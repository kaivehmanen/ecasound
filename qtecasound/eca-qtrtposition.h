#ifndef INCLUDED_QE_RTPOSITION_H
#define INCLUDED_QE_RTPOSITION_H

#include <qslider.h>

class QERuntimePosition : public QSlider
{
  Q_OBJECT
public:
  QERuntimePosition (double length, QWidget *parent = 0, const char *name = 0);
  
  bool does_widget_have_control(void) const;

public slots:

   void length_in_seconds(double seconds);
   void position_in_seconds(double seconds);
   void control_back_to_parent(void);
 
private slots:

    void change_position_from_widget(void);
    void mouse_active(void);

signals:

    void position_changed_from_widget(double new_pos);

 private:

    bool widget_control_rep;
    int last_normally_changed_rep;
    double totallen_rep, position_rep;
};

#endif
