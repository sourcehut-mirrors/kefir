$(BIN_HEADERS_DESTDIR)/runtime/amd64.binary.h: $(BIN_HEADERS_SRCDIR)/runtime/amd64.s
$(BIN_HEADERS_DESTDIR)/runtime/amd64.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_SRCDIR)/runtime/amd64.s --zero
$(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h: $(BIN_HEADERS_SRCDIR)/compiler/predefined_defs.h
$(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_SRCDIR)/compiler/predefined_defs.h --zero
$(BIN_HEADERS_DESTDIR)/cc1/help.binary.h: $(GENERATED_HELP_DIR)/kefir-cc1.1.txt
$(BIN_HEADERS_DESTDIR)/cc1/help.binary.h: BINARY_HEADER_CONTENT=$(GENERATED_HELP_DIR)/kefir-cc1.1.txt --zero
$(BIN_HEADERS_DESTDIR)/driver/help.binary.h: $(GENERATED_HELP_DIR)/kefir.1.txt
$(BIN_HEADERS_DESTDIR)/driver/help.binary.h: BINARY_HEADER_CONTENT=$(GENERATED_HELP_DIR)/kefir.1.txt --zero

$(BIN_HEADERS_DESTDIR)/compiler/profile.deps: $(BIN_HEADERS_DESTDIR)/runtime/amd64.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@")/../runtime -DKEFIR_AMD64_RUNTIME_INCLUDE=amd64.binary.h' > $@

$(BIN_HEADERS_DESTDIR)/compiler/compiler.deps: $(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo "-I$(shell dirname "$@") -DKEFIR_COMPILER_PREDEFINED_DEFS_INCLUDE=predefined_defs.binary.h" > $@
	
$(BIN_HEADERS_DESTDIR)/cc1/cc1.deps: $(BIN_HEADERS_DESTDIR)/cc1/help.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@") -DKEFIR_CC1_HELP_INCLUDE=help.binary.h' > $@

$(BIN_HEADERS_DESTDIR)/driver/main.deps: $(BIN_HEADERS_DESTDIR)/driver/help.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@") -DKEFIR_DRIVER_HELP_INCLUDE=help.binary.h' > $@

$(BIN_HEADERS_DESTDIR)/web/main.deps: $(BIN_HEADERS_DESTDIR)/driver/help.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@")/../driver -DKEFIR_DRIVER_HELP_INCLUDE=help.binary.h' > $@