#ifndef INCLUDED_QESTATUSBAR_H
#define INCLUDED_QESTATUSBAR_H

#include <string>
#include <memory>

#include <qstatusbar.h>

#include <ecasound/eca-audio-time.h>

class ECA_CONTROL;

/**
 * Ecawave statusbar that displays info about currently edited audio file
 */
class QEStatusBar : public QStatusBar {
  Q_OBJECT

public slots:

  void current_position(ECA_AUDIO_TIME pos) { curpos_rep = pos; }
  void visible_area(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end);
  void marked_area(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end);
  void toggle_editing(bool v) { editing_rep = v; }
  void update(void);

 private:

  ECA_CONTROL* ectrl_repp;
  ECA_AUDIO_TIME curpos_rep, vstartpos_rep, vendpos_rep, mstartpos_rep, mendpos_rep;
  bool editing_rep;

 public:

  QEStatusBar (ECA_CONTROL* ctrl, QWidget *parent = 0, const char *name = 0);
};

#endif

