#ifndef INCLUDED_SAMPLE_SPECS_H
#define INCLUDED_SAMPLE_SPECS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/**
 * Sample value defaults and constants.
 */
namespace SAMPLE_SPECS {
  
  /**
   * Type used to represent one sample value; should
   * be a floating point value (floating-point type)
   */
  typedef float sample_t;

  /**
   * Type used to represent position in sample 
   * frames (signed integer).
   */
#if defined _ISOC99_SOURCE || defined _ISOC9X_SOURCE || defined __GNUG__
  typedef long long int sample_pos_t;
#else
  typedef long int sample_pos_t;
#endif

  /**
   * Type used to represent sample rate values (signed integer).
   */
  typedef long int sample_rate_t;

  /**
   * Type used to identify individual channels (signed integer).
   */
  typedef int channel_t;

  static const sample_rate_t sample_rate_default = 44100;
  static const channel_t channel_count_default = 2;

  static const sample_t silent_value = 0.0f;     // do not change!
  static const sample_t max_amplitude = 1.0f;
  static const sample_t impl_max_value = silent_value + max_amplitude;
  static const sample_t impl_min_value = silent_value - max_amplitude;

  static const channel_t ch_left = 0;
  static const channel_t ch_right = 1;

#ifdef WORDS_BIGENDIAN
  static const bool is_system_littleendian = false;
#else
  static const bool is_system_littleendian = true;
#endif

  static const sample_t s16_to_st_constant = (32768.0 / max_amplitude); // 2^15
  static const sample_t s24_to_st_constant = (8388608.0 / max_amplitude); // 2^23
  static const sample_t s32_to_st_constant = (2147483648.0 / max_amplitude);  // 2^31
  static const sample_t u8_to_st_delta = 128;
  static const sample_t u8_to_st_constant = (max_amplitude / 128);
}

#endif
