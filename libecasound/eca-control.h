#ifndef INCLUDED_ECA_CONTROL_H
#define INCLUDED_ECA_CONTROL_H

#include <string>
#include <vector>

#include "eca-iamode-parser.h"
#include "eca-control-objects.h"
#include "eca-control-dump.h"
#include "sample-specs.h"
#include "eca-chainsetup-edit.h"

class CHAIN_OPERATOR;
class ECA_CHAINSETUP;
class ECA_OBJECT_MAP;

/**
 * Class for controlling the whole ecasound library
 *
 * Related design patters: Facade (GoF185)
 *
 * @author Kai Vehmanen
 */
class ECA_CONTROL : public ECA_CONTROL_OBJECTS,
		    public ECA_IAMODE_PARSER {


 public:

  /** @name Constructors and dtors */
  /*@{*/

  ECA_CONTROL (ECA_SESSION* psession);
  ~ECA_CONTROL (void);

  /*@}*/

  /** @name Public functions for issuing command */
  /*@{*/

  /**
   * Parses and executes a string containing a single Ecasound 
   * Interactive Mode (EIAM) command and its arguments.
   *
   * Result of the command can be queried with last_value_to_string().
   */
  void command(const std::string& cmd_and_args);

  /**
   * A special version of 'command()' which parses a command taking 
   * a single double parameter.
   *
   * Result of the command can be queried with last_value_to_string().
   */
  void command_float_arg(const std::string& cmd, double arg);

  /**
   * See ECA_IAMODE_PARSER for detailed decsription of 'action_id'.
   *
   * Result of the command can be queried with last_value_to_string().
   */
  void action(int action_id, const std::vector<std::string>& args);

  /*@}*/

  // -------------------------------------------------------------------

  /** @name Public functions / execute edit objects */
  /*@{*/


  bool execute_edit_on_connected(const ECA::chainsetup_edit_t& edit);
  bool execute_edit_on_selected(const ECA::chainsetup_edit_t& edit, int index = -1);

  /*@}*/

  // -------------------------------------------------------------------

  /** @name Public functions for getting session information */
  /*@{*/

  /**
   * Return info about chainsetups
   */
  std::string chainsetup_status(void) const;

  /**
   * Return info about current chain status
   *
   * require:
   *  is_selected() == true
   *  selected_chains().size() > 0
   */
  std::string chain_status(void) const;

  /**
   * Return info about inputs and outputs
   */
  std::string aio_status(void) const;

  /**
   * Return info about chain operators (selected chainsetup)
   *
   * require:
   *  is_selected() == true
   */
  std::string chain_operator_status(void) const;

  /**
   * Return info about controllers (selected chainsetup)
   *
   * require:
   *  is_selected() == true
   */
  std::string controller_status(void) const;

  void aio_register(void); 
  void cop_register(void);
  void preset_register(void); 
  void ladspa_register(void);
  void ctrl_register(void);

  void operator_descriptions_helper(const ECA_OBJECT_MAP& arg, std::string* result);
  void cop_descriptions(void);
  void preset_descriptions(void);
  void ladspa_descriptions(bool use_id);
  void ctrl_descriptions(void);

  /*@}*/

  /** @name Public functions printing status information */
  /*@{*/

  void print_last_value(void);
  std::string last_value_to_string(void);

  /*@}*/

 private:

  std::vector<std::string> action_args_rep;
  double action_arg_f_rep; 
  bool action_arg_f_set_rep;
  bool action_ok;
  bool action_reconnect;
  bool action_restart;
  bool wellformed_mode_rep;
  ECA_CONTROL_DUMP ctrl_dump_rep;

  void action(int action_id);
  void check_action_preconditions(int action_id);
  void chainsetup_option(const std::string& cmd);
  void set_action_argument(const std::string& s);
  void set_action_argument(const std::vector<std::string>& s);
  void set_action_argument(double v);
  void clear_action_arguments(void);
  double first_action_argument_as_float(void) const;
  std::string first_action_argument_as_string(void) const;
  int first_action_argument_as_int(void) const;
  long int first_action_argument_as_long_int(void) const;
  SAMPLE_SPECS::sample_pos_t first_action_argument_as_samples(void) const;
  const std::vector<std::string>& action_arguments_as_vector(void) const;

  std::string chainsetup_details_to_string(const ECA_CHAINSETUP* cs) const;
};

#endif
