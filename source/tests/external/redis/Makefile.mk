
KEFIR_EXTERNAL_TEST_REDIS_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/redis

KEFIR_EXTERNAL_TEST_REDIS_VERSION := 8.2.0
KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE_FILENAME := $(KEFIR_EXTERNAL_TEST_REDIS_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE := $(KEFIR_EXTERNAL_TEST_REDIS_DIR)/$(KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_REDIS_DIR)/redis-$(KEFIR_EXTERNAL_TEST_REDIS_VERSION)
KEFIR_EXTERNAL_TEST_REDIS_URL := https://github.com/redis/redis/archive/refs/tags/$(KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE_SHA256 := c64219bdcba407d18c8dde1fb87b86945aebf75e60f5b44ff463785a962645ed

$(KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_REDIS_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_REDIS_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_REDIS_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_REDIS_ARCHIVE_FILENAME)"
	@echo "Pathcing Redis $(KEFIR_EXTERNAL_TEST_REDIS_VERSION)..."
	@cd $(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR) && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/redis/redismodule.patch"
	@find $(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR) -type f -name "*.h" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@find $(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR) -type f -name "*.c" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@rm "$(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR)/tests/integration/logging.tcl"
# Unstable in runtime
	@rm "$(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR)/tests/integration/replication-rdbchannel.tcl"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR)/src/redis-server: $(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Building redis $(KEFIR_EXTERNAL_TEST_REDIS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		$(MAKE) V=1 MALLOC=libc CC="$(realpath $(KEFIR_EXE))" CFLAGS="-include $(realpath $(SOURCE_DIR))/tests/external/redis/include/kefir_atomic_builtins.h -isystem $(realpath $(SOURCE_DIR))/tests/external/redis/include"

$(KEFIR_EXTERNAL_TEST_REDIS_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR)/src/redis-server
	@echo "Testing redis $(KEFIR_EXTERNAL_TEST_REDIS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_REDIS_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		bash -c 'set -o pipefail; $(MAKE) V=1 MALLOC=libc CC="$(realpath $(KEFIR_EXE))" CFLAGS="-include $(realpath $(SOURCE_DIR))/tests/external/redis/include/kefir_atomic_builtins.h -isystem $(realpath $(SOURCE_DIR))/tests/external/redis/include" test 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/redis.test.done: $(KEFIR_EXTERNAL_TEST_REDIS_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/redis/validate.sh "$(KEFIR_EXTERNAL_TEST_REDIS_DIR)/tests.log"
	@touch "$@"
	@echo "Redis $(KEFIR_EXTERNAL_TEST_REDIS_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/redis.test.done
