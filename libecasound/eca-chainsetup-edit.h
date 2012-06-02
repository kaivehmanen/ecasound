// ------------------------------------------------------------------------
// eca-chainsetup-edit.h: Chainsetup edit object
// Copyright (C) 2009,2012 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
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

#ifndef INCLUDED_ECA_CHAINSETUP_EDIT_H
#define INCLUDED_ECA_CHAINSETUP_EDIT_H

class CHAIN;
class ECA_CHAINSETUP;

namespace ECA {
  enum Chainsetup_edit_type {
  
    edit_c_bypass = 0,
    edit_c_muting,
    edit_cop_add,
    edit_cop_bypass,
    edit_cop_set_param,
    edit_ctrl_add,
    edit_ctrl_set_param,
  };

  /*
   * Chainsetup edit objects are defined for all operations 
   * that can be performed either from the real-time engine
   * (if modifying chainsetup that is currently run), or
   * from the non-real-time control thread (modifying 
   * selected but not running chainsetup).
   *
   * Using edit objects avoids duplicated code to describe
   * and parse the needed actions in both ECA_ENGINE and 
   * ECA_CONTROL.
   */

  struct chainsetup_edit {

    Chainsetup_edit_type type;
    const ECA_CHAINSETUP *cs_ptr;

    /* FIXME: should a version tag be added as way to invalidate
     *        edit objects in case chainsetup is modified */

    union {
      struct {
	int chain;     /**< @see ECA_CHAINSETUP::get_chain_index() */
	int val;
      } c_bypass;

      struct {
	int chain;     /**< @see ECA_CHAINSETUP::get_chain_index() */
	int val;
      } c_muting;

      struct {
	int chain;     /**< @see ECA_CHAINSETUP::get_chain_index() */
	int op;        /**< @see CHAIN::set_parameter() */
	int param;     /**< @see CHAIN::set_parameter() */
	double value;  /**< @see CHAIN::set_parameter() */
      } cop_set_param;

      struct {
	int chain;     /**< @see ECA_CHAINSETUP::get_chain_index() */
	int op;        /**< @see CHAIN::bypass_operator() */
	int bypass;    /**< @see CHAIN::bypass_operator() */
      } cop_bypass;

      struct {
        int chain;     /**< @see ECA_CHAINSETUP::get_chain_index() */
      } c_generic_param;

      struct {
	int chain;     /**< @see ECA_CHAINSETUP::get_chain_index() */
	int op;        /**< @see CHAIN::set_controller_parameter() */
	int param;     /**< @see CHAIN::set_controller_parameter() */
	double value;  /**< @see CHAIN::set_controller_parameter() */
      } ctrl_set_param;
    } m;

    bool need_chain_reinit;
    std::string param; /**< arbitrary string parameter, semantics 
                          depend on 'type' */
  };

  typedef struct chainsetup_edit chainsetup_edit_t;
}

#endif /* INCLUDED_ECA_CHAINSETUP_EDIT_H */
