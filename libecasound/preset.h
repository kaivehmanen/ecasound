#ifndef INCLUDED_PRESET_H
#define INCLUDED_PRESET_H

#include <string>
#include <map>

#include <kvutils/definition_by_contract.h>

#include "eca-chainop.h"
#include "eca-chain.h"
#include "sample-specs.h"
#include "eca-chainsetup.h"
#include "samplebuffer.h"

class AUDIO_IO;

/**
 * Class for representing effect presets
 *
 * @author Arto Hamara
 * @author Kai Vehmanen
 */
class PRESET : public CHAIN_OPERATOR,
	       public DEFINITION_BY_CONTRACT {

 private:

  SAMPLE_BUFFER* first_buffer;

  vector<SAMPLE_BUFFER*> buffers;
  vector<CHAIN*> chains;

  //vector< pair<DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>*, int> > param_objects;
  //map<string, int> param_names; // maps parameter names to indexes
  vector< string > param_names;
  vector< DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>* > param_objects;
  vector< int > param_arg_indices;


/*    vector<DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>* > param_objects; */
/*    vector<int> param_numbers; */

  ECA_CHAINSETUP csetup;

  bool parsed_rep;
  string parse_string_rep;

  string name_rep;

  void add_chain(void);

  PRESET (const PRESET& x) : csetup("untitled", false) { }
  PRESET& operator=(const PRESET& x) { return *this; }

 public:

  virtual PRESET* clone(void) { return(new PRESET(parse_string_rep)); }
  virtual PRESET* new_expr(void) { return(new PRESET(parse_string_rep)); }
  virtual ~PRESET (void);

  string name(void) const { return(name_rep); }
  void set_name(const string& v) { name_rep = v; }

  /**
   * Connect input to chain
   */
  void connect_input(AUDIO_IO* input);

  virtual void init(SAMPLE_BUFFER* sbuf);
  virtual void process(void);
  virtual string parameter_names(void) const;
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  /**
   * Parse preset data from the formatted string given 
   * as argument.
   *
   * require:
   *  formatted_string.empty() == false
   * ensure:
   *  is_parsed() == true
   */
  void parse(const string& formatted_string);
  
  /**
   * Whether preset data has been parsed
   */
  bool is_parsed(void) const { return(parsed_rep); }

  PRESET(void);
  PRESET(const string& formatted_string);
};

#endif
