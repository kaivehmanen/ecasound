// ------------------------------------------------------------------------
// eca-iamode-parser.cpp: Class that handles registering and querying 
//                        interactive mode commands.
// Copyright (C) 1999-2001 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <map>
#include <vector>
#include <string>

#include <kvutils/message_item.h>

#include "eca-iamode-parser.h"
#include "eca-debug.h"

std::map<std::string,int> ECA_IAMODE_PARSER::cmd_map_rep;

ECA_IAMODE_PARSER::~ECA_IAMODE_PARSER(void) { }

std::vector<std::string> ECA_IAMODE_PARSER::registered_commands_list(void) {
  std::vector<std::string> cmdlist;
  const std::map<std::string,int>& map_ref = ECA_IAMODE_PARSER::registered_commands();
  std::map<std::string,int>::const_iterator p = map_ref.begin();
  while (p != map_ref.end()) {
    cmdlist.push_back(p->first);
    ++p;
  }
  return(cmdlist);
}

void ECA_IAMODE_PARSER::register_commands(void) {
  register_commands_misc();
  register_commands_cs();
  register_commands_c();
  register_commands_aio();
  register_commands_ai();
  register_commands_ao();
  register_commands_cop();
  register_commands_copp();
  register_commands_ctrl();
  register_commands_dump();
}

void ECA_IAMODE_PARSER::register_commands_misc(void) {
  cmd_map_rep["help"] = ec_help;
  cmd_map_rep["?"] = ec_help;
  cmd_map_rep["h"] = ec_help;

  cmd_map_rep["quit"] = ec_exit;
  cmd_map_rep["q"] = ec_exit;
   
  cmd_map_rep["start"] = ec_start;
  cmd_map_rep["t"] = ec_start;
  cmd_map_rep["stop"] = ec_stop;
  cmd_map_rep["s"] = ec_stop;
  cmd_map_rep["run"] = ec_run;

  cmd_map_rep["debug"] = ec_debug;

  cmd_map_rep["engine-status"] = ec_engine_status;
  cmd_map_rep["status"] = ec_cs_status;
  cmd_map_rep["st"] = ec_cs_status;
  cmd_map_rep["cs"] = ec_c_status;
  cmd_map_rep["es"] = ec_cop_status;
  cmd_map_rep["fs"] = ec_aio_status;

  cmd_map_rep["int-cmd-list"] = ec_int_cmd_list;
  cmd_map_rep["int-version-string"] = ec_int_version_string;
  cmd_map_rep["int-version-lib-current"] = ec_int_version_lib_current;
  cmd_map_rep["int-version-lib-revision"] = ec_int_version_lib_revision;
  cmd_map_rep["int-version-lib-age"] = ec_int_version_lib_age;

  cmd_map_rep["preset-register"] = ec_preset_register;
  cmd_map_rep["ladspa-register"] = ec_ladspa_register;
}

