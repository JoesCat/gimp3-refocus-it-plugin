/*
 * Written 2003 Lukas Kunc <Lukas.Kunc@seznam.cz>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _CONVMASK_H
#define _CONVMASK_H

#include <stdio.h>
#include "compiler.h"

C_DECL_BEGIN

/** structures */

typedef struct {
	int      radius;
	int      r21;
	int      speeder;
	real_t  *coef;
} convmask_t;


convmask_t* convmask_create(convmask_t* convmask, int radius);
void convmask_destroy(convmask_t* convmask);
convmask_t* convmask_normalize(convmask_t* convmask);
convmask_t* convmask_convolve(convmask_t* ct, convmask_t* c1, convmask_t* c2);

void convmask_set_circle(convmask_t* convmask, int i, int j, real_t value);
void convmask_print(convmask_t* convmask, FILE* file);

/*
* GET / SET
*/

#define MACRO_CONVMASK_SET(convmask, i, j, value) (convmask)->coef[(j)*(convmask)->r21 + (convmask)->speeder + (i)] = (value)
#define MACRO_CONVMASK_GET(convmask, i, j) ((convmask)->coef[(j)*(convmask)->r21 + (convmask)->speeder + (i)])
#define MACRO_CONVMASK_GET_0(c, i, j) ((abs(i) <= (c)->radius && abs(j) <= (c)->radius) ? MACRO_CONVMASK_GET(c, i, j) : R(0.0))

#if defined(USE_INLINES)

#if defined(INLINE)

static INLINE void convmask_set(convmask_t* convmask, int i, int j, real_t value)
{
	MACRO_CONVMASK_SET(convmask, i, j, value);
}

static INLINE real_t convmask_get(convmask_t* convmask, int i, int j)
{
	return MACRO_CONVMASK_GET(convmask, i, j);
}

#else

void convmask_set(convmask_t* convmask, int i, int j, real_t value);
real_t convmask_get(convmask_t* convmask, int i, int j);
real_t convmask_get_0(convmask_t* convmask, int i, int j);

#endif

#else

#define convmask_set(convmask, i, j, value) MACRO_CONVMASK_SET(convmask, i, j, value)
#define convmask_get(convmask, i, j) MACRO_CONVMASK_GET(convmask, i, j)
#define convmask_get_0(convmask, i, j) MACRO_CONVMASK_GET_0(convmask, i, j)

#endif

C_DECL_END

#endif
