#ifndef _ECA_IAMODE_PARSER_H
#define _ECA_IAMODE_PARSER_H

#include <map>
#include <string>

#include "eca-error.h"

/**
 * Class that handles registering and querying interactive mode commands.
 * @author Kai Vehmanen
 */
class ECA_IAMODE_PARSER {

  static map<string,int> cmd_map;
  static void register_commands(void);

 public:
  
  enum COMMANDS {
    ec_direct_option,
    // --
    ec_help,
    ec_exit,
    ec_start,
    ec_stop,
    ec_debug,
    // --
    ec_st_general,
    // --
    ec_rewind,
    ec_forward,
    ec_setpos,
    // --
    ec_cs_add,
    ec_cs_remove,
    ec_cs_select,
    ec_cs_index_select,
    ec_cs_load,
    ec_cs_save,
    ec_cs_save_as,
    ec_cs_edit,
    ec_cs_connect,
    ec_cs_disconnect,
    ec_cs_set,
    ec_cs_format,
    ec_cs_status,
    ec_cs_length,
    ec_cs_loop,
    // --
    ec_c_add,
    ec_c_select,
    ec_c_select_all,
    ec_c_remove,
    ec_c_clear,
    ec_c_name,
    ec_c_mute,
    ec_c_bypass,
    ec_c_forward,
    ec_c_rewind,
    ec_c_setpos,
    ec_c_status,
    // --
    ec_aio_add_input,
    ec_aio_add_output,
    ec_aio_remove,
    ec_aio_select,
    ec_aio_index_select,
    ec_aio_attach,
    ec_aio_status,
    ec_aio_forward,
    ec_aio_rewind,
    ec_aio_setpos,
    ec_aio_wave_edit,
    // --
    ec_cop_add,
    ec_cop_remove,
    ec_cop_select,  // not implemented
    ec_cop_set,
    ec_cop_add_controller,
    ec_cop_remove_controller, // not implemented
    ec_cop_status,
  };

  /**
   * Parse string mode command and act accordingly.
   */
  static const map<string,int>& registered_commands(void) { return(cmd_map); }

  /**
   * Parse string mode command and act accordingly.
   */
  void command(const string& cmd) throw(ECA_ERROR*);

  /** 
   * Execute actions 'action_id'.
   */
  virtual void action(int action_id, const vector<string>& args) = 0;

  bool action_requires_params(int id);
  bool action_requires_connected(int id);
  bool action_requires_selected_not_connected(int id);
  bool action_requires_selected(int id);

  ECA_IAMODE_PARSER(void) { register_commands(); }
  virtual ~ECA_IAMODE_PARSER(void) { }
};

void show_controller_help(void);
void show_controller_help_more(void);

#endif
