// ------------------------------------------------------------------------
// eca-iamode-parser.cpp: Class that handles registering and querying 
//                        interactive mode commands.
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

map<string,int> ECA_IAMODE_PARSER::cmd_map_rep;

void ECA_IAMODE_PARSER::register_commands(void) {
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

  cmd_map_rep["cs-add"] = ec_cs_add;
  cmd_map_rep["cs-remove"] = ec_cs_remove;
  cmd_map_rep["cs-select"] = ec_cs_select;
  cmd_map_rep["cs-index-select"] = ec_cs_index_select;
  cmd_map_rep["cs-load"] = ec_cs_load;
  cmd_map_rep["cs-save"] = ec_cs_save;
  cmd_map_rep["cs-save-as"] = ec_cs_save_as;
  cmd_map_rep["cs-edit"] = ec_cs_edit;
  cmd_map_rep["cs-connect"] = ec_cs_connect;
  cmd_map_rep["cs-disconnect"] = ec_cs_disconnect;
  cmd_map_rep["cs-set"] = ec_cs_set;
  cmd_map_rep["cs-format"] = ec_cs_format;
  cmd_map_rep["cs-status"] = ec_cs_status;
  cmd_map_rep["cs-list"] = ec_cs_status;
  cmd_map_rep["cs-length"] = ec_cs_length;
  cmd_map_rep["cs-loop"] = ec_cs_loop;

  cmd_map_rep["c-add"] = ec_c_add;
  cmd_map_rep["c-select"] = ec_c_select;
  cmd_map_rep["c-deselect"] = ec_c_deselect;
  cmd_map_rep["c-select-all"] = ec_c_select_all;
  cmd_map_rep["c-select-add"] = ec_c_select_add;
  cmd_map_rep["c-remove"] = ec_c_remove;
  cmd_map_rep["c-clear"] = ec_c_clear;
  cmd_map_rep["c-name"] = ec_c_name;
  cmd_map_rep["c-mute"] = ec_c_mute;
  cmd_map_rep["c-bypass"] = ec_c_bypass;
  cmd_map_rep["c-forward"] = ec_c_forward;
  cmd_map_rep["c-fw"] = ec_c_forward;
  cmd_map_rep["c-rewind"] = ec_c_rewind;
  cmd_map_rep["c-rw"] = ec_c_rewind;
  cmd_map_rep["c-setpos"] = ec_c_setpos;
  cmd_map_rep["c-status"] = ec_c_status;
  cmd_map_rep["c-list"] = ec_c_status;

  cmd_map_rep["aio-add-input"] = ec_aio_add_input;
  cmd_map_rep["aio-add-output"] = ec_aio_add_output;
  cmd_map_rep["aio-select"] = ec_aio_select;
  cmd_map_rep["aio-select-input"] = ec_aio_select_input;
  cmd_map_rep["aio-select-output"] = ec_aio_select_output;
  cmd_map_rep["aio-index-select"] = ec_aio_index_select;
  cmd_map_rep["aio-attach"] = ec_aio_attach;
  cmd_map_rep["aio-remove"] = ec_aio_remove;
  cmd_map_rep["aio-status"] = ec_aio_status;
  cmd_map_rep["aio-list"] = ec_aio_status;
  cmd_map_rep["aio-forward"] = ec_aio_forward;
  cmd_map_rep["aio-rewind"] = ec_aio_rewind;
  cmd_map_rep["aio-setpos"] = ec_aio_setpos;
  cmd_map_rep["aio-wave-edit"] = ec_aio_wave_edit;
  cmd_map_rep["aio-register"] = ec_aio_register;

  cmd_map_rep["cop-add"] = ec_cop_add;
  cmd_map_rep["cop-remove"] = ec_cop_remove;
  cmd_map_rep["cop-select"] = ec_cop_select;
  cmd_map_rep["cop-set"] = ec_cop_set;
  cmd_map_rep["cop-status"] = ec_cop_status;
  cmd_map_rep["cop-list"] = ec_cop_status;
  cmd_map_rep["cop-register"] = ec_cop_register;

  cmd_map_rep["preset-register"] = ec_preset_register;
  cmd_map_rep["ladspa-register"] = ec_ladspa_register;

  cmd_map_rep["ctrl-add"] = ec_ctrl_add;
  cmd_map_rep["ctrl-remove"] = ec_ctrl_remove;
  cmd_map_rep["ctrl-status"] = ec_ctrl_status;
  cmd_map_rep["ctrl-register"] = ec_ctrl_register;

  cmd_map_rep["status"] = ec_st_general;
  cmd_map_rep["st"] = ec_st_general;
  cmd_map_rep["u"] = ec_st_general;

  cmd_map_rep["cstatus"] = ec_c_status;
  cmd_map_rep["cs"] = ec_c_status;
  cmd_map_rep["a"] = ec_c_status;

  cmd_map_rep["estatus"] = ec_cop_status;
  cmd_map_rep["es"] = ec_cop_status;
  cmd_map_rep["x"] = ec_cop_status;

  cmd_map_rep["fstatus"] = ec_aio_status;
  cmd_map_rep["fs"] = ec_aio_status;
  cmd_map_rep["l"] = ec_aio_status;

  cmd_map_rep["rewind"] = ec_rewind;
  cmd_map_rep["rw"] = ec_rewind;
  cmd_map_rep["forward"] = ec_forward;
  cmd_map_rep["fw"] = ec_forward;
  cmd_map_rep["setpos"] = ec_setpos;

  cmd_map_rep["dump-target"] = ec_dump_target;
  cmd_map_rep["dump-status"] = ec_dump_status;
  cmd_map_rep["dump-position"] = ec_dump_position;
  cmd_map_rep["dump-length"] = ec_dump_length;
  cmd_map_rep["dump-cs-status"] = ec_dump_cs_status;
  cmd_map_rep["dump-c-selected"] = ec_dump_c_selected;
  cmd_map_rep["dump-aio-selected"] = ec_dump_aio_selected;
  cmd_map_rep["dump-aio-position"] = ec_dump_aio_position;
  cmd_map_rep["dump-aio-length"] = ec_dump_aio_length;
  cmd_map_rep["dump-aio-open-state"] = ec_dump_aio_open_state;
  cmd_map_rep["dump-cop-value"] = ec_dump_cop_value;
}

