#ifndef INCLUDED_QE_CHAINSETUP_H
#define INCLUDED_QE_CHAINSETUP_H

#include <vector>

#include <qwidget.h>
#include <qlayout.h>
#include <qlistview.h>

class AUDIO_IO;
class ECA_CONTROLLER;
class ECA_CHAINSETUP;
class QEButtonRow;
class QEChain;

/**
 * Qt widget representing an ecasound chainsetup.
 */
class QEChainsetup : public QWidget
{
  Q_OBJECT
public:
  QEChainsetup (ECA_CONTROLLER* econtrol, QWidget *parent=0, const char *name=0);
  
public slots:
 void update_chain_list(void);
 void update_chain_list_clean(void);

 // --
 void button_add_file(void);
 void button_remove_file(void);
 void button_select_chain(void);
/*   void init_wave_edit(void); */
 // --
 void button_add_chain(void);
 void button_remove_chain(void);
 void button_chain_muting(void);
 void button_chain_bypass(void);

private slots:
  void not_implemented(void);

signals:
 void widget_closed(void);

protected:

  void closeEvent(QCloseEvent *);
  void timerEvent(QTimerEvent *);

 private:

  bool is_chain_highlighted(void) const;
  void select_highlighted_chain(void);

  void init_chain_list(void);
  void init_buttons(void);

  QBoxLayout* top_layout_repp;
  QEButtonRow* buttons_repp;

  ECA_CONTROLLER* ctrl_repp;

  QListView* chain_list_repp;

  QString current_dir_rep;
  QString cs_namestring_rep;
  QString cs_modestring_rep, cs_posstring_rep, cs_statusstring_rep;
  QString cs_chainstring_rep;
  QString cs_rtstring_rep;
  QString cs_format_rep;
};

#endif
