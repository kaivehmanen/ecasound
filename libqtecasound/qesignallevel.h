#ifndef _QESIGNALLEVEL_H
#define _QESIGNALLEVEL_H

#include <vector>
#include <deque>
#include <qsizepolicy.h>

#include "qechainoperator.h"

class QESignalLevel : public QEChainOperator {
  Q_OBJECT

public:

  virtual string name(void) const { return("Signallevel"); }

  virtual void init(SAMPLE_BUFFER* sbuf);
  virtual void process(void);

  virtual string parameter_names(void) const { return(""); }
  virtual CHAIN_OPERATOR::parameter_type get_parameter(int param) const { return(0.f); }
  virtual void set_parameter(int param, CHAIN_OPERATOR::parameter_type value) { }

  void timerEvent(QTimerEvent* e);
  void paintEvent(QPaintEvent* e);
  QSize sizeHint(void) const;
  QSizePolicy sizePolicy (void) const { return(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred)); }
  
  OPERATOR* clone(void)  { return new QESignalLevel(); }
  OPERATOR* new_expr(void)  { return new QESignalLevel(); }

  /**
   * Class constructor
   *
   * @param buffer_latency Latency between library and screen updates in buffers. 
   */
  QESignalLevel(int buffer_latency = 1, QWidget *parent = 0, const char *name = 0);

 private:

  int buffer_latency_rep;
  bool update_geometry_request;

  SAMPLE_BUFFER* buffer_rep;
  int channels_rep;
  vector<deque<CHAIN_OPERATOR::parameter_type> > rms_volume;
};

#endif
