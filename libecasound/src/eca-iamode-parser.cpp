// ------------------------------------------------------------------------
// eca-iamode-parser.cpp: Class that handles registering and querying 
//                        interactive mode commands.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include "kvutils.h"

#include "eca-iamode-parser.h"
#include "eca-debug.h"

map<string,int> ECA_IAMODE_PARSER::cmd_map;

void ECA_IAMODE_PARSER::register_commands(void) {
  cmd_map["help"] = ec_help;
  cmd_map["?"] = ec_help;
  cmd_map["h"] = ec_help;

  cmd_map["quit"] = ec_exit;
  cmd_map["q"] = ec_exit;
   
  cmd_map["start"] = ec_start;
  cmd_map["t"] = ec_start;
  cmd_map["stop"] = ec_stop;
  cmd_map["s"] = ec_stop;
  cmd_map["run"] = ec_run;

  cmd_map["debug"] = ec_debug;

  cmd_map["cs-add"] = ec_cs_add;
  cmd_map["cs-remove"] = ec_cs_remove;
  cmd_map["cs-select"] = ec_cs_select;
  cmd_map["cs-index-select"] = ec_cs_index_select;
  cmd_map["cs-load"] = ec_cs_load;
  cmd_map["cs-save"] = ec_cs_save;
  cmd_map["cs-save-as"] = ec_cs_save_as;
  cmd_map["cs-edit"] = ec_cs_edit;
  cmd_map["cs-connect"] = ec_cs_connect;
  cmd_map["cs-disconnect"] = ec_cs_disconnect;
  cmd_map["cs-set"] = ec_cs_set;
  cmd_map["cs-format"] = ec_cs_format;
  cmd_map["cs-status"] = ec_cs_status;
  cmd_map["cs-length"] = ec_cs_length;
  cmd_map["cs-loop"] = ec_cs_loop;

  cmd_map["c-add"] = ec_c_add;
  cmd_map["c-select"] = ec_c_select;
  cmd_map["c-deselect"] = ec_c_deselect;
  cmd_map["c-select-all"] = ec_c_select_all;
  cmd_map["c-select-add"] = ec_c_select_add;
  cmd_map["c-remove"] = ec_c_remove;
  cmd_map["c-clear"] = ec_c_clear;
  cmd_map["c-name"] = ec_c_name;
  cmd_map["c-mute"] = ec_c_mute;
  cmd_map["c-bypass"] = ec_c_bypass;
  cmd_map["c-forward"] = ec_c_forward;
  cmd_map["c-fw"] = ec_c_forward;
  cmd_map["c-rewind"] = ec_c_rewind;
  cmd_map["c-rw"] = ec_c_rewind;
  cmd_map["c-setpos"] = ec_c_setpos;
  cmd_map["c-status"] = ec_c_status;

  cmd_map["aio-add-input"] = ec_aio_add_input;
  cmd_map["aio-add-output"] = ec_aio_add_output;
  cmd_map["aio-select"] = ec_aio_select;
  cmd_map["aio-index-select"] = ec_aio_index_select;
  cmd_map["aio-attach"] = ec_aio_attach;
  cmd_map["aio-remove"] = ec_aio_remove;
  cmd_map["aio-status"] = ec_aio_status;
  cmd_map["aio-forward"] = ec_aio_forward;
  cmd_map["aio-rewind"] = ec_aio_rewind;
  cmd_map["aio-setpos"] = ec_aio_setpos;
  cmd_map["aio-wave-edit"] = ec_aio_wave_edit;

  cmd_map["cop-add"] = ec_cop_add;
  cmd_map["cop-remove"] = ec_cop_remove;
  cmd_map["cop-select"] = ec_cop_select;
  cmd_map["cop-set"] = ec_cop_set;
  cmd_map["cop-add-controller"] = ec_cop_add_controller;
  cmd_map["cop-remove-controller"] = ec_cop_remove_controller;
  cmd_map["cop-status"] = ec_cop_status;

  cmd_map["status"] = ec_st_general;
  cmd_map["st"] = ec_st_general;
  cmd_map["u"] = ec_st_general;

  cmd_map["cstatus"] = ec_c_status;
  cmd_map["cs"] = ec_c_status;
  cmd_map["a"] = ec_c_status;

  cmd_map["estatus"] = ec_cop_status;
  cmd_map["es"] = ec_cop_status;
  cmd_map["x"] = ec_cop_status;

  cmd_map["fstatus"] = ec_aio_status;
  cmd_map["fs"] = ec_aio_status;
  cmd_map["l"] = ec_aio_status;

  cmd_map["rewind"] = ec_rewind;
  cmd_map["rw"] = ec_rewind;
  cmd_map["forward"] = ec_forward;
  cmd_map["fw"] = ec_forward;
  cmd_map["setpos"] = ec_setpos;
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
  case ec_aio_add_output:
  case ec_aio_select:
  case ec_aio_index_select:
  case ec_aio_forward:
  case ec_aio_rewind:
  case ec_aio_setpos:
  case ec_cop_add:
  case ec_cop_remove:
  case ec_cop_select:
  case ec_cop_set:
  case ec_cop_add_controller:
  case ec_cop_remove_controller:
  case ec_rewind:
  case ec_forward:
  case ec_setpos:

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
  case ec_cop_add_controller:
  case ec_cop_remove_controller:
  case ec_cop_status:

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
  case ec_c_clear:
  case ec_c_name:
  case ec_aio_add_input:
  case ec_aio_add_output:
  case ec_aio_remove:
  case ec_aio_attach:
  case ec_aio_forward:
  case ec_aio_rewind:
  case ec_aio_setpos:
  case ec_aio_wave_edit:
  case ec_cop_add:
  case ec_cop_remove:
  case ec_cop_add_controller:
  case ec_cop_remove_controller:

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
 
  mitem << "\n'aio-status', 'fstatus', 'fs','l' - Audio input/output status";

  mitem << "\n--- see ecasound-iam(1) manual page for more info -----------------\n";
  //  mitem << "\n'chain chainname', 'c chainname' - Enable/disable the the chain 'chainname'";
 
  ecadebug->msg(mitem.to_string());
}
