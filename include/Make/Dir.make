
# common dependencies and rules for building subdirs

include $(MODULE_TOPDIR)/include/Make/Platform.make
include $(MODULE_TOPDIR)/include/Make/Grass.make
include $(MODULE_TOPDIR)/include/Make/Rules.make

subdirs:
	@list='$(SUBDIRS)'; \
	for subdir in $$list; do \
	    echo $$subdir ; \
	    (cd $$subdir && $(MAKE)) || exit 1; \
	done

cleansubdirs:
	@list='$(SUBDIRS)'; \
	for subdir in $$list; do \
	    echo $$subdir ; \
	    (cd $$subdir && $(MAKE) clean) || exit 1; \
	done