bool ECA_IAMODE_PARSER::action_requires_params(int id) { 
  switch(id) {
  case ec_direct_option:
  case ec_debug:
  case ec_cs_add:
  case ec_cs_select:
  case ec_cs_index_select:
  case ec_cs_load: 
  case ec_cs_save_as: 
  case ec_cs_set:
  case ec_cs_format:
  case ec_cs_length:
  case ec_c_add:
  case ec_c_select:
  case ec_c_deselect:
  case ec_c_select_add:
  case ec_c_name:
  case ec_c_forward: 
  case ec_c_rewind: 
  case ec_c_setpos:
  case ec_aio_add_input:
  case ec_aio_select:
  case ec_aio_select_input:
  case ec_aio_select_output:
  case ec_aio_index_select:
  case ec_aio_forward:
  case ec_aio_rewind:
  case ec_aio_setpos:
  case ec_cop_add:
  case ec_cop_remove:
  case ec_cop_select:
  case ec_cop_set:
  case ec_ctrl_add:
  case ec_ctrl_remove:
  case ec_rewind:
  case ec_forward:
  case ec_setpos:
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
  case ec_c_forward:
  case ec_c_rewind: 
  case ec_c_setpos: 
  case ec_rewind:
  case ec_forward:
  case ec_setpos:

    return(true);
    
  default: 
    break;
  }
  return(false);
}

bool ECA_IAMODE_PARSER::action_requires_selected(int id) {
  switch(id) {
  case ec_direct_option:
  case ec_cs_remove: 
  case ec_cs_edit:
  case ec_cs_save: 
  case ec_cs_save_as: 
  case ec_cs_connect: 
  case ec_cs_set:
  case ec_cs_length:
  case ec_cs_loop:
  case ec_c_remove:
  case ec_c_clear:
  case ec_c_name:
  case ec_c_mute:
  case ec_c_bypass:
  case ec_c_forward: 
  case ec_c_rewind: 
  case ec_c_setpos:
  case ec_c_status:
  case ec_aio_add_input:
  case ec_aio_add_output:
  case ec_aio_select:
  case ec_aio_select_input:
  case ec_aio_select_output:
  case ec_aio_index_select:
  case ec_aio_remove:
  case ec_aio_attach:
  case ec_aio_status:
  case ec_aio_forward:
  case ec_aio_rewind:
  case ec_aio_setpos:
  case ec_aio_wave_edit:
  case ec_cop_add:
  case ec_cop_remove:
  case ec_cop_select:
  case ec_cop_set:
  case ec_cop_status:
  case ec_ctrl_add:
  case ec_ctrl_remove:
  case ec_ctrl_status:

    return(true);

  default: break;
  }
  
  return(false);
}

bool ECA_IAMODE_PARSER::action_requires_selected_not_connected(int id) { 
  switch(id) {
  case ec_direct_option:
  case ec_cs_remove:
  case ec_cs_length:
  case ec_cs_loop:
  case ec_cs_set:
  case ec_c_add:
  case ec_c_remove:
  case ec_c_name:
  case ec_aio_add_input:
  case ec_aio_add_output:
  case ec_aio_remove:
  case ec_aio_attach:
  case ec_aio_forward:
  case ec_aio_rewind:
  case ec_aio_setpos:
  case ec_aio_wave_edit:

    return(true);
    
  default: 
    break;
  }
  return(false);

}

bool ECA_IAMODE_PARSER::action_requires_selected_audio_object(int id) { 
  switch(id) {
  case ec_aio_remove:
  case ec_aio_attach:
  case ec_aio_forward:
  case ec_aio_rewind:
  case ec_aio_setpos:
  case ec_aio_wave_edit:
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
  mitem << "\n'status','st','u' - General status info";
  mitem << "\n'cs-status' - Chainsetup status";
  mitem << "\n'c-status', 'cstatus','cs','a' - Chain status";
  mitem << "\n'cop-status', 'estatus', 'es','x' - Chain operator status";
  mitem << "\n'ctrl-status' - Controller status"; 
  mitem << "\n'aio-status', 'fstatus', 'fs','l' - Audio input/output status";

  mitem << "\n--- see ecasound-iam(1) manual page for more info -----------------\n";
  //  mitem << "\n'chain chainname', 'c chainname' - Enable/disable the the chain 'chainname'";
 
  ecadebug->msg(mitem.to_string());
}
