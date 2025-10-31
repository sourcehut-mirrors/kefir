KEFIR_EXTERNAL_TEST_GCC_474_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/gcc-474-bootstrap

KEFIR_EXTERNAL_TEST_GCC_474_VERSION := 4.7.4
KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_FILENAME := gcc-$(KEFIR_EXTERNAL_TEST_GCC_474_VERSION).tar.bz2
KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE := $(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/$(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_GCC_474_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/gcc-$(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)
KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR := $(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/gcc-$(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)-build
KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR := $(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/gcc-$(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)-install
KEFIR_EXTERNAL_TEST_GCC_474_URL := https://ftp.mpi-inf.mpg.de/mirrors/gnu/mirror/gcc.gnu.org/pub/gcc/releases/gcc-$(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)/$(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_SHA256 := 92e61c6dc3a0a449e62d72a38185fda550168a86702dea07125ebd3ec3996282

$(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_GCC_474_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_GCC_474_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)" && \
		bunzip2 -d < "$(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_FILENAME)" > "$(basename $(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_FILENAME))"
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)" && tar xvf "$(basename $(KEFIR_EXTERNAL_TEST_GCC_474_ARCHIVE_FILENAME))"
	@echo "Applying $(SOURCE_DIR)/tests/external/gcc-474-bootstrap/gcc-474.patch"
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_474_SOURCE_DIR)" && \
		patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/gcc-474-bootstrap/gcc-474.patch"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)" "$(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)"
	@mkdir "$(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)"
	@mkdir "$(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring gcc $(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC=$(realpath $(KEFIR_EXE)) \
		KEFIR_AS=$(AS) \
		KEFIR_LD=$(LD) \
		$(realpath $(KEFIR_EXTERNAL_TEST_GCC_474_SOURCE_DIR))/configure --enable-languages=c \
			--prefix=$(realpath $(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)) \
			--enable-languages=c,c++ \
			--disable-build-poststage1-with-cxx \
			--disable-build-with-cxx \
			--disable-multilib \
			--enable-cloog-backend='isl'

$(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)/gcc/xgcc: $(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)/Makefile
	@echo "Building gcc $(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		KEFIR_AS=$(AS) \
		KEFIR_LD=$(LD) \
		LD_PRELOAD='/usr/lib/libstdc++.so' \
		$(MAKE) bootstrap

$(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/bin/gcc: $(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)/gcc/xgcc
	@echo "Installing gcc $(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_474_BUILD_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		KEFIR_AS=$(AS) \
		KEFIR_LD=$(LD) \
		$(MAKE) install -j1

$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/test.log: $(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/bin/gcc
	@echo "Validating gcc $(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)..."
	@"$(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/bin/gcc" \
		-O2 "$(SOURCE_DIR)/tests/external/gcc-474-bootstrap/test.c" -o "$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/test"
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)" && ./test "test case" | tee "test.log.tmp"
	@diff "$@.tmp" "$(SOURCE_DIR)/tests/external/gcc-474-bootstrap/test.log.expected"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/test.cxx.log: $(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/bin/gcc
	@echo "Validating g++ $(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)..."
	@"$(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/bin/g++" \
		-O2 "$(SOURCE_DIR)/tests/external/gcc-474-bootstrap/test.cpp" -o "$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/test.cxx"
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_474_DIR)" && ./test.cxx "test case" | tee "test.cxx.log.tmp"
	@diff "$@.tmp" "$(SOURCE_DIR)/tests/external/gcc-474-bootstrap/test.cxx.log.expected"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/gcc-474-bootstrap.test.done: $(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/test.log $(KEFIR_EXTERNAL_TEST_GCC_474_DIR)/test.cxx.log
	@touch "$@"
	@echo "Successfully validated gcc $(KEFIR_EXTERNAL_TEST_GCC_474_VERSION)"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/gcc-474-bootstrap.test.done
