#ifndef INCLUDED_ECATESTSUITE_H
#define INCLUDED_ECATESTSUITE_H

#ifdef VERBOSE
#define ECA_TEST_ENTRY()   printf("%s:%d - Test started\n", __FILE__, __LINE__)
#define ECA_TEST_SUCCESS() printf("%s:%d - Test passed\n", __FILE__, __LINE__); return(0)
#define ECA_TEST_FAIL(x,y) printf("%s:%d - Test failed: \"%s\"\n", __FILE__, __LINE__, y); return(x)
#else
#define ECA_TEST_ENTRY()   ((void) 0)
#define ECA_TEST_SUCCESS() return(0)
#define ECA_TEST_FAIL(x,y) return(x)
#endif

#endif /* INCLUDED_ECATESTSUITE_H */
