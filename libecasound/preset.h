#ifndef INCLUDED_PRESET_H
#define INCLUDED_PRESET_H

#include <string>
#include <map>

#include "eca-chainop.h"
#include "eca-chain.h"
#include "sample-specs.h"
#include "samplebuffer.h"

using std::string;

class AUDIO_IO;
class GENERIC_CONTROLLER;

/**
 * Class for representing effect presets
 *
 * @author Arto Hamara
 * @author Kai Vehmanen
 */
class PRESET : public CHAIN_OPERATOR {

 private:

  SAMPLE_BUFFER* first_buffer;

  std::vector<SAMPLE_BUFFER*> buffers;
  std::vector<CHAIN*> chains;
  std::vector<std::string> param_names;
  std::vector<DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>* > param_objects;
  std::vector<int> param_arg_indices;
  std::vector<GENERIC_CONTROLLER*> gctrls_rep;
  bool parsed_rep;
  std::string parse_string_rep;
  std::string name_rep;

  void add_chain(void);
  PRESET& operator=(const PRESET& x) { return *this; }
  PRESET(const PRESET& x) { }

 public:

  virtual PRESET* clone(void);
  virtual PRESET* new_expr(void) { return(new PRESET(parse_string_rep)); }
  virtual ~PRESET (void);

  string name(void) const { return(name_rep); }
  void set_name(const std::string& v) { name_rep = v; }

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
  void parse(const std::string& formatted_string);
  
  /**
   * Whether preset data has been parsed
   */
  bool is_parsed(void) const { return(parsed_rep); }

  PRESET(void);
  PRESET(const std::string& formatted_string);
};

#endif