void ECA_IAMODE_PARSER::register_commands_cs(void) {
  cmd_map_rep["cs-add"] = ec_cs_add;
  cmd_map_rep["cs-remove"] = ec_cs_remove;
  cmd_map_rep["cs-list"] = ec_cs_list;
  cmd_map_rep["cs-select"] = ec_cs_select;
  cmd_map_rep["cs-selected"] = ec_cs_selected;
  cmd_map_rep["cs-index-select"] = ec_cs_index_select;
  cmd_map_rep["cs-iselect"] = ec_cs_index_select;
  cmd_map_rep["cs-load"] = ec_cs_load;
  cmd_map_rep["cs-save"] = ec_cs_save;
  cmd_map_rep["cs-save-as"] = ec_cs_save_as;
  cmd_map_rep["cs-edit"] = ec_cs_edit;
  cmd_map_rep["cs-is-valid"] = ec_cs_is_valid;
  cmd_map_rep["cs-connect"] = ec_cs_connect;
  cmd_map_rep["cs-connected"] = ec_cs_connected;
  cmd_map_rep["cs-disconnect"] = ec_cs_disconnect;
  cmd_map_rep["cs-set-param"] = ec_cs_set_param;
  cmd_map_rep["cs-set-audio-format"] = ec_cs_set_audio_format;
  cmd_map_rep["cs-status"] = ec_cs_status;
  cmd_map_rep["cs-rewind"] = ec_cs_rewind;
  cmd_map_rep["rewind"] = ec_cs_rewind;
  cmd_map_rep["rw"] = ec_cs_rewind;
  cmd_map_rep["cs-forward"] = ec_cs_forward;
  cmd_map_rep["forward"] = ec_cs_forward;
  cmd_map_rep["fw"] = ec_cs_forward;
  cmd_map_rep["cs-setpos"] = ec_cs_set_position;
  cmd_map_rep["cs-set-position"] = ec_cs_set_position;
  cmd_map_rep["cs-set-position-samples"] = ec_cs_set_position_samples;
  cmd_map_rep["setpos"] = ec_cs_set_position;
  cmd_map_rep["set-position"] = ec_cs_set_position;
  cmd_map_rep["cs-getpos"] = ec_cs_get_position;
  cmd_map_rep["cs-get-position"] = ec_cs_get_position;
  cmd_map_rep["cs-get-position-samples"] = ec_cs_get_position_samples;
  cmd_map_rep["getpos"] = ec_cs_get_position;
  cmd_map_rep["get-position"] = ec_cs_get_position;
  cmd_map_rep["cs-get-length"] = ec_cs_get_length;
  cmd_map_rep["cs-get-length-samples"] = ec_cs_get_length_samples;
  cmd_map_rep["get-length"] = ec_cs_get_length;
  cmd_map_rep["cs-set-length"] = ec_cs_set_length;
  cmd_map_rep["cs-set-length-samples"] = ec_cs_set_length_samples;
  cmd_map_rep["cs-toggle-loop"] = ec_cs_toggle_loop;
  cmd_map_rep["cs-option"] = ec_cs_option;
}

void ECA_IAMODE_PARSER::register_commands_c(void) {
  cmd_map_rep["c-add"] = ec_c_add;
  cmd_map_rep["c-remove"] = ec_c_remove;
  cmd_map_rep["c-list"] = ec_c_list;
  cmd_map_rep["c-select"] = ec_c_select;
  cmd_map_rep["c-selected"] = ec_c_selected;
  cmd_map_rep["c-index-select"] = ec_c_index_select;
  cmd_map_rep["c-iselect"] = ec_c_index_select;
  cmd_map_rep["c-deselect"] = ec_c_deselect;
  cmd_map_rep["c-selected"] = ec_c_selected;
  cmd_map_rep["c-select-all"] = ec_c_select_all;
  cmd_map_rep["c-select-add"] = ec_c_select_add;
  cmd_map_rep["c-clear"] = ec_c_clear;
  cmd_map_rep["c-rename"] = ec_c_rename;
  cmd_map_rep["c-mute"] = ec_c_mute;
  cmd_map_rep["c-bypass"] = ec_c_bypass;
  cmd_map_rep["c-status"] = ec_c_status;
}

void ECA_IAMODE_PARSER::register_commands_aio(void) {
  cmd_map_rep["aio-register"] = ec_aio_register;
  cmd_map_rep["aio-status"] = ec_aio_status;
}

void ECA_IAMODE_PARSER::register_commands_ai(void) {
  cmd_map_rep["ai-add"] = ec_ai_add;
  cmd_map_rep["ai-remove"] = ec_ai_remove;
  cmd_map_rep["ai-list"] = ec_ai_list;
  cmd_map_rep["ai-select"] = ec_ai_select;
  cmd_map_rep["ai-index-select"] = ec_ai_index_select;
  cmd_map_rep["ai-iselect"] = ec_ai_index_select;
  cmd_map_rep["ai-selected"] = ec_ai_selected;
  cmd_map_rep["ai-attach"] = ec_ai_attach;
  cmd_map_rep["ai-status"] = ec_ai_status;
  cmd_map_rep["ai-forward"] = ec_ai_forward;
  cmd_map_rep["ai-rewind"] = ec_ai_rewind;
  cmd_map_rep["ai-setpos"] = ec_ai_set_position;
  cmd_map_rep["ai-set-position"] = ec_ai_set_position;
  cmd_map_rep["ai-set-position-samples"] = ec_ai_set_position_samples;
  cmd_map_rep["ai-getpos"] = ec_ai_get_position;
  cmd_map_rep["ai-get-position"] = ec_ai_get_position;
  cmd_map_rep["ai-get-position-samples"] = ec_ai_get_position_samples;
  cmd_map_rep["ai-get-length"] = ec_ai_get_length;
  cmd_map_rep["ai-get-length-samples"] = ec_ai_get_length_samples;
  cmd_map_rep["ai-get-format"] = ec_ai_get_format;
  cmd_map_rep["ai-wave-edit"] = ec_ai_wave_edit;
}

