# Toplevel Makefile for GFAST

SUBDIRS=src

.PHONY: all show-targets $(SUBDIRS)
show-targets:
	@echo TARGETS:  $(TARGET_LIST)
TARGET_LIST=show-targets

all: $(SUBDIRS)

# define macro to generate rules for target, list of sub targets and rule for each.
define gen_recursive_targets
.PHONY: $(1) $(2:%=$(1)-%)
$(1): $(2:%=$(1)-%)
$(2:%=$(1)-%):
	$(MAKE) -C $$(@:$$$(1)-%=%) $1
	@echo -e ""
TARGET_LIST+= $(1) $(2:%=$(1)-%)
endef

# use macro to define recursive targets
$(eval $(call gen_recursive_targets, all, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, ids, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, rm-ids, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, clean, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, depend, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, veryclean, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, ut, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, test, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, docs, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, cleancoverage, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, buildcoverage, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, runcoverage, $(SUBDIRS)))

ut: # no-op

test: # no-op

# test report
GTEST_OUTPUT_DIR = ../utsummary/gfast
allreports testreport: export DO_GTEST_OUTPUT = 1
allreports: cleantestreport coverage
testreport: cleantestreport test

cleantestreport:
	-rm -rf $(GTEST_OUTPUT_DIR)

# coverage
INFODIR = ../coverage/info
COVDIR = ../coverage/html

covdirs:
	if [ ! -d "$(COVDIR)" ]; then mkdir -p $(COVDIR); fi
	if [ ! -d "$(INFODIR)" ]; then mkdir -p $(INFODIR); fi

coverage: cleancoverage covdirs buildcoverage runcoverage
	genhtml $(INFODIR)/*.info -o $(COVDIR) -s --legend

cleancoverage:
	-rm -rf $(COVDIR)

printvars:
	@echo SUBDIRS=$(SUBDIRS)
