#ifndef INCLUDED_ECATESTSUITE_H
#define INCLUDED_ECATESTSUITE_H

#ifdef VERBOSE
#define ECA_TEST_ENTRY()   printf("\n%s:%d - Test started", __FILE__, __LINE__)
#define ECA_TEST_SUCCESS() printf("\n%s:%d - Test passed\n", __FILE__, __LINE__); exit(0)
#define ECA_TEST_FAIL(x,y) printf("\n%s:%d - Test failed: \"%s\"\n", __FILE__, __LINE__, y); exit(x)
#define ECA_TEST_CASE()    printf("."); fflush(stdout)
#else
#define ECA_TEST_ENTRY()   ((void) 0)
#define ECA_TEST_SUCCESS() exit(0)
#define ECA_TEST_FAIL(x,y) exit(x)
#define ECA_TEST_CASE()    ((void) 0)
#endif

#endif /* INCLUDED_ECATESTSUITE_H */
