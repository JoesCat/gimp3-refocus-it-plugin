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

#ifndef _BOUNDARY_H
#define _BOUNDARY_H

#include "compiler.h"

#define MACRO_BOUNDARY_NORMALIZE_MIRROR(x, y) (((x) >= (y)) ? 2*(y)-(x)-2 : (((x) < 0) ? -(x) : (x)))
#define MACRO_BOUNDARY_NORMALIZE_PERIOD(x, y) (((x) >= (y)) ? ((x) - (y)) : (((x) < 0) ? (x) + (y) : (x)))

/* Inefficient with MSVC
* #define MACRO_BOUNDARY_NORMALIZE_PERIOD(x, y)	(((x) + (y)) % (y))
*/

#if defined(USE_INLINES)

#if defined(INLINE)

static INLINE int boundary_normalize_mirror(int x, int lx)
{
	return MACRO_BOUNDARY_NORMALIZE_MIRROR(x, lx);
}

static INLINE int boundary_normalize_period(int x, int lx)
{
	return MACRO_BOUNDARY_NORMALIZE_PERIOD(x, lx);
}

#else

int boundary_normalize_mirror(int x, int lx);
int boundary_normalize_period(int x, int lx);

#endif

#else

#define boundary_normalize_mirror(x, y) MACRO_BOUNDARY_NORMALIZE_MIRROR(x, y)
#define boundary_normalize_period(x, y)	MACRO_BOUNDARY_NORMALIZE_PERIOD(x, y)

#endif

#endif
