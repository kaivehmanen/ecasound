#ifndef INCLUDED_ECA_CONTROL_H
#define INCLUDED_ECA_CONTROL_H

#include "eca-iamode-parser.h"
#include "eca-control-dump.h"

class CHAIN_OPERATOR;

/**
 * Class for controlling the whole ecasound library
 * @author Kai Vehmanen
 */
class ECA_CONTROL : public ECA_CONTROL_DUMP,
		    public ECA_IAMODE_PARSER {

 private:

  void direct_command(const string& cmd);

 public:

  /**
   * Parse string mode command and act accordingly.
   */
  void command(const string& cmd) throw(ECA_ERROR&);
 
  /** 
   * See ECA_IAMODE_PARSER
   */
  void action(int action_id, const vector<string>& args) throw(ECA_ERROR&);

  // -------------------------------------------------------------------
  // Session info / output to ecadebug
  // -------------------------------------------------------------------
  
  void print_general_status(void);

  /**
   * Return info about chainsetups
   */
  string chainsetup_status(void) const;

  /**
   * Return info about current chain status
   *
   * require:
   *  is_selected() == true
   *  selected_chains().size() > 0
   */
  string chain_status(void) const;

  /**
   * Return info about inputs and outputs
   */
  string aio_status(void) const;

  /**
   * Return info about chain operators (selected chainsetup)
   *
   * require:
   *  is_selected() == true
   */
  string chain_operator_status(void) const;

  /**
   * Return info about controllers (selected chainsetup)
   *
   * require:
   *  is_selected() == true
   */
  string controller_status(void) const;

  void aio_register(void) const; 
  void cop_register(void) const; 
  void preset_register(void) const; 
  void ladspa_register(void) const; 
  void ctrl_register(void) const; 

  ECA_CONTROL (ECA_SESSION* psession);
  virtual ~ECA_CONTROL (void) { }
};

#endif
