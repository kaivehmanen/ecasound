#ifndef _ECA_QT_CHAINSETUP_H
#define _ECA_QT_CHAINSETUP_H

#include <qwidget.h>
#include <qlayout.h>
#include <qlistview.h>

class AUDIO_IO;
class ECA_CONTROLLER;
class ECA_CHAINSETUP;

#include "eca-qtwaveform.h"

class QEChainsetup : public QWidget
{
  Q_OBJECT
public:
  QEChainsetup (ECA_CONTROLLER* econtrol, const ECA_CHAINSETUP* setup, QWidget *parent=0, const char *name=0);
  
public slots:
 void update_filesetuplist(bool clean = true);
 void update_chainsetuplist(void);
 void update_chainsetuplist_clean(void);
 void update_layout(void);
 // --
 void button_add_file(void);
 void button_remove_file(void);
 void button_chainselect(void);
 void init_waveform(void);
 void init_waveedit(void);
 // --
 void init_chainview(void);
 void init_chainview(QListViewItem*);
 void button_add_chain(void);
 void button_remove_chain(void);
 void button_chain_muting(void);
 void button_chain_bypass(void);
 // --
 void close_session(void);

private slots:
  void not_implemented(void);

signals:
  void close_waveforms(void); 

protected:

  void timerEvent( QTimerEvent * );
  //  void keyPressEvent(QKeyEvent*  kevent);

 private:

  void update_filesetup (const vector<AUDIO_IO*>& flist, const QString& selname);
  void update_filesetup_clean (const vector<AUDIO_IO*>& flist, const QString& selname);

  bool is_filesetup_highlighted(void) const;
  void select_highlighted_filesetup(void);
  bool is_chain_highlighted(void) const;
  void select_highlighted_chain(void);

  void init_filesetuplist(void);
  void init_chainsetuplist(void);

  void init_gen_buttons(void);
  void init_file_buttons(void);
  void init_chain_buttons(void);

  void init_shortcuts(void);

  QBoxLayout* topLayout;
  QBoxLayout* gen_buttons;
  QBoxLayout* file_buttons;
  QBoxLayout* chain_buttons;

  ECA_CONTROLLER* ctrl;
  const ECA_CHAINSETUP* chainsetup;

  QListView* filesetupview;
  QListView* chainsetupview;

  QListViewItem* newitem;
  vector<AUDIO_IO*>::size_type aiod_sizet;

  QString current_dir;

  QString cs_namestring;
  QString cs_modestring, cs_posstring, cs_statusstring;
  QString cs_chainstring;
  QString cs_rtstring;
  QString cs_format;
};

#endif


