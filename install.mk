DESTDIR=
prefix=/opt/kefir
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
libdir=$(exec_prefix)/lib
datarootdir=$(prefix)/share
mandir=$(datarootdir)/man
man1dir=$(mandir)/man1
sysconfdir=$(prefix)/etc
includedir=$(prefix)/include

INSTALL_LICENSES=yes

install:
	@echo "Creating directories..."
	@install -d "$(DESTDIR)$(includedir)"/kefir/toolchain
	@install -d "$(DESTDIR)$(libdir)"
	@install -d "$(DESTDIR)$(bindir)"
	@install -d "$(DESTDIR)$(sysconfdir)"
	@install -d "$(DESTDIR)$(man1dir)"
	@echo "Installing libraries.."
ifneq ($(wildcard $(LIBKEFIR_SO).$(LIBKEFIR_SO_VERSION)),)
	@install "$(LIBKEFIR_SO).$(LIBKEFIR_SO_VERSION)" "$(DESTDIR)$(libdir)"
	@ln -sf libkefir.so.$(LIBKEFIR_SO_VERSION) "$(DESTDIR)$(libdir)"/libkefir.so
endif
	@install "$(LIBKEFIR_A)" "$(DESTDIR)$(libdir)"
	@echo "Installing headers..."
	@cp -r -P -p "$(HEADERS_DIR)"/kefir "$(DESTDIR)$(includedir)"/kefir/toolchain
	@ln -sfn toolchain/kefir/runtime "$(DESTDIR)$(includedir)"/kefir/runtime
	@echo "Installing binaries..."
	@install "$(KEFIR_BIN_DIR)"/kefir "$(DESTDIR)$(bindir)"/kefir-cc
	@install "$(KEFIR_BIN_DIR)"/kefir-cc1 "$(DESTDIR)$(bindir)"/kefir-cc1
	@m4 -Dbindir="$$($(REALPATH) --relative-to=$(DESTDIR)$(bindir) $(DESTDIR)$(bindir))" \
	    -Dlibdir="$$($(REALPATH) --relative-to=$(DESTDIR)$(bindir) $(DESTDIR)$(libdir))" \
		-Dsysconfdir="$$($(REALPATH) --relative-to=$(DESTDIR)$(bindir) $(DESTDIR)$(sysconfdir))" \
		-Dincludedir="$$($(REALPATH) --relative-to=$(DESTDIR)$(bindir) $(DESTDIR)$(includedir))" \
		"$(SCRIPTS_DIR)"/kefir.m4 > "$(DESTDIR)$(bindir)"/kefir
	@chmod 755 "$(DESTDIR)$(bindir)"/kefir
	@install "$(SCRIPTS_DIR)"/detect-host-env.sh "$(DESTDIR)$(bindir)"/kefir-detect-host-env
	@echo "Installing man pages..."
	@install "$(KEFIR_BIN_DIR)"/man/kefir.1.gz "$(DESTDIR)$(man1dir)"
	@install "$(KEFIR_BIN_DIR)"/man/kefir-cc1.1.gz "$(DESTDIR)$(man1dir)"
	@install "$(KEFIR_BIN_DIR)"/man/kefir-detect-host-env.1.gz "$(DESTDIR)$(man1dir)"
ifeq ($(INSTALL_LICENSES),yes)
	@echo "Installing license files..."
	@mkdir -p "$(DESTDIR)$(datarootdir)/licenses/kefir"
	@install $(ROOT)/COPYING "$(DESTDIR)$(datarootdir)/licenses/kefir/COPYING"
	@install $(ROOT)/dist/README.license "$(DESTDIR)$(datarootdir)/licenses/kefir/README"
endif
	@echo "Initializing local config..."
	@touch "$(DESTDIR)$(sysconfdir)"/kefir.local

uninstall:
	@echo "Removing local config..."
	@rm -rf "$(DESTDIR)$(sysconfdir)"/kefir.local
	@echo "Removing man pages..."
	@rm -rf "$(DESTDIR)$(man1dir)"/kefir.1.gz
	@rm -rf "$(DESTDIR)$(man1dir)"/kefir-cc1.1.gz
	@rm -rf "$(DESTDIR)$(man1dir)"/kefir-detect-host-env.1.gz
	@echo "Removing binaries..."
	@rm -rf "$(DESTDIR)$(bindir)"/kefir
	@rm -rf "$(DESTDIR)$(bindir)"/kefir-detect-host-env
	@rm -rf "$(DESTDIR)$(bindir)"/kefir-cc
	@rm -rf "$(DESTDIR)$(bindir)"/kefir-cc1
	@echo "Removing headers..."
	@rm -rf "$(DESTDIR)$(includedir)"/kefir
	@echo "Removing libraries..."
	@rm -rf "$(DESTDIR)$(libdir)"/libkefir.so
	@rm -rf "$(DESTDIR)$(libdir)"/libkefir.so.$(LIBKEFIR_SO_VERSION)
	@rm -rf "$(DESTDIR)$(libdir)"/libkefir.a

.PHONY: install uninstall
