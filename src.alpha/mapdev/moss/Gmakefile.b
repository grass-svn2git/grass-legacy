LIBDIG = $(SRC)/mapdev/lib
#  DIGLIB = $(LIBDIG)/libdig.a

EXTRA_CFLAGS = -I$(LIBDIG)

OFILES = \
	v.out.moss.o \
	site_to_moss.o \
        pnt_to_moss.o \
	line_to_moss.o \
	area_to_moss.o \
        prune_points.o \
        store_points.o \
	wr_moss_head.o \
	wr_moss_coor.o \
        get_isle_xy.o

all: $(BIN_ALPHA_INTER)/v.out.moss

$(BIN_ALPHA_INTER)/v.out.moss: $(OFILES)
	cc $(LDFLAGS) -o $@ $(OFILES) $(DIGLIB) $(GISLIB) $(MATHLIB)

$(OFILES):	$(LIBDIG)/digit.h $(LIBDIR)/gis.h

