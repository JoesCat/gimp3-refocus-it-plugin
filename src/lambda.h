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

#ifndef _LAMBDA_H
#define _LAMBDA_H

#include "compiler.h"
#include "convmask.h"
#include "image.h"
#include "boundary.h"

C_DECL_BEGIN

typedef struct {
	convmask_t *filter;
	int         x;
	int         y;
	int         winsize;
	real_t      minlambda;
	real_t     *lambda;
	int         mirror;
	int         nl;
} lambda_t;

lambda_t* lambda_create(lambda_t* lambda, int x, int y, real_t minlambda, int winsize, convmask_t* filter);
void lambda_destroy(lambda_t* lambda);

lambda_t* lambda_calculate_period(lambda_t* lambda, image_t* image);
lambda_t* lambda_calculate_period_nl(lambda_t* lambda, image_t* image);
lambda_t* lambda_calculate_mirror(lambda_t* lambda, image_t* image);
lambda_t* lambda_calculate_mirror_nl(lambda_t* lambda, image_t* image);
lambda_t* lambda_calculate(lambda_t* lambda, image_t* image);

void lambda_set_mirror(lambda_t* lambda, int mirror);
void lambda_set_nl(lambda_t* lambda, int nl);

/*
* GET / SET
*/

#define MACRO_LAMBDA_GET_MIRROR(l, c, r) ((l)->lambda[boundary_normalize_mirror(r, (l)->y)*(l)->x + boundary_normalize_mirror(c, (l)->x)])
#define MACRO_LAMBDA_GET_PERIOD(l, c, r) ((l)->lambda[boundary_normalize_period(r, (l)->y)*(l)->x + boundary_normalize_period(c, (l)->x)])

#if defined(USE_INLINES)

#if defined(INLINE)

static INLINE real_t lambda_get_mirror(lambda_t* lambda, int x, int y)
{
	return MACRO_LAMBDA_GET_MIRROR(lambda, x, y);
}

static INLINE real_t lambda_get_period(lambda_t* lambda, int x, int y)
{
	return MACRO_LAMBDA_GET_PERIOD(lambda, x, y);
}

#else

real_t lambda_get_mirror(lambda_t* lambda, int x, int y);
real_t lambda_get_period(lambda_t* lambda, int x, int y);

#endif

#else

#define lambda_get_mirror(l, c, r) MACRO_LAMBDA_GET_MIRROR(l, c, r)
#define lambda_get_period(l, c, r) MACRO_LAMBDA_GET_PERIOD(l, c, r)

#endif

C_DECL_END

#endif
