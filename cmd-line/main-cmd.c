/*
 * Written 2004 Lukas Kunc <Lukas.Kunc@seznam.cz>
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

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "compiler.h"
#include "xmalloc.h"
#include "hopfield.h"
#include "image.h"
#include "lambda.h"
#include "blur.h"
#include "getopt.h"
#include "gettext.h"

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#define TRUE      1
#define FALSE     0

#define LAMBDAMIN_MAX         R(100.0)
#define LAMBDAMIN_USABLE_MAX  R(0.999)
#define LAMBDA_MAX            R(10000.0)


static const char* prog_name = PACKAGE_NAME;
static const char* version = PACKAGE_VERSION;

enum
{
  BOUNDARY_MIRROR = 0,
  BOUNDARY_PERIODICAL,
  BOUNDARY_LAST
};

static const char* boundary_strings[] = {"mirror", "period", NULL};

/*
* FORWARD DECLARATIONS
*/

typedef struct
{
  real_t  radius;
  real_t  gauss;
  real_t  motion;
  real_t  mot_angle;
  real_t  lambda;
  real_t  lambda_min;
  unsigned int  winsize;
  unsigned int  iterations;
  unsigned int  boundary;
  unsigned int  adaptive_smooth;
  unsigned int  save_intermediate;
  unsigned int  verbose;
  unsigned int  binary;
  char   *filename;
  char   *dirname;
} SInputParameters;

typedef struct
{
  unsigned int  width;
  unsigned int  height;
  int     img_bpp;
} SImageParameters;

typedef struct
{
  image_t imageR;
  image_t imageG;
  image_t imageB;
  hopfield_t hopfieldR;
  hopfield_t hopfieldG;
  hopfield_t hopfieldB;
  convmask_t blur;
  convmask_t filter;
  lambda_t lambdafldR;
  lambda_t lambdafldG;
  lambda_t lambdafldB;
} SHopfield;


/*
* STATIC DATA
*/

static SInputParameters   input_parameters;
static SImageParameters   image_parameters;
static SHopfield    hopfield;

static int    progress_output;

/*
* FUNCTIONS
*/

static void log_progress(int level, const char* info, ...)
{
  va_list args;
  if (level <= input_parameters.verbose)
  {
    if (progress_output)
    {
      fprintf(stderr, "\n");
      progress_output = 0;
    }
    fprintf(stderr, ">%d< ", level);
    va_start(args, info);
    vfprintf(stderr, info, args);
    va_end(args);
    fprintf(stderr, "\n");
  }
}

static void input_parameters_destroy()
{
  free(input_parameters.filename);
}

static void input_parameters_default()
{
  input_parameters.radius = R(6.0);
  input_parameters.gauss = R(0.0);
  input_parameters.lambda = R(100.0);
  input_parameters.lambda_min = R(30.0);
  input_parameters.winsize = 3;
  input_parameters.iterations = 100;
  input_parameters.boundary = BOUNDARY_MIRROR;
  input_parameters.adaptive_smooth = TRUE;
  input_parameters.save_intermediate = FALSE;
  input_parameters.filename = NULL;
  input_parameters.verbose = 0;
  input_parameters.binary = TRUE;
  input_parameters.motion = R(0.0);
  input_parameters.mot_angle = R(0.0);
}

static void miss_arg(const char* opt, const char* desc)
{
  if (desc)
  {
    fprintf(stderr, _("%s: expected value for parameter '%s' -%s\n"),
      prog_name, desc, opt);
  }
  else
  {
    fprintf(stderr, _("%s: expected argument for option -%s\n"),
      prog_name, opt);
  }
}


static void bad_arg(const char* opt, const char* desc, const char* type)
{
  fprintf(stderr, _("%s: expected value of type '%s' for parameter '%s' -%s\n"),
    prog_name, type, desc, opt);
}

