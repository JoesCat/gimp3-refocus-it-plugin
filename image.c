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

#include <stdlib.h>
#include <string.h>
#include "xmalloc.h"
#include "image.h"

image_t* image_create(image_t* image, int x, int y)
{
  image->x = x;
  image->y = y;
  image->data = (real_t*)xmalloc(sizeof(real_t) * x * y);
  return image;
}

image_t* image_create_copyparam(image_t* image, image_t* src)
{
  image->x = src->x;
  image->y = src->y;
  image->data = (real_t*)xmalloc(sizeof(real_t) * image->x * image->y);
  return image;
}

void image_destroy(image_t* image)
{
  xfree(image->data);
}

image_t* image_convolve_mirror(image_t* dst, image_t* src, convmask_t* filter)
{
  int i, j, k, l, r;
  real_t value;

  r = filter->radius;
  for (i = 0; i < src->x; i++)
  {
    for (j = 0; j < src->y; j++)
    {
      value = R(0.0);
      for (k = -r; k <= r; k++)
      {
	for (l = -r; l <= r; l++)
	{
	  value += convmask_get(filter, k,l) * image_get_mirror(src, i-k, j-l);
	}
      }
      image_set(dst, i, j, value);
    }
  }
  return dst;
}
	  
image_t* image_convolve_period(image_t* dst, image_t* src, convmask_t* filter)
{
  int i, j, k, l, r;
  real_t value;

  r = filter->radius;
  for (i = 0; i < src->x; i++)
  {
    for (j = 0; j < src->y; j++)
    {
      value = R(0.0);
      for (k = -r; k <= r; k++)
      {
	for (l = -r; l <= r; l++)
	{
	  value += convmask_get(filter, k,l) * image_get_period(src, i-k, j-l);
	}
      }
      image_set(dst, i, j, value);
    }
  }
  return dst;
}

int image_load_png_file(image_t* image, FILE* file)
{
  int c;
  int i, x, y;
  char buff[2];
  unsigned char byte;

  if (fread(buff, sizeof(buff), 1, file) != 1) {
    return -1;
  }

  if (buff[0] != 'P' || buff[1] != '5') {
    return -1;
  }

  c = getc(file);
  if (c == '\r') c = getc(file);
  if (c != '\n') return -1;
  c = getc(file);
  while (c == '#') {
    while (((c = getc(file)) != '\n') && (c != EOF));
    if (c != EOF) c = getc(file);
  }

  if (c == EOF) return -1;
  ungetc(c, file);

  fscanf(file, "%d %d\n", &x, &y);
  fscanf(file, "%d\n", &c);

  image_create(image, x, y);

  y = 0;
  i = 0;

  while ( fread(&byte, sizeof(byte), 1, file) == 1) {
    image_set(image, i, y, (real_t)byte);
    i++;
    if (i >= x) {
      i = 0;
      y++;
    }
  }
  return 0;
}

int image_save_png_file(image_t* image, FILE* file)
{
  int i, j;
  unsigned char c;
  fprintf(file, "P5\n");
  fprintf(file, "# Deblur output\n");
  fprintf(file, "%d %d\n", image->x, image->y);
  fprintf(file, "255\n");

  for (j = 0; j < image->y; j++)
  {
    for (i = 0; i < image->x; i++)
    {
      c = (unsigned char)(image_get(image, i, j) + R(0.5));
      fwrite(&c, sizeof(c), 1, file);
    }
  }
  return 0;
}

int image_save_png(image_t* image, const char* name)
{
  int retval;
  FILE *file;
  file = fopen(name, "wb");
  if (file) {
    retval = image_save_png_file(image, file);
    fclose(file);
  }
  else {
	retval = -1;
  }
  return retval;
}

int image_load_png(image_t* image, const char* name)
{
  FILE *file;
  int retval;
  file = fopen(name, "rb");
  if (file) {
    retval = image_load_png_file(image, file);
    fclose(file);
  }
  else {
	retval = -1;
  }
  return retval;
}

void image_load_bytes_gray(image_t* image, unsigned char* src)
{
	int size, i;
	real_t *dst;
	size = image->x * image->y;

	dst = image->data;
	for (i = 0; i < size; i++)
	{
		*dst = *src;
		src++;
		dst++;
	}
}

void image_load_bytes_rgb(image_t* image, unsigned char* bytes, unsigned int channel)
{
	int size, i;
	unsigned char *src;
	real_t *dst;
	size = image->x * image->y;

	dst = image->data;
	src = bytes + channel;
	for (i = 0; i < size; i++)
	{
		*dst = *src;
		src += 3;
		dst++;
	}
}



/*
 * GET / SET
 */

#if !defined(INLINE) && !defined(INLINE_MACRO)

real_t image_get_mirror(image_t* image, int x, int y)
{
  return MACRO_IMAGE_GET_MIRROR(image, x, y);
}

real_t image_get_period(image_t* image, int x, int y)
{
  return MACRO_IMAGE_GET_PERIOD(image, x, y);
}

void image_set(image_t* image, int x, int y, real_t value)
{
  MACRO_IMAGE_SET(image, x, y, value);
}

real_t image_get(image_t* image, int x, int y)
{
  return MACRO_IMAGE_GET(image, x, y);
}

#endif
