#ifndef INCLUDED_ECA_CONTROL_H
#define INCLUDED_ECA_CONTROL_H

#include "eca-iamode-parser.h"
#include "eca-control-objects.h"
#include "eca-control-dump.h"

class CHAIN_OPERATOR;

/**
 * Class for controlling the whole ecasound library
 * @author Kai Vehmanen
 */
class ECA_CONTROL : public ECA_CONTROL_OBJECTS,
		    public ECA_IAMODE_PARSER {

 private:

  std::vector<std::string> action_args_rep;
  double action_arg_f_rep; 
  bool action_arg_f_set_rep;
  ECA_CONTROL_DUMP ctrl_dump_rep;

  void action(int action_id);
  void direct_command(const std::string& cmd);
  void set_action_argument(const std::vector<std::string>& s);
  void set_action_argument(double v);
  void clear_action_arguments(void);
  double first_argument_as_number(void) const;

 public:

  /**
   * Parses a string containing set of ecasound interactive mode (EIAM)
   * commands and acts accordingly.
   */
  void command(const std::string& cmd);

  /**
   * A special version of 'command()' which parses a string-float-arg 
   * pair. The string argument is required to contain exactly one EIAM 
   * command, while the float argument contains one numerical parameter.
   */
  void command_float_arg(const std::string& cmd, double arg);
 
  /**
   * See ECA_IAMODE_PARSER for detailed decsription of 'action_id'.
   */
   void action(int action_id, const std::vector<std::string>& args);

  // -------------------------------------------------------------------
  // Session info
  // -------------------------------------------------------------------
  
  void print_general_status(void);

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

  // -------------------------------------------------------------------
  // Session info / output to ecadebug
  // -------------------------------------------------------------------
  
  void print_last_value(void);
  void print_last_error(void);

  // -------------------------------------------------------------------
  // Constructors and dtors
  // -------------------------------------------------------------------

  ECA_CONTROL (ECA_SESSION* psession);
  ~ECA_CONTROL (void);
};

#endif
