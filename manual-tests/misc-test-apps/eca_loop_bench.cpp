#include "samplebuffer.h"
#include "eca-version.h"

#include "kvu_procedure_timer.h"
#include "ecatestsuite.h"

int test_make_silent(void);
int test_copy_ops(void);

int main(int argc, char *argv[])
{
  int res = 0;

  std::printf("********************************************************"
	      "\nTesting with libecasound v%s (%s).\n",
	      ecasound_library_version, __FILE__);

  res += test_make_silent();
  res += test_copy_ops();

  return res;
}

int test_make_silent(void)
{
  const int loops = 100000;
  const int bufsize = 1024;
  const int channels = 2;

  std::printf("make_silent with %d loops (bufsize=%d, ch=%d):\n", 
	      loops, 1024, 12);

  PROCEDURE_TIMER t1;
  SAMPLE_BUFFER sbuf (bufsize, channels);

  /* note: make sure code is paged in */
  sbuf.make_silent();
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf.make_silent_range(0, bufsize);
  }
  t1.stop();

  std::printf("\tmake_silent_range: %.03fms (%.03fms per event)\n", 
	      t1.last_duration_seconds() * 1000.0,
	      t1.last_duration_seconds() * 1000.0 / loops);

  /* note: make sure code is paged in */
  sbuf.make_silent_range(0, bufsize);
  t1.reset();
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf.make_silent();
  }
  t1.stop();
  
  std::printf("\tmake_silent:       %.03fms\n", 
	      t1.last_duration_seconds() * 1000.0);
}  

int test_copy_ops(void)
{
  const int loops = 100000;
  const int bufsize = 1024;
  const int channels = 2;

  PROCEDURE_TIMER t1;
  SAMPLE_BUFFER sbuf_a (bufsize, channels);
  SAMPLE_BUFFER sbuf_b (bufsize, channels);

  std::printf("copy_ops with %d loops (bufsize=%d, ch=%d):\n", 
	      loops, 1024, 12);

#if LIBECASOUND_VERSION >= 22
  /* note: make sure code is paged in */
  sbuf_a.copy_all_content(sbuf_b);

  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.copy_all_content(sbuf_b);
  }
  t1.stop();

  std::printf("\tcopy_all_content: %.03fms (%.03fms per event)\n", 
	      t1.last_duration_seconds() * 1000.0,
	      t1.last_duration_seconds() * 1000.0 / loops);

  /* note: make sure code is paged in */
  sbuf_a.copy_matching_channels(sbuf_b);

  t1.reset();
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.copy_matching_channels(sbuf_b);
  }
  t1.stop();

  std::printf("\tcopy_matching_channels: %.03fms (%.03fms per event)\n", 
	      t1.last_duration_seconds() * 1000.0,
	      t1.last_duration_seconds() * 1000.0 / loops);

#else

  /* note: make sure code is paged in */
  sbuf_a.copy(sbuf_b);

  t1.reset();
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.copy(sbuf_b);
  }
  t1.stop();

  std::printf("\tcopy (v21 lib API): %.03fms (%.03fms per event)\n", 
	      t1.last_duration_seconds() * 1000.0,
	      t1.last_duration_seconds() * 1000.0 / loops);

#endif
}  
