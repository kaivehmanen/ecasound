#ifndef INCLUDED_ECASOUNDC_H
#define INCLUDED_ECASOUNDC_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------
 * Reference on object
 */
typedef void * eci_handle_t;

/* ---------------------------------------------------------------------
 * Constructing and destructing                                       
 */

void eci_init(void);
eci_handle_t eci_init_r(void);

void eci_cleanup(void);
void eci_cleanup_r(eci_handle_t p);
 
/* ---------------------------------------------------------------------
 * Issuing EIAM commands 
 */

void eci_command(const char* cmd);
void eci_command_r(eci_handle_t p, const char* cmd);

void eci_command_float_arg(const char*, double arg);
void eci_command_float_arg_r(eci_handle_t p, const char*, double arg);

/* ---------------------------------------------------------------------
 * Getting return values 
 */

int eci_last_string_list_count(void);
int eci_last_string_list_count_r(eci_handle_t p);

const char* eci_last_string_list_item(int n);
const char* eci_last_string_list_item_r(eci_handle_t p, int n);

const char* eci_last_string(void);
const char* eci_last_string_r(eci_handle_t p);

double eci_last_float(void);
double eci_last_float_r(eci_handle_t p);

int eci_last_integer(void);
int eci_last_integer_r(eci_handle_t p);

long int eci_last_long_integer(void);
long int eci_last_long_integer_r(eci_handle_t p);

const char* eci_last_error(void);
const char* eci_last_error_r(eci_handle_t p);

const char* eci_last_type(void);
const char* eci_last_type_r(eci_handle_t p);

int eci_error(void);
int eci_error_r(eci_handle_t p);
 
/* --------------------------------------------------------------------- 
 * Events 
 */

int eci_events_available(void);
int eci_events_available_r(eci_handle_t p);

void eci_next_event(void);
void eci_next_event_r(eci_handle_t p);

const char* eci_current_event(void);
const char* eci_current_event_r(eci_handle_t p);

#ifdef __cplusplus
}
#endif

#endif
