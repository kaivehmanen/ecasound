#ifndef INCLUDED_GENERIC_LINEAR_ENVELOPE_H
#define INCLUDED_GENERIC_LINEAR_ENVELOPE_H

#include <string>
#include <vector>

#include "ctrl-source.h"

/**
 * Generic multi-stage linear envelope
 */
class GENERIC_LINEAR_ENVELOPE : public CONTROLLER_SOURCE {

public:
    void init(parameter_t step);

    std::string parameter_names(void) const;
    void set_parameter(int param, parameter_t value);
    parameter_t get_parameter(int param) const;

    parameter_t value(void);
    std::string name(void) const { return("Generic linear envelope"); }
    
    GENERIC_LINEAR_ENVELOPE(void); 
    GENERIC_LINEAR_ENVELOPE* clone(void) const { return new GENERIC_LINEAR_ENVELOPE(*this); }
    GENERIC_LINEAR_ENVELOPE* new_expr(void) const { return new GENERIC_LINEAR_ENVELOPE(); }
    
private:
    
    std::vector<parameter_t> pos_rep;
    std::vector<parameter_t> val_rep;
    parameter_t curpos, curval;
    int curstage;
    std::string param_names_rep;
    
    void set_param_count(int params);
};

#endif
