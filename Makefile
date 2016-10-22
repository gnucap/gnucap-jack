GNUCAP_CONF = $(shell which gnucap-conf$(SUFFIX))
PACKAGE_NAME = gnucap-jack

include Make.override

ifneq ($(GNUCAP_CONF),)
    CXX = $(shell $(GNUCAP_CONF) --cxx)
    GNUCAP_CPPFLAGS = $(shell $(GNUCAP_CONF) --cppflags) -DADD_VERSION -DPIC
    GNUCAP_CXXFLAGS = $(shell $(GNUCAP_CONF) --cxxflags)
    GNUCAP_LDFLAGS = $(shell $(GNUCAP_CONF) --ldflags)
	 GNUCAP_LIBDIR   = $(shell $(GNUCAP_CONF) --libdir)
	 GNUCAP_PKGLIBDIR = $(shell $(GNUCAP_CONF) --pkglibdir)
	 GNUCAP_EXEC_PREFIX = $(shell $(GNUCAP_CONF) --exec-prefix)
#	 GNUCAP_PREFIX = $(shell $(GNUCAP_CONF) --prefix)
# TODO complete gnucap-conf
	 GNUCAP_PREFIX   = $(shell $(GNUCAP_CONF) --exec-prefix)# BUG, should be prefix!
	 GNUCAP_PKGLIBDIR = $(GNUCAP_LIBDIR)/gnucap
	 GNUCAP_DOCDIR = $(GNUCAP_PREFIX)/share/doc
else
    $(info no gnucap-conf, this will not work)
endif

GNUCAP_CXXFLAGS+= -fPIC -shared

PLUGIN_FILES = gnucap-jack.so

gnucap_jack_SOURCES = \
	f_gamut.cc \
	bm_jack.cc \
	bm_jack_wrap.cc \
	s_jack.cc

# is this a bug in upstream meter? use this for now.
gnucap_jack_SOURCES += d_meter.cc


CHECK_PLUGINS = ${gnucap_jack_SOURCES:%.cc=%.so}

CLEANFILES = ${EXEC_FILES} *.so *.o

all: $(EXEC_FILES) ${PLUGIN_FILES} ${CHECK_PLUGINS}

gnucap.mk${SUFFIX}: gnucap.mk.in
	sed -e s/@CXX@/g++/ \
	    -e s/@SUFFIX@/${SUFFIX}/ \
	    -e 's#@PREFIX@#${PREFFIX}#' \
	    -e 's#@GNUCAP_CPPFLAGS@#${GNUCAP_CPPFLAGS}#' \
	    -e 's#@GNUCAP_LDFLAGS@#${GNUCAP_LDFLAGS}#' \
	    -e 's#@GNUCAP_CXXFLAGS@#${GNUCAP_CXXFLAGS}#' \
	    -e 's#@WHICH_MAKE@#${WHICH_MAKE}#' \
	    < $< > $@
	chmod +x $@

LIBS=-ljack

%.so: %.cc
	$(CXX) $(CXXFLAGS) $(GNUCAP_CXXFLAGS) $(CPPFLAGS) $(GNUCAP_CPPFLAGS) -o $@ $< ${LIBS}

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(GNUCAP_CXXFLAGS) $(CPPFLAGS) $(GNUCAP_CPPFLAGS) -o $@ -c $<

check:
	:

gnucap_jack_OBJS = ${gnucap_jack_SOURCES:%.cc=%.o}
gnucap_jack_SOBJS = ${gnucap_jack_SOURCES:%.cc=%.so}

gnucap-jack.so: $(gnucap_jack_OBJS)
	$(CXX) -shared $+ -o $@ ${LIBS}

.PHONY: check

install: $(EXEC_FILES) ${PLUGIN_FILES}
	-install -d $(DESTDIR)/$(GNUCAP_EXEC_PREFIX)/bin
	-install -d $(DESTDIR)/$(GNUCAP_PKGLIBDIR)
	install $(EXEC_FILES) $(DESTDIR)/$(GNUCAP_EXEC_PREFIX)/bin
	install $(PLUGIN_FILES) $(DESTDIR)/$(GNUCAP_PKGLIBDIR)

	install -d $(DESTDIR)$(GNUCAP_DOCDIR)/$(PACKAGE_NAME)
	install README $(DESTDIR)$(GNUCAP_DOCDIR)/$(PACKAGE_NAME)

uninstall:
	(cd $(GNUCAP_EXEC_PREFIX)/bin; rm $(EXEC_FILES))

clean:
	rm -f $(CLEANFILES)

distclean: clean
	rm Make.override

Make.override:
	[ -e $@ ] || echo "# here you may override settings" > $@
