#ifndef INCLUDED_ECATOOLS_SIGNALVIEW_H
#define INCLUDED_ECATOOLS_SIGNALVIEW_H

/**
 * Type definitions
 */

struct ecasv_channel_stats {
  double last_peak;
  double drawn_peak;
  double max_peak;
  long int clipped_samples;
};

/**
 * Function declarations
 */

int main(int argc, char *argv[]);
void ecasv_parse_command_line(int argc, char *argv[]);
void ecasv_fill_defaults(void);
std::string ecasv_cop_to_string(CHAIN_OPERATOR* cop);
void ecasv_output_init(EFFECT_VOLUME_PEAK* cop);
void ecasv_output_cleanup(void);
void ecasv_print_vu_meters(EFFECT_VOLUME_PEAK* cop, std::vector<struct ecasv_channel_stats>* chstats);
void ecasv_update_chstats(std::vector<struct ecasv_channel_stats>* chstats, int ch, double value);
void ecasv_create_bar(double value, int barlen, unsigned char* barbuf);
void ecasv_print_usage(void);
void ecasv_signal_handler(int signum);

#endif