static real_t float_arg(const char* optarg, const char* opt, int* err, const char* desc, real_t vmin, real_t vmax)
{
  double val = 0.0;
  int len;
  if (optarg)
  {
    if (sscanf(optarg, "%lf%n", &val, &len) != 1 || optarg[len] != '\0')
    {
      bad_arg(opt, desc, "float");
      (*err)++;
    }
    else
    {
      if (vmin > val || vmax < val)
      {
        fprintf(stderr, _("parameter '%s' -%s out of range (%f, %f)\n"), desc, opt, (double)vmin, (double)vmax);
        (*err)++;
      }
    }
  }
  else
  {
    (*err)++;
    miss_arg(opt, desc);
  }
  return (real_t)val;
}

static int int_arg(const char* optarg, const char* opt, int* err, const char* desc, int vmin, int vmax)
{
  int val = 0;
  int len;
  if (optarg)
  {
    if (sscanf(optarg, "%d%n", &val, &len) != 1 || optarg[len] != '\0')
    {
      bad_arg(opt, desc, "int");
      (*err)++;
    }
    else
    {
      if (vmin > val || vmax < val)
      {
        fprintf(stderr, _("parameter '%s' -%s out of range (%d, %d)\n"), desc, opt, vmin, vmax);
        (*err)++;
      }
    }
  }
  else
  {
    (*err)++;
    miss_arg(opt, desc);
  }
  return val;
}

static int enum_arg(const char* optarg, const char* opt, int* err, const char** enum_enum, const char* desc)
{
  const char** org_enum;
  org_enum = enum_enum;
  if (optarg)
  {
    while (*enum_enum && strcmp(optarg, *enum_enum)) enum_enum++;
    if (!*enum_enum)
    {
      fprintf(stderr, _("expected valid value for parameter '%s' -%s, got '%s'\n"), desc, opt, optarg);
      (*err)++;
    }
  }
  else
  {
    (*err)++;
    miss_arg(opt, desc);
  }
  return enum_enum - org_enum;
}


static char* str_arg(const char* optarg, const char* opt, int* err, const char* desc)
{
  if (optarg)
  {
    return xstrdup(optarg);
  }
  else
  {
    (*err)++;
    miss_arg(opt, desc);
    return NULL;
  }
}

static void print_help()
{
  fprintf(stdout, _(
      "Iterative refocus - refocus images acquired with defocussed camera\n"
      "or blurred with gaussian blur (astronomy). Implemented via\n"
      "minimization of error function using Hopfield neural network.\n\n"
      "Usage: %s [OPTION] ...\n"
      "Options:\n"
      "   -r <defocus radius>, --radius=<defocus radius>\n"
      "                       real number from interval <0.0, 32.0>\n"
      "   -g <gaussian blur variance>, --gauss=<gaussian blur variance>\n"
      "                       real number from <0.0, 32.0>\n"),
      prog_name);

  fprintf(stdout, _(
      "   -n <noise reduction factor>, --noise=<noise reduction factor>\n"
      "                       real number from <0.0, %.1f>\n"
      "   -b <boundary conditions>, --boundary=<boundary conditions>\n"
      "                       'mirror' or 'period'\n"
      "   -s <area smoothness>, --smoothness=<area smoothness>\n"
      "                       real number from <0.0, %.1f>\n"),
      LAMBDA_MAX, LAMBDAMIN_MAX);

  fprintf(stdout, _(
      "   -a, --non-adaptive\n"
      "                       do not use adaptive adjustments of noise\n"
      "                       reduction\n"
      "   -m <n>, --intermediate=<n>\n"
      "                       save result of every n-th iteration\n"
      "   -c, --ascii-pnm\n"
      "                       use binary PNM files for output\n"
      "   -v <level>, --verbosity=<level>\n"
      "                       set verbosity level 0..8\n"
      "   -w <window size>, --window=<window size>\n"
      "                       size of window for area smoothing 1..16\n"));
  fprintf(stdout, _(
      "   -i <number of iterations>, --iterations=<number of iterations>\n"
      "                       number of iterations to compute\n"
      "   -f <file name>, --file=<file name>\n"
      "                       name of file with blurred image,\n"
      "                       if no file name is given, input image is\n"
      "                       read from stdin and output image(s) is\n"
      "                       (are) written to stdout\n"));
  fprintf(stdout, _(
      "   -h, --help\n"
      "                       print this help and exit\n"
      "   -V, --version\n"
      "                       print version information and exit\n"));
}

