#ifndef _QEAUDIOFORMATDIALOG_H
#define _QEAUDIOFORMATDIALOG_H

#include <qwidget.h>
#include <qspinbox.h>

/**
 * Dialog for selecting audio format
 */
class QEAudioFormatInput : public QWidget {
  Q_OBJECT

 public:

  QEAudioFormatInput (QWidget *parent=0, const char *name=0);

  int result_bits(void) const;
  int result_channels(void) const;
  int result_srate(void) const;

public slots:

  void set_bits(int value);
  void set_channels(int value);
  void set_srate(int value);

  void enable_format(void);
  void disable_format(void);

 private:

  void init_layout(void);

  QSpinBox* bits_input;
  QSpinBox* channel_input;
  QSpinBox* srate_input;

};

#endif
