// ------------------------------------------------------------------------
// message_item.cpp: A simple version of C++ stringstream.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <cstdio>

#include "message_item.h"

MESSAGE_ITEM& MESSAGE_ITEM::operator<< (char c) {
    char temp[2];
    temp[0] = c;
    temp[1] = 0;
    stringtemp += string(temp);
    return (*this);    
}

MESSAGE_ITEM& MESSAGE_ITEM::operator<< (int n) {
  char ctmp[12];
  snprintf(ctmp, 12, "%d",n);
  ctmp[11] = 0;
  stringtemp += string(ctmp);
  return (*this);    
}

MESSAGE_ITEM& MESSAGE_ITEM::operator<< (const void *p) {
  char ctmp[12];
  snprintf(ctmp, 12, "%p",p);
  ctmp[11] = 0;
  stringtemp += string(ctmp);
  return (*this);    
}

MESSAGE_ITEM& MESSAGE_ITEM::operator<< (unsigned int n) {
  char ctmp[12];
  snprintf(ctmp, 12, "%u",n);
  ctmp[11] = 0;
  stringtemp += string(ctmp);
  return (*this);    
}

MESSAGE_ITEM& MESSAGE_ITEM::operator<< (long n) {
  char ctmp[12];
  snprintf(ctmp, 12, "%ld",n);
  ctmp[11] = 0;
  stringtemp += string(ctmp);
  return (*this);    
}

MESSAGE_ITEM& MESSAGE_ITEM::operator<< (unsigned long n) {
  char ctmp[12];
  snprintf(ctmp, 12, "%lu",n);
  ctmp[11] = 0;
  stringtemp += string(ctmp);
  return (*this);    
}

MESSAGE_ITEM& MESSAGE_ITEM::operator<< (double n) {
  char ctmp[32];
  snprintf(ctmp, 12, "%.*f",flo_prec, n);
  ctmp[31] = 0;
  stringtemp += string(ctmp);
  return (*this);    
}







