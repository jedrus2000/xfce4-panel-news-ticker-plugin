prefix = /usr
exec_prefix = $(prefix)/bin
bindir = $(exec_prefix)
datarootdir = $(prefix)/share/tickr
datadir = $(datarootdir)
tickrdir = $(datadir)/pixmaps

bin_PROGRAMS = tickr

tickr_SOURCES = tickr_main.c\
	tickr_resource.c\
	tickr_render.c\
	tickr_params.c\
	tickr_clock.c \
	tickr_feedparser.c\
	tickr_list.c\
	tickr_feedpicker.c\
	tickr_prefwin.c\
	tickr_otherwins.c\
	tickr_misc.c\
	tickr_helptext.c\
	tickr_opml.c\
	tickr_http.c\
	tickr_connectwin.c\
	tickr_quickfeedpicker.c\
	tickr_quicksetup.c\
	tickr_check4updates.c\
	tickr_error.c\
	tickr_tls.c

tickr_CFLAGS = -O2 -Wall -Wextra -Wunused-parameter -Wshadow -Wpointer-arith\
	-ffast-math -pedantic -Wno-variadic-macros -fmax-errors=5 $(GTK2_CFLAGS)\
	-Wno-deprecated-declarations -Wno-format -Wno-format-truncation\
	$(XML2_CFLAGS) $(GNUTLS_CFLAGS) $(FRIBIDI_CFLAGS)
#-Wno-deprecated-declarations for boring GTK+2 "'GTypeDebugFlags' is deprecated"

tickr_LDFLAGS = -Wl,-z,defs -Wl,--as-needed

tickr_LDADD = ../libetm-0.5.0/libetm.a $(GTK2_LIBS) $(XML2_LIBS) $(GNUTLS_LIBS)\
	$(FRIBIDI_LIBS) -lm

tickr_DATA = ../../images/tickr-icon.png ../../images/tickr-logo.png\
	../../images/tickr-rss-icon.png ../../images/tickr-icon.xpm
