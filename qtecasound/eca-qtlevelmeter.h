#ifndef _ECA_QT_LEVELMETER_H
#define _ECA_QT_LEVELMETER_H

//#include <qprogressbar.h>
// #include <qlcdnumber.h>

class QELevelMeter : public QWidget
{

public:
  QELevelMeter(double max_value, QWidget *parent=0, const char *name=0);
  
  void set_value(double value);      // between 0 and 'max'

 protected:

  void	timerEvent( QTimerEvent * );
  void paintEvent( QPaintEvent * );

 private:

  double curval, maxval, lastnew, lastvalue; 
};

#endif
