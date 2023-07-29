EMCC=emcc
EMCC_CFGLAGS=$(CFLAGS)

ifeq ($(REALPATH),)
REALPATH=realpath
endif

WEB_BIN_DIR=$(BIN_DIR)/webapp
EMSCRIPTEN_BIN_DIR=$(BIN_DIR)/emscripten
KEFIR_JS=$(WEB_BIN_DIR)/kefir.js
WEB_INCLUDE_DIR=$(WEB_BIN_DIR)/includes
WEB_INCLUDE_LIST=$(WEB_BIN_DIR)/include.list
WEB_INCLUDE_EXTRA=

WEB_MUSL_VERSION=1.2.4
WEB_MUSL_ARCHIVE=$(BIN_DIR)/musl-$(WEB_MUSL_VERSION).tar.gz
WEB_MUSL_BUILD=$(BIN_DIR)/musl-$(WEB_MUSL_VERSION)/.build.done

KEFIR_EMCC_SOURCE = $(KEFIR_LIB_SOURCE)
KEFIR_EMCC_SOURCE += $(wildcard $(SOURCE_DIR)/web/*.c)
KEFIR_EMCC_SOURCE += $(filter-out $(SOURCE_DIR)/driver/main.c,$(wildcard $(SOURCE_DIR)/driver/*.c))

KEFIR_EMCC_DEPENDENCY_FILES := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.d)
KEFIR_EMCC_OBJECT_FILES := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(EMSCRIPTEN_BIN_DIR)/%.o)

$(EMSCRIPTEN_BIN_DIR)/runtime/%.h: $(SOURCE_DIR)/runtime/%.s $(SOURCE_DIR)/runtime/common_amd64.s
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@cat $^ | xxd -i > $@

$(EMSCRIPTEN_BIN_DIR)/compiler/predefined_defs.h: $(SOURCE_DIR)/compiler/predefined_defs.h
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@cat $^ | xxd -i > $@

$(EMSCRIPTEN_BIN_DIR)/driver/help.h: $(SOURCE_DIR)/driver/help.txt
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@cat $^ | xxd -i > $@

$(EMSCRIPTEN_BIN_DIR)/compiler/profile.o: $(EMSCRIPTEN_BIN_DIR)/runtime/opt_amd64_sysv.h
$(EMSCRIPTEN_BIN_DIR)/compiler/profile.o: $(EMSCRIPTEN_BIN_DIR)/runtime/amd64_sysv.h
$(EMSCRIPTEN_BIN_DIR)/compiler/profile.o: CFLAGS+=-I$(ROOT) -DKEFIR_OPT_AMD64_SYSV_RUNTIME_INCLUDE=$(EMSCRIPTEN_BIN_DIR)/runtime/opt_amd64_sysv.h -DKEFIR_AMD64_SYSV_RUNTIME_INCLUDE=$(EMSCRIPTEN_BIN_DIR)/runtime/amd64_sysv.h

$(EMSCRIPTEN_BIN_DIR)/compiler/compiler.o: $(EMSCRIPTEN_BIN_DIR)/compiler/predefined_defs.h
$(EMSCRIPTEN_BIN_DIR)/compiler/compiler.o: CFLAGS+=-I$(ROOT) -DKEFIR_COMPILER_PREDEFINED_DEFS_INCLUDE=$(EMSCRIPTEN_BIN_DIR)/compiler/predefined_defs.h

$(EMSCRIPTEN_BIN_DIR)/web/main.o: $(EMSCRIPTEN_BIN_DIR)/driver/help.h
$(EMSCRIPTEN_BIN_DIR)/web/main.o: CFLAGS+=-I$(ROOT) -DKEFIR_DRIVER_HELP_INCLUDE=$(EMSCRIPTEN_BIN_DIR)/driver/help.h

$(EMSCRIPTEN_BIN_DIR)/%.o: $(SOURCE_DIR)/%.c $(BIN_DIR)/%.d
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(EMCC) $(EMCC_CFGLAGS) $(INCLUDES) -c $< -o $@

$(KEFIR_JS): $(KEFIR_EMCC_OBJECT_FILES)
	@mkdir -p $(shell dirname "$@")
	@echo "Linking $@"
	@$(EMCC) $(EMCC_CFGLAGS) $^ -o $@ \
		-sMODULARIZE -s'EXPORT_NAME="createKefirModule"' \
		-sEXPORTED_FUNCTIONS=UTF8ToString,stringToUTF8Array,lengthBytesUTF8,_free,_malloc,_kefir_run_with_args \
		-sEXPORTED_RUNTIME_METHODS=ccall,cwrap,setValue,FS,ENV

$(WEB_MUSL_ARCHIVE):
	@mkdir -p $(shell dirname "$@")
	@echo "Downloading $@"
	@wget -O $@ "https://musl.libc.org/releases/musl-$(WEB_MUSL_VERSION).tar.gz"

$(WEB_MUSL_BUILD): $(WEB_MUSL_ARCHIVE)
	@echo "Building $(shell dirname $(WEB_MUSL_BUILD))"
	@tar xvf $^ -C $(shell dirname $(shell dirname $(WEB_MUSL_BUILD)))
	@cd $(shell dirname $(WEB_MUSL_BUILD)) && \
		./configure --prefix=/musl && \
		$(MAKE) clean && \
		$(MAKE) && \
		$(MAKE) install DESTDIR=./dist
	@touch $@

$(WEB_INCLUDE_DIR): $(WEB_MUSL_BUILD)
	@echo "Collecting $@"
	@mkdir -p $@/musl $@/kefir $@/extra
	@find $(shell dirname $(WEB_MUSL_BUILD))/dist/musl/include -mindepth 1 -maxdepth 1 -exec cp -r {} $(WEB_INCLUDE_DIR)/musl \;
	@find $(HEADERS_DIR)/kefir/runtime -mindepth 1 -maxdepth 1 -exec cp -r {} $(WEB_INCLUDE_DIR)/kefir \;
ifneq ($(WEB_INCLUDE_EXTRA),)
	@find $(WEB_INCLUDE_EXTRA) -mindepth 1 -maxdepth 1 -exec cp -r {} $(WEB_INCLUDE_DIR)/extra \;
endif

$(WEB_INCLUDE_LIST): $(WEB_INCLUDE_DIR)
	@echo "Generating $@"
	@find $(WEB_INCLUDE_DIR) -type f -exec $(REALPATH) --relative-to=$(WEB_INCLUDE_DIR) {} \; > $@

$(WEB_BIN_DIR)/%.html: $(SOURCE_DIR)/web/%.html
	@mkdir -p $(shell dirname "$@")
	@echo "Copying $@"
	@cp $^ $@

$(WEB_BIN_DIR)/%.js: $(SOURCE_DIR)/web/%.js
	@mkdir -p $(shell dirname "$@")
	@echo "Copying $@"
	@cp $^ $@

$(WEB_BIN_DIR)/%.css: $(SOURCE_DIR)/web/%.css
	@mkdir -p $(shell dirname "$@")
	@echo "Copying $@"
	@cp $^ $@

WEB += $(KEFIR_EMCC_DEPENDENCY_FILES) \
       $(KEFIR_EMCC_OBJECT_FILES) \
       $(KEFIR_JS) \
	   $(WEB_INCLUDE_LIST) \
	   $(WEB_BIN_DIR)/index.html \
	   $(WEB_BIN_DIR)/playground.js \
	   $(WEB_BIN_DIR)/playground.css
