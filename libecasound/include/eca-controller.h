#ifndef _ECA_CONTROLLER_H
#define _ECA_CONTROLLER_H

#include "eca-iamode-parser.h"
#include "eca-controller-objects.h"

class CHAIN_OPERATOR;

/**
 * Class for controlling the whole ecasound library
 * @author Kai Vehmanen
 */
class ECA_CONTROLLER : public ECA_CONTROLLER_OBJECTS,
		       public ECA_IAMODE_PARSER {

 public:

  /**
   * Parse string mode command and act accordingly.
   */
  void command(const string& cmd) throw(ECA_ERROR*);
 
  /** 
   * See ECA_IAMODE_PARSER
   */
  void action(int action_id, const vector<string>& args) throw(ECA_ERROR*);

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

  ECA_CONTROLLER (ECA_SESSION* psession);
  virtual ~ECA_CONTROLLER (void) { }
};

#endif