void ECA_IAMODE_PARSER::register_commands_ao(void) {
  cmd_map_rep["ao-add"] = ec_ao_add;
  cmd_map_rep["ao-list"] = ec_ao_list;
  cmd_map_rep["ao-select"] = ec_ao_select;
  cmd_map_rep["ao-index-select"] = ec_ao_index_select;
  cmd_map_rep["ao-iselect"] = ec_ao_index_select;
  cmd_map_rep["ao-selected"] = ec_ao_selected;
  cmd_map_rep["ao-attach"] = ec_ao_attach;
  cmd_map_rep["ao-remove"] = ec_ao_remove;
  cmd_map_rep["ao-status"] = ec_ao_status;
  cmd_map_rep["ao-forward"] = ec_ao_forward;
  cmd_map_rep["ao-rewind"] = ec_ao_rewind;
  cmd_map_rep["ao-setpos"] = ec_ao_set_position;
  cmd_map_rep["ao-set-position"] = ec_ao_set_position;
  cmd_map_rep["ao-set-position-samples"] = ec_ao_set_position_samples;
  cmd_map_rep["ao-getpos"] = ec_ao_get_position;
  cmd_map_rep["ao-get-position"] = ec_ao_get_position;
  cmd_map_rep["ao-get-position-samples"] = ec_ao_get_position_samples;
  cmd_map_rep["ao-get-length"] = ec_ao_get_length;
  cmd_map_rep["ao-get-length-samples"] = ec_ao_get_length_samples;
  cmd_map_rep["ao-get-format"] = ec_ao_get_format;
  cmd_map_rep["ao-wave-edit"] = ec_ao_wave_edit;
}

void ECA_IAMODE_PARSER::register_commands_cop(void) {
  cmd_map_rep["cop-add"] = ec_cop_add;
  cmd_map_rep["cop-remove"] = ec_cop_remove;
  cmd_map_rep["cop-list"] = ec_cop_list;
  cmd_map_rep["cop-select"] = ec_cop_select;
  cmd_map_rep["cop-index-select"] = ec_cop_select;
  cmd_map_rep["cop-iselect"] = ec_cop_select;
  cmd_map_rep["cop-register"] = ec_cop_register;
  cmd_map_rep["cop-selected"] = ec_cop_selected;
  cmd_map_rep["cop-set"] = ec_cop_set;
  cmd_map_rep["cop-status"] = ec_cop_status;
}

void ECA_IAMODE_PARSER::register_commands_copp(void) {
  cmd_map_rep["copp-list"] = ec_copp_list;
  cmd_map_rep["copp-select"] = ec_copp_select;
  cmd_map_rep["copp-index-select"] = ec_copp_select;
  cmd_map_rep["copp-iselect"] = ec_copp_select;
  cmd_map_rep["copp-selected"] = ec_copp_selected;
  cmd_map_rep["copp-set"] = ec_copp_set;
  cmd_map_rep["copp-get"] = ec_copp_get;
}

void ECA_IAMODE_PARSER::register_commands_ctrl(void) {
  cmd_map_rep["ctrl-add"] = ec_ctrl_add;
  cmd_map_rep["ctrl-remove"] = ec_ctrl_remove;
  cmd_map_rep["ctrl-list"] = ec_ctrl_list;
  cmd_map_rep["ctrl-select"] = ec_ctrl_select;
  cmd_map_rep["ctrl-index-select"] = ec_ctrl_select;
  cmd_map_rep["ctrl-iselect"] = ec_ctrl_select;
  cmd_map_rep["ctrl-register"] = ec_ctrl_register;
  cmd_map_rep["ctrl-selected"] = ec_ctrl_selected;
  cmd_map_rep["ctrl-status"] = ec_ctrl_status;
}

