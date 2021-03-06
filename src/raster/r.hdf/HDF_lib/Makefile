#
# Feb. 1998 Markus Neteler
#
# ##################################################################
#
#                   NCSA HDF Version 3.2 release 3
#                          December 1, 1992
#
#           MAKEFILE for creating the HDF library, libdf.a,
#
# This makefile creates the following FORTRAN and C interfaces 
# for HDF in a library called "libdf.a": 
#
#              DFR8   (8-bit raster image sets)
#              DF24   (24-bit raster image sets)
#              DFP    (palettes)
#              DFSD   (scientific data sets)
#              DFAN   (annotations)
#              V      (vsets)
#
# See the file INSTALL for further information on how to invoke
# this makefile.
#
# This release of HDF marks the first use of a new set of lower
# level routines.  The makefile also creates these interfaces,
# which are available in C only.  These routines are categorized 
# as follows:
#
#              H      (lower level i/o)
#              DF     (emulation of old lower level i/o routines)
#              HD     (lower level utilities for developers)
#              HE     (lower level error-handling)
#              HD     (lower level utilities, for developers)
#              DFK    (conversion routines)
#
# ##################################################################
#
#
#
#
# This is the top level Makefile to build HDF 3.2 on Unix based
#       platforms
#
# Possible Machine Types
#   SUN, IBM6000, HP9000, MIPSEL (Dec Mips based machines), ...
#

#define your sh shell
SHELL=/bin/sh

# Define your machine type
MACHINE=${MACH_TYPE}

# Define your C compiler and flags
#CC= cc
CFLAGS= -D${MACHINE} 

# Define your FORTRAN compiler
# FC= f77
FFLAGS=

# Location where the HDF include files are to be installed
HDFINC= `pwd`/include

# Location where the HDF library is to be installed
HDFLIB= `pwd`/LIB.linux

# Location to put HDF utility executables
HDFBIN= `pwd`/bin

# Name of library archiver and flags to send
AR= ar
ARFLAGS= r

# Name of achive randomizer (use 'touch' if non-existant)
RANLIB= ranlib

# Name of remove utility
RM= /bin/rm
RMFLAGS= -f 

#
#
# Flags to recursively send
#

HDF_FLAGS       = \
#        CC="$(CC)" \
        CFLAGS="$(CFLAGS)" \
#        FC="$(FC)" \
        FFLAGS="$(FFLAGS)" \
        RANLIB="$(RANLIB)" \
        AR="$(AR)" \
        ARFLAGS="$(ARFLAGS)" \
        RM="$(RM)" \
        RMFLAGS="$(RMFLAGS)" \
        MACHINE="$(MACHINE)" \
        HDFLIB="$(HDFLIB)" \
        HDFINC="$(HDFINC)" \
	HDFBIN="$(HDFBIN)"	

#
#
# General rules
#
all:
	@$(MAKE) $(MFLAGS) $(HDF_FLAGS) TARG=$@ \
          SUBDIRS="src util test" _sub message 

allnofortran:
	@$(MAKE) $(MFLAGS) $(HDF_FLAGS) TARG=nofortran \
          SUBDIRS="src util test" _sub message 

libnofortran:
	@$(MAKE) $(MYFLAGS) $(HDF_FLAGS) TARG=nofortran \
	  SUBDIRS="src" _sub message

libnostub: libnofortran

allnostub: allnofortran

libdf:
	@$(MAKE) $(MFLAGS) $(HDF_FLAGS) TARG=all \
          SUBDIRS="src" _sub

utilities:
	@$(MAKE) $(MFLAGS) $(HDF_FLAGS) TARG=all \
          SUBDIRS="util" _sub

tests:
	@$(MAKE) $(MFLAGS) $(HDF_FLAGS) TARG=all \
          SUBDIRS="test" _sub

#install:      $(INSTALL_DIRS)
#	@$(MAKE) $(MFLAGS) $(HDF_FLAGS) TARG=$@  \
#          SUBDIRS="src util test" _sub

clean:
	@$(MAKE) $(MFLAGS) $(HDF_FLAGS) TARG=$@  \
          SUBDIRS="src util test" _sub
	$(RM) $(RMFLAGS) core *.log

_sub:
	@for dir in $(SUBDIRS); do \
          (cd $$dir; echo Making \`$(TARG)\' in `pwd`; \
              $(MAKE) $(MFLAGS) $(HDF_FLAGS) $(TARG)); \
	done

message:
	@echo ""
	@echo "***********************************************************"
	@echo " HDF 3.2 library successfully created."
	@echo ""
	@echo "If you have any old vset files (v 1.0) you must use vcompat"
	@echo "to make them compatible with Vset 2.0 applications"
	@echo ""
	@echo "User programs can be created as follows:"
	@echo ""
	@echo "   cc -D${MACHINE} program.c libdf.a -o program"
	@echo ""
	@echo "***********************************************************"
	@echo ""
