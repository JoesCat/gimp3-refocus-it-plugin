## Process this file with automake to produce Makefile.in

SUBDIRS = po src gimp-plugin

EXTRA_DIST = README README.md img/defocus.pgm img/restored1.jpg \
	     img/restored2.jpg img/restored3.jpg img/restored.pgm
nodist_EXTRA_DATA = .git .github .dep .lib
DISTCHECK_CONFIGURE_FLAGS = --disable-silent-rules --enable-debugtime

DISTCLEANFILES = 

strip:
	${STRIP} ${builddir}/gimp-plugin/refocus-it

