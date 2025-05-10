
KEFIR_EXTERNAL_TEST_POSTGRESQL_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/postgresql

KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION := 17.2
KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE_FILENAME := postgresql-$(KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE := $(KEFIR_EXTERNAL_TEST_POSTGRESQL_DIR)/$(KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_POSTGRESQL_DIR)/postgresql-$(KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION)
KEFIR_EXTERNAL_TEST_POSTGRESQL_URL := https://ftp.postgresql.org/pub/source/v$(KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION)/postgresql-$(KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION).tar.gz

KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE_SHA256 := 51d8cdd6a5220fa8c0a3b12f2d0eeb50fcf5e0bdb7b37904a9cdff5cf1e61c36

$(KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_POSTGRESQL_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_ARCHIVE_FILENAME)"
	@echo "Applying $(SOURCE_DIR)/tests/external/postgresql/postgresql-17.2.patch..."
	@cd "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)" && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/postgresql/postgresql-17.2.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)/config.log: $(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring PostgreSQL $(KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-isystem $(realpath $(SOURCE_DIR))/tests/external/postgresql/include -O1 -g -fPIC" \
		LC_ALL=C.UTF-8 \
		./configure

$(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)/src/bin/psql: $(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)/config.log
	@echo "Building PostgreSQL $(KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-isystem $(realpath $(SOURCE_DIR))/tests/external/postgresql/include -O1 -g -fPIC" \
		LC_ALL=C.UTF-8 \
		$(MAKE) -f Makefile

$(KEFIR_EXTERNAL_TEST_POSTGRESQL_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)/src/bin/psql
	@echo "Testing PostgreSQL $(KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-isystem $(realpath $(SOURCE_DIR))/tests/external/postgresql/include -O1 -g -fPIC" \
		LC_ALL=C.UTF-8 \
		bash -c 'set -o pipefail; $(MAKE) -f Makefile check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/postgresql.test.done: $(KEFIR_EXTERNAL_TEST_POSTGRESQL_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/postgresql/validate.sh "$(KEFIR_EXTERNAL_TEST_POSTGRESQL_DIR)/tests.log"
	@touch "$@"
	@echo "PostgreSQL $(KEFIR_EXTERNAL_TEST_POSTGRESQL_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/postgresql.test.done
