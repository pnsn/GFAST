# Makefile for GFAST/src/traceBuffer

EEWDIR = ../../..
include $(EEWDIR)/Make.include.$(shell uname)

EW_INCL	= -I$(EWDIR)/include

#ifdef GFAST_USE_EW
CFLAGS+=$(GFAST_USE_EW)
#endif
#ifdef GFAST_USE_AMQ
CFLAGS+=$(GFAST_USE_AMQ)
#endif
#ifdef GFAST_USE_DMLIB
CFLAGS+=$(GFAST_USE_DMLIB)
#endif

DEBUG	= -g

INCL            = -I ../../include $(HDF5_INCL) $(EW_INCL) $(CBLAS_INCL) $(ISCL_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

SDIRS	= ewrr h5
LIB	= traceBuffer.a
OBJS := $(foreach dir,$(SDIRS),$(dir)/*.o) 

all: $(LIB)
lib: $(LIB)
# %.o:    %.c
# 	$(cc) -c $< $(CFLAGS) $(DEBUG) $(INCL)

# ids:    *.c
#         $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

# rm-ids:    *.c
#         $(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

$(LIB): $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir all ; \
	done
	$(AR) $(LIB) $(OBJS)

ids: $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir ids ; \
	done

rm-ids: $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir rm-ids ; \
	done

install:
	for dir in $(SDIRS) ; do \
		make -C $$dir install ; \
	done

docs:
	doxygen doxygen.conf

cleandocs:
	rm -rf ../docs/epic

clean:
	-rm -f *.o $(LIB)
	for dir in $(SDIRS) ; do \
		make -C $$dir clean ; \
	done

veryclean: cleandocs
	for dir in $(SDIRS) ; do \
		make -C $$dir veryclean ; \
	done

depend:
	for dir in $(SDIRS) ; do \
		make -C $$dir depend ; \
	done

test: # no-op

# coverage
cleancoverage: clean
	-rm -rf *.gcno *.gcda *.gcov *.info
	for dir in $(SDIRS) ; do \
		make -C $$dir cleancoverage ; \
	done

buildcoverage:
	for dir in $(SDIRS) ; do \
		make -C $$dir buildcoverage ; \
	done
	$(AR) $(LIB) $(OBJS)

prepcoverage:
	$(eval CCFLAGS=${CCFLAGS} -fprofile-arcs -ftest-coverage)
	$(eval CFLAGS=${CFLAGS} -fprofile-arcs -ftest-coverage)
