#ifndef INCLUDED_KVU_NUMTOSTR_H
#define INCLUDED_KVU_NUMTOSTR_H

#include <string>
#include <cstdio>

using std::string;

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
