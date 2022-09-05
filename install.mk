DESTDIR=/opt/kefir

install:
	@echo "Creating directories..."
	@install -d "$(DESTDIR)"/include/kefir/toolchain
	@install -d "$(DESTDIR)"/lib
	@install -d "$(DESTDIR)"/bin
	@echo "Installing libraries.."
ifneq ($(wildcard $(LIBKEFIR_SO).$(LIBKEFIR_SO_VERSION)),)
	@install -D "$(LIBKEFIR_SO).$(LIBKEFIR_SO_VERSION)" -t "$(DESTDIR)"/lib
	@ln -sf libkefir.so.$(LIBKEFIR_SO_VERSION) "$(DESTDIR)"/lib/libkefir.so
endif
	@install -D "$(LIBKEFIR_A)" -t "$(DESTDIR)"/lib
	@install -D "$(LIBKEFIRRT_A)" -t "$(DESTDIR)"/lib
	@echo "Installing headers..."
	@cp -r --no-dereference -p "$(HEADERS_DIR)"/kefir "$(DESTDIR)"/include/kefir/toolchain
	@ln -sf toolchain/kefir/runtime "$(DESTDIR)"/include/kefir/runtime
	@echo "Installing binaries..."
	@install "$(BIN_DIR)"/kefir "$(DESTDIR)"/bin/kefir-cc
	@install "$(BIN_DIR)"/kefir-cc1 "$(DESTDIR)"/bin/kefir-cc1
	@install "$(SCRIPTS_DIR)"/kefir.sh "$(DESTDIR)"/bin/kefir

uninstall:
	@echo "Removing binaries..."
	@rm -rf "$(DESTDIR)"/bin/kefir "$(DESTDIR)"/bin/kefir-cc "$(DESTDIR)"/bin/kefir-cc1
	@echo "Removing headers..."
	@rm -rf "$(DESTDIR)"/include/kefir
	@echo "Removing libraries..."
	@rm -rf "$(DESTDIR)"/lib/libkefir.so
	@rm -rf "$(DESTDIR)"/lib/libkefir.so.$(LIBKEFIR_SO_VERSION)
	@rm -rf "$(DESTDIR)"/lib/libkefirrt.a
	@rm -rf "$(DESTDIR)"/lib/libkefir.a

.PHONY: install
