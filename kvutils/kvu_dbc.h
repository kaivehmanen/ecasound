/**************************************************************************
 * dbc.h: Design-by-contract framework for C and C++
 * Copyright (C) 2001,2002 Kai Vehmanen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 **************************************************************************/

#ifndef INCLUDED_DBC_H
#define INCLUDED_DBC_H

#ifdef ENABLE_DBC

#include <stdio.h>

#define DBC_REQUIRE(expr)						      \
   (expr) ? (void)(0) :	(void)(fprintf(stderr, "Warning: DBC_REQUIRE failed - \"%s\", %s, %d.\n", #expr,__FILE__, __LINE__))
#define DBC_ENSURE(expr)							      \
   (expr) ? (void)(0) :	(void)(fprintf(stderr, "Warning: DBC_ENSURE failed - \"%s\", %s, %d.\n", #expr,__FILE__, __LINE__))
#define DBC_CHECK(expr)							      \
   (expr) ? (void)(0) :	(void)(fprintf(stderr, "Warning: DBC_CHECK failed - \"%s\", %s, %d.\n", #expr,__FILE__, __LINE__))
#define DBC_DECLARE(expr)               expr

#else /* DBC DISABLED --> */

#define DBC_REQUIRE(expr)		((void) 0)
#define DBC_ENSURE(expr)		((void) 0)
#define DBC_CHECK(expr)		        ((void) 0)
#define DBC_DECLARE(expr)               ((void) 0)

#endif /* <-- DBC DISABLED */

#endif /* INCLUDED_DBC_H */
