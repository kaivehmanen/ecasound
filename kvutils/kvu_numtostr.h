#ifndef _KVU_NUMTOSTR_H
#define _KVU_NUMTOSTR_H

#include <string>
#include <cstdio>

string kvu_numtostr(char c);
string kvu_numtostr(unsigned char c);
string kvu_numtostr(signed char c);
string kvu_numtostr(const void *p);
string kvu_numtostr(int n);
string kvu_numtostr(unsigned int n);
string kvu_numtostr(long n);
string kvu_numtostr(unsigned long n);
string kvu_numtostr(short n);
string kvu_numtostr(unsigned short n);
string kvu_numtostr(bool b);
string kvu_numtostr(double n, int flo_prec = 2);
string kvu_numtostr(float n, int flo_prec = 2);

#endif
