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

#include "weights.h"

static void weights_set(weights_t* weights, int x, int y, double value) {
  weights->w[(weights->r2 + y) * weights->size + (weights->r2 + x)] = value;
  weights->w[weights->stride + y * weights->size + x] = value;
}

weights_t* weights_create(weights_t* weights, convmask_t* convmask) {
  int r, r2, i, j, k, l;
  int rxnz, rynz;
  double s;
  int size;

  rxnz = rynz = 0;
  r = convmask->radius;
  weights->r2 = r2 = 2 * r;
  weights->size = size = 2*r2 + 1;
  weights->stride = r2 * (size + 1);
  if (!(weights->w = (double*)malloc(sizeof(double) * size * size)))
    return NULL; /* memory full */

  for (i = 0; i <= r2; i++) {
    for (j = 0; j <= r2; j++) {
      s = 0.0;
      for (k = -r; k <= r-i; k++) {
        for (l = -r; l <= r-j; l++) {
          s -= convmask_get(convmask, k, l) * convmask_get(convmask, k+i, l+j);
        }
      }
      if (fabs(s) > 1e-6) {
        if (i > rxnz) rxnz = i;
        if (j > rynz) rynz = j;
      }
      weights_set(weights,  i,  j, s);
      weights_set(weights, -i, -j, s);

      s = 0.0;
      for (k = -r; k <= r-i; k++) {
        for (l = -r; l <= r-j; l++) {
          s -= convmask_get(convmask, k, l+j) * convmask_get(convmask, k+i, l);
        }
      }
      if (fabs(s) > 1e-6) {
        if (i > rxnz) rxnz = i;
        if (j > rynz) rynz = j;
      }
      weights_set(weights, -i,  j, s);
      weights_set(weights,  i, -j, s);
    }
  }

  weights->rxnz = rxnz;
  weights->rynz = rynz;

  return weights;
}

void weights_destroy(weights_t* weights) {
  free(weights->w);
}

double weights_get(weights_t* weights, int x, int y) {
  return weights->w[(weights->r2 + y) * weights->size + (weights->r2 + x)];
}

#if defined(NDEBUG)
void weights_print(weights_t* weights, FILE* file) {
  int i, j;

  fprintf(file, "WEIGHTS: (rxnz, rynz)=(%d,%d)\n", weights->rxnz, weights->rynz);
  for (j = -weights->r2; j <= weights->r2; j++) {
    for (i = -weights->r2; i <= weights->r2; i++) {
      fprintf(file, " %3.3e", (float)weights_get(weights, i, j));
    }
    fprintf(file, "\n");
  }
}
#endif
