#ifndef _QEAUDIOFORMATDIALOG_H
#define _QEAUDIOFORMATDIALOG_H

#include "eca-audio-format.h"
#include "qeinput.h"

class QSpinBox;

/**
 * Input widget for selecting audio format
 */
class QEAudioFormatInput : public QEInput,
			   public ECA_AUDIO_FORMAT {
  Q_OBJECT

 public:

  QEAudioFormatInput (QWidget *parent = 0, const char *name = 0);

public slots:

  /**
   * Enables/activates the inputs
   */
   virtual void enable(void);
 
  /**
   * Disables the inputs (user can't change)
   */
  virtual void disable(void);

  /** 
   * Fetch result data from widgets
   */
  virtual void update_results(void);

 private:

  /**
   * Creates and initializes widget layout
   */
  void init_layout(void);

  QSpinBox* bits_input;
  QSpinBox* channel_input;
  QSpinBox* srate_input;
};

#endif
