#ifndef _ECA_QT_CHAINSETUP_H
#define _ECA_QT_CHAINSETUP_H

#include <vector>

#include <qwidget.h>
#include <qlayout.h>
#include <qlistview.h>

class AUDIO_IO;
class ECA_CONTROLLER;
class ECA_CHAINSETUP;
class QEButtonRow;
class QEChain;

class QEChainsetup : public QWidget
{
  Q_OBJECT
public:
  QEChainsetup (ECA_CONTROLLER* econtrol, const ECA_CHAINSETUP* setup, QWidget *parent=0, const char *name=0);
  
public slots:
 void update_filesetuplist(bool clean);
 void update_chainsetuplist(void);
 void update_chainsetuplist_clean(void);
 // --
 void button_add_file(void);
 void button_remove_file(void);
 void button_chainselect(void);
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
 void child_closed(void);

private slots:
  void not_implemented(void);

signals:
 void widget_closed(void);

protected:

  void closeEvent(QCloseEvent *);
  void timerEvent(QTimerEvent *);

 private:

  void update_filesetup (const vector<AUDIO_IO*>& flist);
  void update_filesetup_clean (const vector<AUDIO_IO*>& flist);

  bool is_filesetup_highlighted(void) const;
  void select_highlighted_filesetup(void);
  bool is_chain_highlighted(void) const;
  void select_highlighted_chain(void);

  void init_filesetuplist(void);
  void init_chainsetuplist(void);

  void init_gen_buttons(void);
  void init_file_buttons(void);
  void init_chain_buttons(void);

  QBoxLayout* topLayout;
  QEButtonRow* gen_buttons;
  QEButtonRow* file_buttons;
  QEButtonRow* chain_buttons;

  ECA_CONTROLLER* ctrl;
  const ECA_CHAINSETUP* chainsetup;
  QListView* filesetupview;
  QListView* chainsetupview;
  QEChain* child_chain;

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
