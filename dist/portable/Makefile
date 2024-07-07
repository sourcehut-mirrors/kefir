BOOTSTRAP=no
ROOT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))/../..
WORK_DIR=$(ROOT_DIR)/bin/portable

all: portable

clean:
	rm -rf "$(WORK_DIR)"

ifeq ($(BOOTSTRAP), yes)
portable: $(WORK_DIR)/stage2/bin/kefir
	@echo "Packaging stage2 portable kefir distribution..."
	@cd "$(WORK_DIR)/stage2" && tar cfz "$(WORK_DIR)/kefir-portable-$(shell $(WORK_DIR)/stage2/bin/kefir -v).tar.gz" .
else
portable: $(WORK_DIR)/stage1/bin/kefir
	@echo "Packaging stage1 portable kefir distribution..."
	@cd "$(WORK_DIR)/stage1" && tar cfz "$(WORK_DIR)/kefir-portable-$(shell $(WORK_DIR)/stage0/bin/kefir -v).tar.gz" .
endif

$(WORK_DIR)/stage0/bin/kefir:
	@echo "Bootstrapping stage0 portable kefir distribution..."
	@unset KEFIR_RTINC && \
		unset KEFIR_RTLIB && \
		$(MAKE) -f "$(ROOT_DIR)/dist/portable/Makefile.stage" WORK_DIR="$(WORK_DIR)/stage0.work" TARGET_DIR="$(WORK_DIR)/stage0" all

$(WORK_DIR)/stage1/bin/kefir: $(WORK_DIR)/stage0/bin/kefir
	@echo "Bootstrapping stage1 portable kefir distribution..."
	@unset KEFIR_RTINC && \
		unset KEFIR_RTLIB && \
		$(MAKE) -f "$(ROOT_DIR)/dist/portable/Makefile.stage" \
		WORK_DIR="$(WORK_DIR)/stage1.work" TARGET_DIR="$(WORK_DIR)/stage1" \
		CC="$(WORK_DIR)/stage0/bin/kefir" \
		AS="$(WORK_DIR)/stage0/bin/as" \
		LD="$(WORK_DIR)/stage0/bin/ld" \
		all

$(WORK_DIR)/stage2/bin/kefir: $(WORK_DIR)/stage1/bin/kefir
	@echo "Bootstrapping stage2 portable kefir distribution..."
	@unset KEFIR_RTINC && \
		unset KEFIR_RTLIB && \
		$(MAKE) -f "$(ROOT_DIR)/dist/portable/Makefile.stage" \
		WORK_DIR="$(WORK_DIR)/stage2.work" TARGET_DIR="$(WORK_DIR)/stage2" \
		CC="$(WORK_DIR)/stage1/bin/kefir" \
		AS="$(WORK_DIR)/stage1/bin/as" \
		LD="$(WORK_DIR)/stage1/bin/ld" \
		all

.PHONY: all portable clean