void ECA_IAMODE_PARSER::register_commands_dump(void) {
  cmd_map_rep["dump-target"] = ec_dump_target;
  cmd_map_rep["dump-status"] = ec_dump_status;
  cmd_map_rep["dump-position"] = ec_dump_position;
  cmd_map_rep["dump-length"] = ec_dump_length;
  cmd_map_rep["dump-cs-status"] = ec_dump_cs_status;
  cmd_map_rep["dump-c-selected"] = ec_dump_c_selected;
  cmd_map_rep["dump-ai-selected"] = ec_dump_ai_selected;
  cmd_map_rep["dump-ai-position"] = ec_dump_ai_position;
  cmd_map_rep["dump-ai-length"] = ec_dump_ai_length;
  cmd_map_rep["dump-ai-open-state"] = ec_dump_ai_open_state;
  cmd_map_rep["dump-ao-selected"] = ec_dump_ao_selected;
  cmd_map_rep["dump-ao-position"] = ec_dump_ao_position;
  cmd_map_rep["dump-ao-length"] = ec_dump_ao_length;
  cmd_map_rep["dump-ao-open-state"] = ec_dump_ao_open_state;
  cmd_map_rep["dump-cop-value"] = ec_dump_cop_value;
}

bool ECA_IAMODE_PARSER::action_requires_params(int id) { 
  switch(id) {
  case ec_debug:

  case ec_cs_add:
  case ec_cs_select:
  case ec_cs_index_select:
  case ec_cs_load: 
  case ec_cs_save_as: 
  case ec_cs_set_param:
  case ec_cs_set_audio_format:
  case ec_cs_set_length:
  case ec_cs_set_length_samples:
  case ec_cs_rewind:
  case ec_cs_forward:
  case ec_cs_set_position:
  case ec_cs_option:

  case ec_c_add:
  case ec_c_select:
  case ec_c_index_select:
  case ec_c_deselect:
  case ec_c_select_add:
  case ec_c_rename:

  case ec_ai_add:
  case ec_ai_select:
  case ec_ai_index_select:
  case ec_ai_forward:
  case ec_ai_rewind:
  case ec_ai_set_position:
  case ec_ai_set_position_samples:

  case ec_ao_add:
  case ec_ao_select:
  case ec_ao_index_select:
  case ec_ao_forward:
  case ec_ao_rewind:
  case ec_ao_set_position:
  case ec_ao_set_position_samples:

  case ec_cop_add:
  case ec_cop_select:
  case ec_cop_set:

  case ec_copp_select:
  case ec_copp_set:

  case ec_ctrl_add:
  case ec_ctrl_select:

  case ec_dump_target:
  case ec_dump_cop_value:

    return(true);
    
  default: 
    break;
  }
  return(false);
}

bool ECA_IAMODE_PARSER::action_requires_connected(int id) { 
  switch(id) {
  case ec_start:
  case ec_run:

  case ec_cs_disconnect:
    return(true);
    
  default: 
    break;
  }
  return(false);
}

bool ECA_IAMODE_PARSER::action_requires_selected(int id) {
  switch(id) {

  case ec_cs_remove: 
  case ec_cs_edit:
  case ec_cs_is_valid:
  case ec_cs_save: 
  case ec_cs_save_as: 
  case ec_cs_connect: 
  case ec_cs_set_param:
  case ec_cs_rewind:
  case ec_cs_forward:
  case ec_cs_set_position:
  case ec_cs_set_position_samples:
  case ec_cs_get_position:
  case ec_cs_get_position_samples:
  case ec_cs_get_length:
  case ec_cs_get_length_samples:
  case ec_cs_set_length:
  case ec_cs_set_length_samples:
  case ec_cs_toggle_loop:
  case ec_cs_option:

  case ec_c_remove:
  case ec_c_clear:
  case ec_c_rename:
  case ec_c_mute:
  case ec_c_bypass:
  case ec_c_status:
  case ec_c_list:

  case ec_aio_status:

  case ec_ai_add:
  case ec_ai_select:
  case ec_ai_selected:
  case ec_ai_index_select:
  case ec_ai_remove:
  case ec_ai_attach:
  case ec_ai_status:
  case ec_ai_forward:
  case ec_ai_rewind:
  case ec_ai_set_position:
  case ec_ai_set_position_samples:
  case ec_ai_get_position:
  case ec_ai_get_position_samples:
  case ec_ai_get_length:
  case ec_ai_get_length_samples:
  case ec_ai_get_format:
  case ec_ai_wave_edit:

  case ec_ao_add:
  case ec_ao_select:
  case ec_ao_selected:
  case ec_ao_index_select:
  case ec_ao_remove:
  case ec_ao_attach:
  case ec_ao_status:
  case ec_ao_forward:
  case ec_ao_rewind:
  case ec_ao_set_position:
  case ec_ao_set_position_samples:
  case ec_ao_get_position:
  case ec_ao_get_position_samples:
  case ec_ao_get_length:
  case ec_ao_get_length_samples:
  case ec_ao_get_format:
  case ec_ao_wave_edit:

  case ec_cop_add:
  case ec_cop_select:
  case ec_cop_selected:
  case ec_cop_set:
  case ec_cop_status:

  case ec_copp_select:
  case ec_copp_selected:
  case ec_copp_set:

  case ec_ctrl_add:
  case ec_ctrl_select:
  case ec_ctrl_selected:
  case ec_ctrl_status:

    return(true);

  default: break;
  }
  
  return(false);
}

