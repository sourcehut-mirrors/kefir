WEBAPP_KEFIR_BIN_DIR=$(KEFIR_BIN_DIR)/webapp
WEBAPP_INCLUDE_DIR=$(WEBAPP_KEFIR_BIN_DIR)/includes
WEBAPP_INCLUDE_LIST=$(WEBAPP_KEFIR_BIN_DIR)/include.list
WEBAPP_INCLUDE_EXTRA=

WEBAPP_MUSL_VERSION=1.2.5
WEBAPP_MUSL_ARCHIVE_FILENAME=musl-$(WEBAPP_MUSL_VERSION).tar.gz
WEBAPP_MUSL_ARCHIVE=$(KEFIR_BIN_DIR)/$(WEBAPP_MUSL_ARCHIVE_FILENAME)
WEBAPP_MUSL_URL=https://musl.libc.org/releases/$(WEBAPP_MUSL_ARCHIVE_FILENAME)
WEBAPP_MUSL_BUILD=$(KEFIR_BIN_DIR)/musl-$(WEBAPP_MUSL_VERSION)/.build.done
WEBAPP_MUSL_SHA256=a9a118bbe84d8764da0ea0d28b3ab3fae8477fc7e4085d90102b8596fc7c75e4

$(WEBAPP_MUSL_ARCHIVE):
	@mkdir -p $(shell dirname "$@")
	@echo "Downloading $@"
	@wget -O "$@.tmp" "https://musl.libc.org/releases/musl-$(WEBAPP_MUSL_VERSION).tar.gz"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(WEBAPP_MUSL_SHA256)"
	@mv "$@.tmp" "$@"

$(WEBAPP_MUSL_BUILD): $(WEBAPP_MUSL_ARCHIVE)
	@echo "Building $(shell dirname $(WEBAPP_MUSL_BUILD))"
	@tar xvf $^ -C $(shell dirname $(shell dirname $(WEBAPP_MUSL_BUILD)))
	@cd $(shell dirname $(WEBAPP_MUSL_BUILD)) && \
		./configure --prefix=/musl && \
		$(MAKE) clean && \
		$(MAKE) && \
		$(MAKE) install DESTDIR=./dist
	@touch $@

$(WEBAPP_INCLUDE_DIR): $(WEBAPP_MUSL_BUILD)
	@echo "Collecting $@"
	@mkdir -p $@/musl $@/kefir $@/extra
	@find $(shell dirname $(WEBAPP_MUSL_BUILD))/dist/musl/include -mindepth 1 -maxdepth 1 -exec cp -r {} $(WEBAPP_INCLUDE_DIR)/musl \;
	@find $(HEADERS_DIR)/kefir/runtime -mindepth 1 -maxdepth 1 -exec cp -r {} $(WEBAPP_INCLUDE_DIR)/kefir \;
ifneq ($(WEBAPP_INCLUDE_EXTRA),)
	@find $(WEBAPP_INCLUDE_EXTRA) -mindepth 1 -maxdepth 1 -exec cp -r {} $(WEBAPP_INCLUDE_DIR)/extra \;
endif

$(WEBAPP_INCLUDE_LIST): $(WEBAPP_INCLUDE_DIR)
	@echo "Generating $@"
	@find $(WEBAPP_INCLUDE_DIR) -type f -exec $(REALPATH) --relative-to=$(WEBAPP_INCLUDE_DIR) {} \; > $@

$(WEBAPP_KEFIR_BIN_DIR)/%: $(SOURCE_DIR)/web/app/%
	@mkdir -p $(shell dirname "$@")
	@echo "Copying $@"
	@cp $^ $@

$(WEBAPP_KEFIR_BIN_DIR)/kefir.js: $(KEFIR_JS)
	@mkdir -p $(shell dirname "$@")
	@echo "Copying $@"
	@cp $^ $@

$(WEBAPP_KEFIR_BIN_DIR)/kefir.wasm: $(KEFIR_JS)
	@mkdir -p $(shell dirname "$@")
	@echo "Copying $@"
	@cp $(shell dirname "$^")/kefir.wasm "$@"

$(WEBAPP_KEFIR_BIN_DIR)/kefir.pdf: $(DOCS_MAN_DIR)/kefir.1
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(MDOC_CONV) -Tpdf "$<" > "$@"

WEBAPP += $(WEBAPP_INCLUDE_LIST) \
	      $(WEBAPP_KEFIR_BIN_DIR)/index.html \
	      $(WEBAPP_KEFIR_BIN_DIR)/playground.js \
	      $(WEBAPP_KEFIR_BIN_DIR)/kefir.js \
	      $(WEBAPP_KEFIR_BIN_DIR)/kefir.wasm \
	      $(WEBAPP_KEFIR_BIN_DIR)/playground.css \
		  $(WEBAPP_KEFIR_BIN_DIR)/kefir.pdf
