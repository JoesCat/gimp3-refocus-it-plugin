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

#ifndef _WEIGHTS_H
#define _WEIGHTS_H

#include <stdio.h>
#include "compiler.h"
#include "convmask.h"

C_DECL_BEGIN

typedef struct {
	real_t *w;
	int     r2;
	int     rxnz, rynz;
	int     stride;
	int     size;
} weights_t;

weights_t* weights_create(weights_t* weights, convmask_t* convmask);
void weights_destroy(weights_t* weights);

void weights_print(weights_t* weights, FILE* file);

/*
* GET / SET
*/

#define MACRO_WEIGHTS_GET(weig, x, y) ((weig)->w[(weig)->stride + (y) * (weig)->size + (x)])
#define MACRO_WEIGHTS_SET(weig, x, y, value) (weig)->w[(weig)->stride + (y) * (weig)->size + (x)] = value

#if defined(USE_INLINES)

#if defined(INLINE)

static INLINE real_t weights_get(weights_t* weights, int x, int y)
{
	return MACRO_WEIGHTS_GET(weights, x, y);
}

static INLINE void weights_set(weights_t* weights, int x, int y, real_t value)
{
	MACRO_WEIGHTS_SET(weights, x, y, value);
}

#else

real_t weights_get(weights_t* weights, int x, int y);
void weights_set(weights_t* weights, int x, int y, real_t value);

#endif

#else

#define weights_get(weig, x, y) MACRO_WEIGHTS_GET(weig, x, y)
#define weights_set(weig, x, y, value) MACRO_WEIGHTS_SET(weig, x, y, value)

#endif

C_DECL_END

#endif
