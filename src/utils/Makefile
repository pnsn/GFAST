# Makefile for GFAST/src/utils
# This directory includes files that are obtained from
# the ISTI ISCL software package
#
GFASTROOT=..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

OBJS	= timeutils.o fileutils.o\
	iscl_applyPermutations.o iscl_argmax.o iscl_argmin.o \
	iscl_argsort.o iscl_calloc.o iscl_free.o iscl_issorted.o \
    iscl_isfile.o iscl_isdir.o iscl_makedirs.o iscl_lstsq.o \
	iscl_percentile.o iscl_verbosity.o iscl_malloc.o iscl_mkdir.o\
	iscl_sort.o iscl_abs.o iscl_set.o iscl_copy.o iscl_min.o\
	iscl_zeros.o iscl_absArgmin.o iscl_reverse.o

LIB     =
DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config ${GFAST_VERSION}

INCL     = -I ../../include/ $(LAPACKE_INCL)

SYSLIBS	 = -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.o:    %.c
	$(cc) -c $< -o $@ $(CCFLAGS) $(DEBUG) $(INCL)

ids:    *.cc *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.cc *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

# depend
-include $(DEPENDFILES)

depend:
	touch $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES) -a -Y -- $(CCFLAGS) $(INCL) *.cc

cleandepend:
	rm -rf $(DEPENDFILES)

clean:
	-rm -f *.o $(BIN)

veryclean: clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/
