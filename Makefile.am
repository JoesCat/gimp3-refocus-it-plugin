## Process this file with automake to produce Makefile.in

INCLUDES=@GTK_CFLAGS@ @GIMP_CFLAGS@

bin_PROGRAMS = refocus-it

noinst_HEADERS=blur.h compiler.h convmask.h image.h threshold.h xmalloc.h boundary.h hopfield.h lambda.h weights.h

refocus_it_SOURCES = blur.c boundary.c convmask.c hopfield.c image.c lambda.c main.c threshold.c weights.c xmalloc.c

refocus_it_LDADD = @GTK_LIBS@ -lgimp -lgimpui
refocus_it_DEPENDENCIES=

EXTRA_DIST=Makefile.msc demo

install-bin: refocus-it
	@if test "x@GIMPTOOL@" = "x" ; then \
		echo "Warning: configure did not find gimptool" ; \
		echo "install refocus-it manually in your plug-in directory" ; \
		echo "e.g.: cp refocus-it ~/.gimp-1.2/plug-ins" ; \
	else \
		@GIMPTOOL@ --install-bin refocus-it ; \
	fi

install-admin-bin: refocus-it
	@if test "x@GIMPTOOL@" = "x" ; then \
		echo "Warning: configure did not find gimptool" ; \
		echo "install refocus-it manually in your plug-in directory" ; \
		echo "e.g.: install refocus-it /usr/local/lib/gimp-1.2/plug-ins" ; \
	else \
		@GIMPTOOL@ --install-admin-bin refocus-it ; \
	fi

%.s: %.c
	${CC} ${CFLAGS} -S $< -o $@


