#ifndef QECHAINOPEVENT_H
#define QECHAINOPEVENT_H

#include <string>

#include <qdialog.h>
#include <qwidget.h>

#include <ecasound/eca-chainop.h>
#include <ecasound/qechainoperatorinput.h>

#include "qenonblockingevent.h"

/**
 * Process audio data with a chain operator 
 * provided by libecasound
 */
class QEChainopEvent : public QDialog, 
		       public QENonblockingEvent {
  Q_OBJECT

public slots:

  void process(void);
  void preview(void); 

signals:

  void finished(void);

private:

  enum { preview_mode, process_mode, invalid } mode;

  void init_layout(void);

  ECA_CONTROLLER* ectrl;
  string input_rep, output_rep;
  long int start_pos_rep;
  long int length_rep;
  QEChainOperatorInput* copinput;

public:

  virtual long int position_in_samples(void) const;
  virtual void restart(long int start_pos, long int length);

  QSize sizeHint(void) const { return(QSize(400,400)); }
  QEChainopEvent (ECA_CONTROLLER* ctrl, 
		  const string& input,
		  const string& output,
		  long int start_pos, 
		  long int length,
		  QWidget *parent = 0, 
		  const char *name = 0);
};

#endif
