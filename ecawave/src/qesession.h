#ifndef QESESSION_H
#define QESESSION_H

#include <string>
#include <memory>
#include <vector>

#include <qwidget.h>

#include <kvutils/definition_by_contract.h>
#include <ecasound/eca-audio-format.h>

class ECA_CONTROL;
class ECA_SESSION;

class QEFile;
class QEButtonRow;
class QEStatusBar;
class QENonblockingEvent;
class QVBoxLayout;

#include "resources.h"

/**
 * Ecawave session widget
 *
 * Top level user-interface object. This class provides global
 * functions like opening a new file and opening a new session
 * window.
 */
class QESession : public QWidget, public DEFINITION_BY_CONTRACT {
  Q_OBJECT

public slots:

  void timerEvent(QTimerEvent* e);

  void new_session(void);
  void new_file(void);
  void open_file(void);
  void save_event(void);
  void save_as_event(void);
  void play_event(void);
  void stop_event(void);
  void effect_event(void);
  void copy_event(void);
  void paste_event(void);
  void cut_event(void);
  void fade_in_event(void);
  void fade_out_event(void);

  void debug_event(void);

private slots:

  void position_update(void);
  void update_wave_data(void);

signals:

  void filename_changed(const string& filename);

 public:

  /**
   * Whether to use wave form cache
   */
  void toggle_wave_cache(bool v) { wcache_toggle_rep = v; }

  /**
   * Whether to force wave form cache refresh
   */
  void toggle_cache_refresh(bool v) { refresh_toggle_rep = v; }

  QESession (const string& filename = "", 
	     const ECA_AUDIO_FORMAT& frm = ECA_AUDIO_FORMAT(), 
	     bool use_wave_cache = true, 
	     bool force_refresh = false,
	     bool direct_mode = false, 
	     QWidget *parent = 0,
	     const char *name = 0);
  ~QESession(void);

 private:

  void init_layout(void);
  void prepare_event(void);
  void prepare_temp(void);
  void remove_temps(void);
  void copy_file(const string& a, const string& b);
  bool temp_file_created(void);

  enum { 
    state_orig_file, // original file open, not edited
    state_orig_direct, // orig. file open, direct-mode
    state_edit_file, // original and temp file open
    state_new_file,  // no original file, temp file open
    state_invalid    // unspecified
  } state_rep;

  string orig_filename_rep;
  string active_filename_rep;

  long int start_pos;
  long int sel_length;

  QEResources ecawaverc;
  vector<QESession*> child_sessions;
  
  QEFile* file;
  QEButtonRow* buttonrow;
  QEButtonRow* buttonrow2;
  QENonblockingEvent* nb_event;

  QVBoxLayout* vlayout;
  QEStatusBar* statusbar;

  bool refresh_toggle_rep, wcache_toggle_rep;
  bool direct_mode_rep;

  auto_ptr<ECA_CONTROL> auto_ectrl;
  auto_ptr<ECA_SESSION> auto_esession;

  ECA_CONTROL* ectrl;
  ECA_SESSION* esession;
};

#endif
