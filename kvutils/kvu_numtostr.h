#ifndef INCLUDED_KVU_NUMTOSTR_H
#define INCLUDED_KVU_NUMTOSTR_H

#include <string>
#include <cstdio>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef USE_CXX_STD_NAMESPACE
using std::string;
#endif

std::string kvu_numtostr(char c);
std::string kvu_numtostr(unsigned char c);
std::string kvu_numtostr(signed char c);
std::string kvu_numtostr(const void *p);
std::string kvu_numtostr(int n);
std::string kvu_numtostr(unsigned int n);
std::string kvu_numtostr(long n);
std::string kvu_numtostr(unsigned long n);
std::string kvu_numtostr(short n);
std::string kvu_numtostr(unsigned short n);
std::string kvu_numtostr(bool b);
std::string kvu_numtostr(double n, int flo_prec = 2);
std::string kvu_numtostr(float n, int flo_prec = 2);

#endif
