#ifndef _FILEIO_MMAP_FTHREAD_H
#define _FILEIO_MMAP_FTHREAD_H

#include <vector>
#include <map>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#ifndef MAP_FAILED
#define MAP_FAILED	((__ptr_t) -1)
#endif

#if !defined _CADDR_T && !defined __USE_BSD
#define _CADDR_T
typedef char* caddr_t;
#endif

void ecasound_fiommap_register_fd(int fd, long int length);
void ecasound_fiommap_close_fd(int fd);
void ecasound_fiommap_reset(int fd, long fposition);
caddr_t ecasound_fiommap_active_buffer(int fd);
long int ecasound_fiommap_active_buffersize(int fd);
long int ecasound_fiommap_maximum_buffersize(void);
void ecasound_fiommap_next_buffer(int fd);
void ecasound_fiommap_exec_thread(void) throw(ECA_ERROR*); 
void *ecasound_fiommap_process(void *);
void ecasound_fiommap_clean(void*);

/**
 * Class that represents a two-part mmap buffer.
 */
class ECASOUND_FIOMMAP_BUFFER {
 public:

  vector<caddr_t> buffers;
  vector<caddr_t>::size_type locked_buffer;
  map<int,bool> ready_for_fill;
  vector<long int> mmap_length;

  long int mmap_low;
  long int mmap_high;
  long int file_length;
  int fd;

  ECASOUND_FIOMMAP_BUFFER(void) {
    locked_buffer = 0;
    mmap_low = -1;
    mmap_high = 0;
    mmap_length.resize(2);
    buffers.resize(2);
    file_length = 0;
  }

  ~ECASOUND_FIOMMAP_BUFFER(void) {
    for(vector<caddr_t>::size_type p = 0;
	p < buffers.size();
	p++) {
      munlock(buffers[p], mmap_length[p]);
      munmap(buffers[p], mmap_length[p]);
    }

    while(buffers.size() > 0) {
      if (buffers.back() != 0) {
	delete buffers.back();
      }
      buffers.pop_back();
    }
  }
};

#endif
