#ifndef INCLUDED_PRESET_H
#define INCLUDED_PRESET_H

#include <string>
#include <vector>

class PRESET_impl;
class CHAIN;

#include "samplebuffer.h"
#include "eca-chainop.h"

/**
 * Class for representing effect presets
 *
 * @author Arto Hamara
 * @author Kai Vehmanen
 */
class PRESET : public CHAIN_OPERATOR {

 public:

  PRESET(void);
  PRESET(const std::string& formatted_string);

  virtual PRESET* clone(void);
  virtual PRESET* new_expr(void);
  virtual ~PRESET (void);

  virtual std::string name(void) const;
  virtual std::string description(void) const;

  void set_name(const std::string& v);

  virtual void init(SAMPLE_BUFFER* sbuf);
  virtual void process(void);
  virtual std::string parameter_names(void) const;
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;
  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd);

  void parse(const std::string& formatted_string);
  
  bool is_parsed(void) const;

 private:

  PRESET_impl* impl_repp;
  SAMPLE_BUFFER* first_buffer;
  std::vector<SAMPLE_BUFFER*> buffers;
  std::vector<CHAIN*> chains;

  bool is_preset_option(const std::string& arg) const;
  void add_chain(void);
  void extend_pardesc_vector(int number);
  void parse_preset_option(const std::string& arg);
  void parse_operator_option(const std::string& arg);
  void set_preset_defaults(const std::vector<std::string>& args);
  void set_preset_param_names(const std::vector<string>& args);
  void set_preset_lower_bounds(const std::vector<std::string>& args);
  void set_preset_upper_bounds(const std::vector<std::string>& args);
  void set_preset_toggles(const std::vector<std::string>& args);

  PRESET& operator=(const PRESET& x) { return *this; }
  PRESET(const PRESET& x) { }
};

#endif


