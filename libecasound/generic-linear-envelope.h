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
    void init(parameter_type step);

    string parameter_names(void) const;
    void set_parameter(int param, parameter_type value);
    parameter_type get_parameter(int param) const;

    parameter_type value(void);
    string name(void) const { return("Generic linear envelope"); }
    
    GENERIC_LINEAR_ENVELOPE(void); 
    GENERIC_LINEAR_ENVELOPE* clone(void)  { return new GENERIC_LINEAR_ENVELOPE(*this); }
    GENERIC_LINEAR_ENVELOPE* new_expr(void)  { return new GENERIC_LINEAR_ENVELOPE(); }
    
private:
    
    vector<parameter_type> pos_rep;
    vector<parameter_type> val_rep;
    parameter_type curpos, curval;
    int curstage;
    string param_names_rep;
    
    void set_param_count(int params);
};

#endif
