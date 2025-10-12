BOOTSTRAP=no
ROOT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))/../..
BIN_DIR := $(ROOT_DIR)/bin
WORK_DIR := $(BIN_DIR)/portable
BUILD_LIBGCC := no

all: portable

clean:
	rm -rf "$(WORK_DIR)"

ifeq ($(BUILD_LIBGCC), yes)
STAGE0_LIBGCC_DEP=$(WORK_DIR)/libgcc/libgcc-474/libgcc.a
LIBGCC_DIR=$(WORK_DIR)/libgcc/libgcc-474
else
STAGE0_LIBGCC_DEP=
LIBGCC_DIR=
endif

ifeq ($(BOOTSTRAP), yes)
portable: $(WORK_DIR)/stage2/README
	@echo "Packaging stage2 portable kefir distribution..."
	@cd "$(WORK_DIR)/stage2" && tar cfz "$(WORK_DIR)/kefir-portable-$(shell $(WORK_DIR)/stage2/bin/kefir -v).tar.gz" .
else
portable: $(WORK_DIR)/stage1/README
	@echo "Packaging stage1 portable kefir distribution..."
	@cd "$(WORK_DIR)/stage1" && tar cfz "$(WORK_DIR)/kefir-portable-$(shell $(WORK_DIR)/stage0/bin/kefir -v).tar.gz" .
endif

$(WORK_DIR)/stage0/README: $(STAGE0_LIBGCC_DEP)
	@echo "Bootstrapping stage0 portable kefir distribution..."
	@unset KEFIR_RTINC && \
		$(MAKE) -f "$(ROOT_DIR)/dist/portable/Makefile.stage" WORK_DIR="$(WORK_DIR)/stage0.work" LIBGCC_DIR="$(LIBGCC_DIR)" TARGET_DIR="$(WORK_DIR)/stage0" all

$(WORK_DIR)/stage1/README: $(WORK_DIR)/stage0/README
	@echo "Bootstrapping stage1 portable kefir distribution..."
	@unset KEFIR_RTINC && \
		$(MAKE) -f "$(ROOT_DIR)/dist/portable/Makefile.stage" \
		WORK_DIR="$(WORK_DIR)/stage1.work" TARGET_DIR="$(WORK_DIR)/stage1" \
		CC="$(WORK_DIR)/stage0/bin/kefir" \
		AS="$(WORK_DIR)/stage0/bin/as" \
		LD="$(WORK_DIR)/stage0/bin/ld" \
		LIBGCC_DIR="$(LIBGCC_DIR)" \
		all

$(WORK_DIR)/stage2/README: $(WORK_DIR)/stage1/README
	@echo "Bootstrapping stage2 portable kefir distribution..."
	@unset KEFIR_RTINC && \
		$(MAKE) -f "$(ROOT_DIR)/dist/portable/Makefile.stage" \
		WORK_DIR="$(WORK_DIR)/stage2.work" TARGET_DIR="$(WORK_DIR)/stage2" \
		CC="$(WORK_DIR)/stage1/bin/kefir" \
		AS="$(WORK_DIR)/stage1/bin/as" \
		LD="$(WORK_DIR)/stage1/bin/ld" \
		LIBGCC_DIR="$(LIBGCC_DIR)" \
		all

$(WORK_DIR)/libgcc/libgcc-474/libgcc.a:
	@echo "Bootstrapping libgcc..."
	@unset KEFIR_RTINC && \
		$(MAKE) -f "$(ROOT_DIR)/Makefile" KEFIR_BIN_DIR="$(WORK_DIR)/libgcc" bootstrap_libgcc474

.PHONY: all portable clean