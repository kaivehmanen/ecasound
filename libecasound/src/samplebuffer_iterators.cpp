#include <vector>

#include "samplebuffer.h"
#include "samplebuffer_iterators.h"

// ---------------------------------------------------------------------

void SAMPLE_ITERATOR::begin(void) {
  index = 0;
  channel_index = 0;
  if (target->buffersize_rep == 0)
    channel_index = target->channel_count_rep;
}

void SAMPLE_ITERATOR::next(void) {
  ++index;
  if (index == target->buffersize_rep) {
    ++channel_index;
    index = 0;
  }
}

// ---------------------------------------------------------------------

void SAMPLE_ITERATOR_CHANNEL::begin(int channel) {
  index = 0;
  channel_index = channel;
}

// ---------------------------------------------------------------------

void SAMPLE_ITERATOR_CHANNELS::begin(void) {
  index = 0;
  channel_index = 0;
  if (target->buffersize_rep == 0)
    channel_index = target->channel_count_rep;
}

void SAMPLE_ITERATOR_CHANNELS::next(void) {
  ++index;
  if (index == target->buffersize_rep) {
    ++channel_index;
    index = 0;
  }
}

// ---------------------------------------------------------------------
