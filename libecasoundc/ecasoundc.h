#ifndef INCLUDED_ECASOUNDC_H
#define INCLUDED_ECASOUNDC_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------
 * Constructing and destructing                                       
 */

void eci_init(void);
void eci_cleanup(void);
 
/* ---------------------------------------------------------------------
 * Issuing EIAM commands 
 */

void eci_command(const char* cmd);
void eci_command_float_arg(const char*, double arg);

/* ---------------------------------------------------------------------
 * Getting return values 
 */

int eci_last_string_list_count(void);
const char* eci_last_string_list_item(int n);
const char* eci_last_string(void);
double eci_last_float(void);
int eci_last_integer(void);
long int eci_last_long_integer(void);
const char* eci_last_error(void);
const char* eci_last_type(void);
int eci_error(void);
 
/* --------------------------------------------------------------------- 
 * Events 
 */

int eci_events_available(void);
void eci_next_event(void);
const char* eci_current_event(void);

#ifdef __cplusplus
}
#endif

#endif
