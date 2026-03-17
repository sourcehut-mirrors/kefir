
KEFIR_EXTERNAL_TEST_PYTHON_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/python

KEFIR_EXTERNAL_TEST_PYTHON_VERSION := 3.14.3
KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE_FILENAME := python-$(KEFIR_EXTERNAL_TEST_PYTHON_VERSION).tar.xz
KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE := $(KEFIR_EXTERNAL_TEST_PYTHON_DIR)/$(KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_PYTHON_DIR)/Python-$(KEFIR_EXTERNAL_TEST_PYTHON_VERSION)
KEFIR_EXTERNAL_TEST_PYTHON_URL := https://www.python.org/ftp/python/$(KEFIR_EXTERNAL_TEST_PYTHON_VERSION)/Python-$(KEFIR_EXTERNAL_TEST_PYTHON_VERSION).tar.xz

KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE_SHA256 := a97d5549e9ad81fe17159ed02c68774ad5d266c72f8d9a0b5a9c371fe85d902b

$(KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_PYTHON_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_PYTHON_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_PYTHON_DIR)" && xz -d "$(KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE_FILENAME)" --stdout > "$(basename $(KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE_FILENAME))"
	@cd "$(KEFIR_EXTERNAL_TEST_PYTHON_DIR)" && tar xvf "$(basename $(KEFIR_EXTERNAL_TEST_PYTHON_ARCHIVE_FILENAME))"
	@cd "$(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)" && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/python/python.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring python $(KEFIR_EXTERNAL_TEST_PYTHON_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-isystem $(realpath $(SOURCE_DIR))/tests/external/python/include" \
		LC_ALL=C.UTF-8 \
		./configure

$(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)/python: $(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)/Makefile
	@echo "Building python $(KEFIR_EXTERNAL_TEST_PYTHON_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_PYTHON_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)/python
	@echo "Testing python $(KEFIR_EXTERNAL_TEST_PYTHON_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_PYTHON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		bash -c "set -o pipefail; ./python -m test -x test_gdb test_remote_pdb test_external_inspection test_socket test_socketserver  -i "test.test_sys.TestRemoteExec.*" -i test.test_audit.AuditTest.test_sys_remote_exec -i test.test_interpreters.test_queues.LowLevelTests.test_highlevel_reloaded -i test.test_interpreters.test_channels.LowLevelTests.test_highlevel_reloade | tee ../tests.log.tmp"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/python.test.done: $(KEFIR_EXTERNAL_TEST_PYTHON_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/python/validate.sh "$(KEFIR_EXTERNAL_TEST_PYTHON_DIR)/tests.log"
	@touch "$@"
	@echo "Python $(KEFIR_EXTERNAL_TEST_PYTHON_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/python.test.done
