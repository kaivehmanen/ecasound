#include "audioio-proxy-buffer.h"

/**
 * Constructor.
 */
AUDIO_IO_PROXY_BUFFER::AUDIO_IO_PROXY_BUFFER(int number_of_buffers,
					     long int buffersize,
					     int channels,
					     long int sample_rate)
  :  readptr_rep(0),
     writeptr_rep(0),
     finished_rep(0),
     sbufs_rep(number_of_buffers, SAMPLE_BUFFER(buffersize,
						channels,
						sample_rate))
{
  cerr << "Created a new client buffer with " 
       << sbufs_rep.size()
       << " buffers." 
       << endl;
}

/**
 * Number of sample buffer available for reading.
 */
int AUDIO_IO_PROXY_BUFFER::read_space(void) {
  int write = writeptr_rep.get();
  int read = readptr_rep.get();
  
  if (write >= read)
    return(write - read);
  else
    return((write - read + sbufs_rep.size()) % sbufs_rep.size());
}

/**
 * Number of sample buffers available for writing.
 */
int AUDIO_IO_PROXY_BUFFER::write_space(void) {
  int write = writeptr_rep.get();
  int read = readptr_rep.get();
  
  if (write > read)
    return(((read - write + sbufs_rep.size()) % sbufs_rep.size()) - 1);
  else if (write < read)
    return(read - write - 1);
  else 
    return(sbufs_rep.size() - 1);
}

void AUDIO_IO_PROXY_BUFFER::advance_read_pointer(void) {
  readptr_rep.set((readptr_rep.get() + 1) % sbufs_rep.size());
}

void AUDIO_IO_PROXY_BUFFER::advance_write_pointer(void) {
  writeptr_rep.set((writeptr_rep.get() + 1) % sbufs_rep.size());
}