bool ECA_IAMODE_PARSER::action_requires_selected_not_connected(int id) { 
  switch(id) {
  case ec_cs_remove:
  case ec_cs_set_length:
  case ec_cs_set_length_samples:
  case ec_cs_toggle_loop:
  case ec_cs_set_param:
  case ec_cs_set_position_samples:
  case ec_cs_option:

  case ec_c_add:
  case ec_c_remove:
  case ec_c_rename:
  case ec_c_clear:

  case ec_ai_add:
  case ec_ai_remove:
  case ec_ai_attach:
  case ec_ai_forward:
  case ec_ai_rewind:
  case ec_ai_set_position:
  case ec_ai_set_position_samples:
  case ec_ai_wave_edit:

  case ec_ao_add:
  case ec_ao_remove:
  case ec_ao_attach:
  case ec_ao_forward:
  case ec_ao_rewind:
  case ec_ao_set_position:
  case ec_ao_set_position_samples:
  case ec_ao_wave_edit:

    return(true);
    
  default: 
    break;
  }
  return(false);

}

bool ECA_IAMODE_PARSER::action_requires_selected_audio_input(int id) { 
  switch(id) {
  case ec_ai_remove:
  case ec_ai_attach:
  case ec_ai_forward:
  case ec_ai_rewind:
  case ec_ai_set_position:
  case ec_ai_set_position_samples:
  case ec_ai_get_position:
  case ec_ai_get_position_samples:
  case ec_ai_selected:
  case ec_ai_get_length:
  case ec_ai_get_length_samples:
  case ec_ai_get_format:
  case ec_ai_wave_edit:
    return(true);
    
  default: 
    break;
  }
  return(false);

}

bool ECA_IAMODE_PARSER::action_requires_selected_audio_output(int id) { 
  switch(id) {
  case ec_ao_remove:
  case ec_ao_attach:
  case ec_ao_forward:
  case ec_ao_rewind:
  case ec_ao_set_position:
  case ec_ao_set_position_samples:
  case ec_ao_get_position:
  case ec_ao_get_position_samples:
  case ec_ao_selected:
  case ec_ao_get_length:
  case ec_ao_get_length_samples:
  case ec_ao_get_format:
  case ec_ao_wave_edit:
    return(true);
    
  default: 
    break;
  }
  return(false);

}

void show_controller_help(void) {
  MESSAGE_ITEM mitem; 

  mitem << "\n-------------------------------------------------------------------";
  mitem << "\n ecasound interactive-mode - command reference";
  mitem << "\n-------------------------------------------------------------------";

  mitem << "\n'q' - Quits ecasound";
  mitem << "\n'start', 't' - Processing is started (play)";
  mitem << "\n'stop', 's' - Stops processing"; 
  mitem << "\n'rewind time-in-seconds', 'rw time-in-seconds' - Rewind";
  mitem << "\n'forward time-in-seconds', 'fw time-in-seconds' - Forward";
  mitem << "\n'setpos time-in-seconds' - Sets the current position to 'time-in-seconds' seconds from the beginning.";
  mitem << "\n'engine-status' - Engine status";
  mitem << "\n'cs-status', 'st' - Chainsetup status";
  mitem << "\n'c-status', 'cs' - Chain status";
  mitem << "\n'cop-status', 'es' - Chain operator status";
  mitem << "\n'ctrl-status' - Controller status"; 
  mitem << "\n'aio-status', 'fs' - Audio input/output status";

  mitem << "\n--- see ecasound-iam(1) manual page for more info -----------------\n";
  //  mitem << "\n'chain chainname', 'c chainname' - Enable/disable the the chain 'chainname'";
 
  ecadebug->msg(mitem.to_string());
}
