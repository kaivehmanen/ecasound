#ifndef INCLUDED_ECASOUNDC_H
#define INCLUDED_ECASOUNDC_H

#ifdef __cplusplus
extern "C" {
#endif

void ecac_init(void);
void ecac_cleanup(void);
  
void ecac_command(const char* command);
const char* ecac_error_string(void);

/**
 * Fills the structure pointed by 'status'. Provides
 * info like "running/stopped", position, active objects,
 * selected options, etc.
 */
/*  void eca_engine_status(eca_engine_status_t* status); */

#ifdef __cplusplus
}
#endif

#endif
