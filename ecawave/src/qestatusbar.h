#ifndef QESTATUSBAR_H
#define QESTATUSBAR_H

#include <string>
#include <memory>

#include <qstatusbar.h>

#include <ecasound/eca-audio-time.h>

class ECA_CONTROLLER;

/**
 * Ecawave statusbar that displays info about currently edited audio file
 */
class QEStatusBar : public QStatusBar {
  Q_OBJECT

public slots:

  void current_position(ECA_AUDIO_TIME pos) { curpos = pos; }
  void visible_area(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end);
  void marked_area(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end);
  void toggle_editing(bool v) { editing_rep = v; }
  void update(void);

 private:

  ECA_CONTROLLER* ectrl;
  string filename_rep;
  ECA_AUDIO_TIME curpos, vstartpos, vendpos, mstartpos, mendpos;
  bool editing_rep;

 public:

  QEStatusBar (ECA_CONTROLLER* ctrl, const string& filename, QWidget *parent = 0, const char *name = 0);
};

#endif

