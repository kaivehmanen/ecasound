#ifndef INCLUDED_ECA_IAMODE_PARSER_H
#define INCLUDED_ECA_IAMODE_PARSER_H

#include <map>
#include <string>
#include <vector>

#include "eca-error.h"

/**
 * Class that handles registering and querying interactive mode commands.
 * @author Kai Vehmanen
 */
class ECA_IAMODE_PARSER {

 protected:

  static map<string,int> cmd_map_rep;
  static void register_commands(void);

  enum Commands {
    ec_unknown,
    ec_direct_option,
    // --
    ec_help,
    ec_exit,
    ec_start,
    ec_stop,
    ec_run,
    ec_debug,
    // --
    ec_st_general,
    ec_engine_status,
    // --
    ec_cs_add,
    ec_cs_remove,
    ec_cs_list,
    ec_cs_select,
    ec_cs_selected,
    ec_cs_index_select,
    ec_cs_load,
    ec_cs_save,
    ec_cs_save_as,
    ec_cs_edit,
    ec_cs_is_valid,
    ec_cs_connect,
    ec_cs_connected,
    ec_cs_disconnect,
    ec_cs_set_param,
    ec_cs_set_audio_format,
    ec_cs_status,
    ec_cs_rewind,
    ec_cs_forward,
    ec_cs_set_position,
    ec_cs_get_position,
    ec_cs_get_length,
    ec_cs_set_length,
    ec_cs_toggle_loop,
    // --
    ec_c_add,
    ec_c_remove,
    ec_c_list,
    ec_c_select,
    ec_c_selected,
    ec_c_select_all,
    ec_c_select_add,
    ec_c_index_select,
    ec_c_deselect,
    ec_c_clear,
    ec_c_rename,
    ec_c_mute,
    ec_c_bypass,
    ec_c_forward,
    ec_c_rewind,
    ec_c_setpos,
    ec_c_status,
    // --
    ec_aio_register,
    ec_aio_status,
    // --
    ec_ai_add,
    ec_ai_remove,
    ec_ai_list,
    ec_ai_select,
    ec_ai_selected,
    ec_ai_index_select,
    ec_ai_attach,
    ec_ai_status,
    ec_ai_forward,
    ec_ai_rewind,
    ec_ai_set_position,
    ec_ai_get_position,
    ec_ai_get_length,
    ec_ai_get_format,
    ec_ai_wave_edit,
    // --
    ec_ao_add,
    ec_ao_remove,
    ec_ao_list,
    ec_ao_select,
    ec_ao_selected,
    ec_ao_index_select,
    ec_ao_attach,
    ec_ao_status,
    ec_ao_forward,
    ec_ao_rewind,
    ec_ao_set_position,
    ec_ao_get_position,
    ec_ao_get_length,
    ec_ao_get_format,
    ec_ao_wave_edit,
    // --
    ec_cop_add,
    ec_cop_remove,
    ec_cop_list,
    ec_cop_select,
    ec_cop_selected,
    ec_cop_set,
    ec_cop_status,
    ec_cop_register,
    ec_copp_list,
    ec_copp_select,
    ec_copp_selected,
    ec_copp_set,
    ec_copp_get,
    // --
    ec_ladspa_register,
    ec_preset_register,
    // --
    ec_ctrl_add,
    ec_ctrl_remove,
    ec_ctrl_list,
    ec_ctrl_select,
    ec_ctrl_selected,
    ec_ctrl_status,
    ec_ctrl_register,
    ec_ctrlp_list,     /* FIXME: not implemented */
    ec_ctrlp_select,   /* FIXME: not implemented */
    ec_ctrlp_selected, /* FIXME: not implemented */
    ec_ctrlp_set,      /* FIXME: not implemented */
    ec_ctrlp_get,      /* FIXME: not implemented */
    // --
    ec_int_cmd_list,
    ec_int_version_string,
    ec_int_version_lib_current,
    ec_int_version_lib_revision,
    ec_int_version_lib_age,
    // --
    ec_dump_target,
    ec_dump_status,
    ec_dump_position,
    ec_dump_length,
    ec_dump_cs_status,
    ec_dump_c_selected,
    ec_dump_ai_selected,
    ec_dump_ai_position,
    ec_dump_ai_length,
    ec_dump_ai_open_state,
    ec_dump_ao_selected,
    ec_dump_ao_position,
    ec_dump_ao_length,
    ec_dump_ao_open_state,
    ec_dump_cop_value
  };

 public:

  static const map<string,int>& registered_commands(void) { return(cmd_map_rep); }
  static vector<string> registered_commands_list(void);

  bool action_requires_params(int id);
  bool action_requires_connected(int id);
  bool action_requires_selected_not_connected(int id);
  bool action_requires_selected(int id);
  bool action_requires_selected_audio_input(int id);
  bool action_requires_selected_audio_output(int id);

  ECA_IAMODE_PARSER(void) { register_commands(); }
  virtual ~ECA_IAMODE_PARSER(void);
};

void show_controller_help(void);
void show_controller_help_more(void);

#endif