static void print_version()
{
  fprintf(stdout, _("Iterative refocus - %s version %s\n"
    "Written by Lukas Kunc\n\n"
    "Copyright (C) 2004 Free Software Foundation, Inc.\n"
    "This is free software; see the source for copying conditions."
    "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR"
    "A PARTICULAR PURPOSE.\n"), prog_name, version);
}

static void input_parameters_init(int argc, char **argv)
{
  int c;
  int err = 0;
  char opt[2];
  int option_index;
  static struct option long_options[] = {
    {"radius", 1, 0, 'r'},
    {"gauss", 1, 0, 'g'},
    {"noise", 1, 0, 'n'},
    {"boundary", 1, 0, 'b'},
    {"smoothness", 1, 0, 's'},
    {"non-adaptive", 0, 0, 'a'},
    {"intermediate", 1, 0, 'm'},
    {"ascii-pnm", 0, 0, 'c'},
    {"verbosity", 1, 0, 'v'},
    {"window", 1, 0, 'w'},
    {"iterations", 1, 0, 'i'},
    {"file", 1, 0, 'f'},
    {"help", 0, 0, 'h'},
    {"version", 0, 0, 'V'},
    {NULL, 0, 0, 0}
  };

  opterr = 0;
  while ((c = getopt_long(argc, argv, "ab:cf:g:hi:m:n:r:s:v:w:V", long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case 'r':
        input_parameters.radius = float_arg(optarg, "r", &err, _("defocus radius"), 0.0, 32.0);
        break;
      case 'g':
        input_parameters.gauss = float_arg(optarg, "g", &err, _("gaussian blur variance"), 0.0, 32.0);
        break;
      case 'n':
        input_parameters.lambda = float_arg(optarg, "n", &err, _("noise reduction"), 0.0, LAMBDA_MAX);
        break;
      case 'b':
        input_parameters.boundary = enum_arg(optarg, "b", &err, boundary_strings, _("boundary"));
        break;
      case 's':
        input_parameters.lambda_min = float_arg(optarg, "s", &err, _("smoothness"), 0.0, LAMBDAMIN_MAX);
        break;
      case 'a':
        input_parameters.adaptive_smooth = FALSE;
        break;
      case 'm':
        input_parameters.save_intermediate = int_arg(optarg, "m", &err, _("save every n-th result"), 0, 1000);
        break;
      case 'c':
        input_parameters.binary = FALSE;
        break;
      case 'v':
        input_parameters.verbose = int_arg(optarg, "v", &err, _("verbosity level"), 0, 8);
        break;
      case 'w':
        input_parameters.winsize = int_arg(optarg, "w", &err, _("window size"), 1, 16);
        break;
      case 'i':
        input_parameters.iterations = int_arg(optarg, "i", &err, _("number of iterations"), 1, 1000);
        break;
      case 'f':
        input_parameters.filename = str_arg(optarg, "f", &err, _("file name"));
        break;
      case 'h':
        print_help();
        exit(0);
        break;
      case 'V':
        print_version();
        exit(0);
        break;
        
      case '?':
        opt[0] = optopt;
        opt[1] = '\0';
        fprintf(stderr, _("%s: unrecognized option '-%s'\n"), prog_name, opt);
        err++;
        break;
      case ':':
        opt[0] = optopt;
        opt[1] = '\0';
        miss_arg(opt, NULL);
        err++;
        break;
      default:
        fprintf(stderr, _("%s: error processing options\n"), prog_name);
        err++;
        break;
    }
  }
  if (optind < argc)
  {
    fprintf(stderr, _("%s: unrecognized option '%s'\n"), prog_name, argv[optind]);
    err++;
  }
  if (err)
  {
    fprintf(stderr, _("Try '%s -h' for more information\n"), prog_name);
    exit(1);
  }


  log_progress(8, _("defocus blur radius: %f"), input_parameters.radius);
  log_progress(8, _("gaussian blur variance: %f"), input_parameters.gauss);
  log_progress(8, _("noise reduction: %f"), input_parameters.lambda);
  log_progress(8, _("area smoothness: %f"), input_parameters.lambda_min);
  log_progress(8, _("area smoothing window size: %d"), input_parameters.winsize);
  log_progress(8, _("number of iterations: %d"), input_parameters.iterations);
  log_progress(8, _("boundaries: %s"), boundary_strings[input_parameters.boundary]);
  log_progress(8, _("adaptive noise reduction: %s"), input_parameters.adaptive_smooth ? _("yes") : _("no"));
  log_progress(8, _("save intermediate results: %d"), input_parameters.save_intermediate);
  log_progress(8, _("input file: %s"), input_parameters.filename ? input_parameters.filename : "<stdin>");
  log_progress(8, _("verbosity: %d"), input_parameters.verbose);
  log_progress(8, _("use binary PNM: %s"), input_parameters.binary ? _("yes") : _("no"));
  log_progress(8, "---------------------------------------");
}

static void hopfield_data_init()
{
  if (input_parameters.filename)
  {
    if (image_load_pnm(&hopfield.imageR, &hopfield.imageG, &hopfield.imageB,
          &(image_parameters.img_bpp), input_parameters.filename) == -1)
    {
      fprintf(stderr, _("%s: failed to load PNM file '%s': %s\n"), prog_name,
          input_parameters.filename, strerror(errno));
      exit(1);
    }
  }
  else
  {
    if (image_load_pnm_file(&hopfield.imageR, &hopfield.imageG, &hopfield.imageB,
          &(image_parameters.img_bpp), stdin) == -1)
    {
      fprintf(stderr, _("%s: failed to read PNM file from stdin: %s\n"), prog_name,
          strerror(errno));
      exit(1);
    }
  }
  image_parameters.width = hopfield.imageR.x;
  image_parameters.height = hopfield.imageR.y;
}

static void hopfield_data_destroy()
{
  switch (image_parameters.img_bpp)
  {
  case 1:
  case 2:
    image_destroy(&hopfield.imageR);
    break;
  case 3:
  case 4:
    image_destroy(&hopfield.imageR);
    image_destroy(&hopfield.imageG);
    image_destroy(&hopfield.imageB);
    break;
  }
}

static void hopfield_data_save(int prefix)
{
  char *filename;
  char *slpos;
  if (input_parameters.filename)
  {
    filename = xmalloc(strlen(input_parameters.filename) + 32);
    slpos = strrchr(input_parameters.filename, OS_SLASH);
    if (slpos)
    {
      *slpos = '\0';
      sprintf(filename, "%s%c%04d%s", input_parameters.filename, OS_SLASH, prefix, slpos+1);
      *slpos = OS_SLASH;
    }
    else
    {
      sprintf(filename, "%04d%s", prefix, input_parameters.filename);
    }
    log_progress(4, _("saving file %s"), filename);
    if (image_save_pnm(&hopfield.imageR, &hopfield.imageG, &hopfield.imageB,
          input_parameters.binary, filename) == -1)
    {
      fprintf(stderr, _("%s: failed to save PNM file '%s': %s\n"), prog_name, filename, strerror(errno));
      exit(1);
    }
    xfree(filename);
  }
  else
  {
    if (image_save_pnm_file(&hopfield.imageR, &hopfield.imageG, &hopfield.imageB,
          input_parameters.binary, stdout) == -1)
    {
      log_progress(4, _("saving to stdout"));
      fprintf(stderr, _("%s: failed to write PNM file to stdout: %s\n"), prog_name, strerror(errno));
      exit(1);
    }

  }
}

static void get_lambdas(real_t* lambda, real_t* lambda_min)
{
  *lambda_min = (real_t)(1.0 / exp(input_parameters.lambda_min / 4.0));
  *lambda = (real_t)input_parameters.lambda / (real_t)LAMBDA_MAX;
  *lambda *= (real_t)0.001 / *lambda_min;
}

static void progress_bar_init()
{
}

static void progress_bar_destroy()
{
  if (input_parameters.verbose > 0 && progress_output)
  {
    progress_output = 0;
    fprintf(stderr, "\n");
  }
}

static void progress_bar_update(real_t part)
{
  if (input_parameters.verbose > 0)
  {
    progress_output = 1;
    fprintf(stderr, _("%5.1f%% completed"), 100.0 * (double)part);
    fprintf(stderr, "\r");
  }
}

static void compute()
{
  int i;
  real_t lambda_min, lambda;
  real_t step, final;
  int is_rgb, is_adaptive, is_smooth, is_mirror;
  convmask_t defoc, gauss;
  /*
  convmask_t motion, blur;;
  */

  log_progress(2, "computation starts");

  get_lambdas(&lambda, &lambda_min);

  is_rgb = image_parameters.img_bpp >= 3;
  is_smooth = (lambda > 1e-8 && lambda_min < LAMBDAMIN_USABLE_MAX);
  is_adaptive = (input_parameters.adaptive_smooth && is_smooth);
  is_mirror = (input_parameters.boundary == BOUNDARY_MIRROR);

  if (is_mirror)
  {
    log_progress(2, _("mirror boundaries"));
  }
  else
  {
    log_progress(2, _("periodical boundaries"));
  }

  /* PROGRESS BAR */
  step = R(1.0);
  final = (real_t)input_parameters.iterations;
  if (is_adaptive)
  {
    log_progress(2, _("adaptive noise reduction"));
    final *= 2;
  }
  else if (is_smooth)
  {
    log_progress(2, _("noise reduction"));
    final++;
  }
  if (is_rgb)
  {
    log_progress(2, _("RGB picture"));
    final *= R(3.0);
  }
  else
  {
    log_progress(2, _("gray picture"));
  }
  progress_bar_init();

  blur_create_defocus(&defoc, (real_t)input_parameters.radius);
  blur_create_gauss(&gauss, (real_t)input_parameters.gauss);
  /*
  blur_create_motion(&motion, (real_t)input_parameters.motion, (real_t)input_parameters.mot_angle);
  */
  convmask_convolve(&hopfield.blur, &defoc, &gauss);
  /*
  convmask_convolve(&hopfield.blur, &blur, &motion);
  */
  convmask_destroy(&gauss);
  convmask_destroy(&defoc);
  /*
  convmask_destroy(&motion);
  */


  if (is_smooth)
  {
    blur_create_gauss(&hopfield.filter, R(1.0));
    lambda_set_mirror(&hopfield.lambdafldR, is_mirror);
    lambda_set_nl(&hopfield.lambdafldR, TRUE);
    lambda_create(&hopfield.lambdafldR, image_parameters.width, image_parameters.height,
      lambda_min, input_parameters.winsize, &hopfield.filter);
    if (is_rgb)
    {
      lambda_set_mirror(&hopfield.lambdafldG, is_mirror);
      lambda_set_mirror(&hopfield.lambdafldB, is_mirror);
      lambda_set_nl(&hopfield.lambdafldG, TRUE);
      lambda_set_nl(&hopfield.lambdafldB, TRUE);
      lambda_create(&hopfield.lambdafldG, image_parameters.width,
        image_parameters.height, lambda_min, input_parameters.winsize, &hopfield.filter);
      lambda_create(&hopfield.lambdafldB, image_parameters.width,
        image_parameters.height, lambda_min, input_parameters.winsize, &hopfield.filter);
    }
  }

  if (is_smooth && !is_adaptive)
  {
    lambda_calculate(&hopfield.lambdafldR, &hopfield.imageR);
    progress_bar_update(step++ / final);
    if (is_rgb)
    {
      lambda_calculate(&hopfield.lambdafldG, &hopfield.imageG);
      progress_bar_update(step++ / final);

      lambda_calculate(&hopfield.lambdafldB, &hopfield.imageB);
      progress_bar_update(step++ / final);
    }
  }

  hopfield.hopfieldR.lambda = lambda;
  hopfield_set_mirror(&hopfield.hopfieldR, is_mirror);
  if (is_smooth)
  {
    hopfield_create(&hopfield.hopfieldR, &hopfield.blur, &hopfield.imageR, &hopfield.lambdafldR);
  }
  else
  {
    hopfield_create(&hopfield.hopfieldR, &hopfield.blur, &hopfield.imageR, NULL);
  }
  if (is_rgb)
  {
    hopfield.hopfieldG.lambda = lambda;
    hopfield.hopfieldB.lambda = lambda;
    hopfield_set_mirror(&hopfield.hopfieldG, is_mirror);
    hopfield_set_mirror(&hopfield.hopfieldB, is_mirror);
    if (is_smooth)
    {
      hopfield_create(&hopfield.hopfieldG, &hopfield.blur, &hopfield.imageG, &hopfield.lambdafldG);
      hopfield_create(&hopfield.hopfieldB, &hopfield.blur, &hopfield.imageB, &hopfield.lambdafldB);
    }
    else
    {
      hopfield_create(&hopfield.hopfieldG, &hopfield.blur, &hopfield.imageG, NULL);
      hopfield_create(&hopfield.hopfieldB, &hopfield.blur, &hopfield.imageB, NULL);
    }
  }

  log_progress(2, _("starting iterations"));

  for (i = 1; i <= input_parameters.iterations; i++)
  {
    if (is_adaptive)
    {
      lambda_calculate(&hopfield.lambdafldR, &hopfield.imageR);
      progress_bar_update(step++ / final);

      if (is_rgb)
      {
        lambda_calculate(&hopfield.lambdafldG, &hopfield.imageG);
        progress_bar_update(step++ / final);
        lambda_calculate(&hopfield.lambdafldB, &hopfield.imageB);
        progress_bar_update(step++ / final);
      }
    }
    hopfield_iteration(&hopfield.hopfieldR);
    progress_bar_update(step++ / final);

    if (is_rgb)
    {
      hopfield_iteration(&hopfield.hopfieldG);
      progress_bar_update(step++ / final);
      hopfield_iteration(&hopfield.hopfieldB);
      progress_bar_update(step++ / final);
    }
    if (input_parameters.save_intermediate && i % input_parameters.save_intermediate == 0) hopfield_data_save(i);
  }

  if (!input_parameters.save_intermediate || (i-1) % input_parameters.save_intermediate != 0) hopfield_data_save(i-1);

  log_progress(2, _("finished iterations"));

  convmask_destroy(&hopfield.blur);
  if (is_smooth)
  {
    convmask_destroy(&hopfield.filter);
    lambda_destroy(&hopfield.lambdafldR);
  }
  hopfield_destroy(&hopfield.hopfieldR);
  if (is_rgb)
  {
    if (is_smooth)
    {
      lambda_destroy(&hopfield.lambdafldG);
      lambda_destroy(&hopfield.lambdafldB);
    }
    hopfield_destroy(&hopfield.hopfieldG);
    hopfield_destroy(&hopfield.hopfieldB);
  }
  progress_bar_destroy();
  log_progress(2, _("computation finished"));
}

int main (int argc, char **argv)
{
#ifdef HAVE_SETLOCALE
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");
#endif

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  textdomain (GETTEXT_PACKAGE);
#endif

  input_parameters_default();
  input_parameters_init(argc, argv);
  log_progress(8, _("processed input parameters"));
  hopfield_data_init();
  log_progress(8, _("data initialized"));
  compute ();
  hopfield_data_destroy();
  log_progress(8, _("data destroyed"));
  input_parameters_destroy();
  return 0;
}
