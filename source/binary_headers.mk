
$(BIN_HEADERS_DESTDIR)/runtime/opt_amd64_sysv.binary.h: $(BIN_HEADERS_SRCDIR)/runtime/opt_amd64_sysv.s $(BIN_HEADERS_SRCDIR)/runtime/common_amd64.inc.s
$(BIN_HEADERS_DESTDIR)/runtime/opt_amd64_sysv.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_SRCDIR)/runtime/opt_amd64_sysv.s $(BIN_HEADERS_SRCDIR)/runtime/common_amd64.inc.s --zero
$(BIN_HEADERS_DESTDIR)/runtime/amd64_sysv.binary.h: $(BIN_HEADERS_SRCDIR)/runtime/amd64_sysv.s $(BIN_HEADERS_SRCDIR)/runtime/common_amd64.inc.s
$(BIN_HEADERS_DESTDIR)/runtime/amd64_sysv.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_SRCDIR)/runtime/amd64_sysv.s $(BIN_HEADERS_SRCDIR)/runtime/common_amd64.inc.s --zero
$(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h: $(BIN_HEADERS_SRCDIR)/compiler/predefined_defs.h
$(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_SRCDIR)/compiler/predefined_defs.h --zero
$(BIN_HEADERS_DESTDIR)/driver/help.binary.h: $(GENERATED_HELP_DIR)/kefir.1.txt
$(BIN_HEADERS_DESTDIR)/driver/help.binary.h: BINARY_HEADER_CONTENT=$(GENERATED_HELP_DIR)/kefir.1.txt --zero

$(BIN_HEADERS_DESTDIR)/compiler/profile.deps: $(BIN_HEADERS_DESTDIR)/runtime/opt_amd64_sysv.binary.h $(BIN_HEADERS_DESTDIR)/runtime/amd64_sysv.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@")/../runtime -DKEFIR_OPT_AMD64_SYSV_RUNTIME_INCLUDE=opt_amd64_sysv.binary.h -DKEFIR_AMD64_SYSV_RUNTIME_INCLUDE=amd64_sysv.binary.h' > $@

$(BIN_HEADERS_DESTDIR)/compiler/compiler.deps: $(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo "-I$(shell dirname "$@") -DKEFIR_COMPILER_PREDEFINED_DEFS_INCLUDE=predefined_defs.binary.h" > $@
	
$(BIN_HEADERS_DESTDIR)/driver/main.deps: $(BIN_HEADERS_DESTDIR)/driver/help.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@") -DKEFIR_DRIVER_HELP_INCLUDE=help.binary.h' > $@

$(BIN_HEADERS_DESTDIR)/web/main.deps: $(BIN_HEADERS_DESTDIR)/driver/help.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@")/../driver -DKEFIR_DRIVER_HELP_INCLUDE=help.binary.h' > $@