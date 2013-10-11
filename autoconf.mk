#!/usr/bin/make -f
#
# Copyright 2011 Sergey Kolotsey.
#
# This file is part of libexpression library.
#
# libexpression is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libexpression is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public Licenise
# along with libexpression. If not, see <http://www.gnu.org/licenses/>.
#
# ======================================================================
#
# This makefile allows to automatically
# create configure script


# Define extra header files that need to be removed when
# the package is *extra* cleaned
AUTO_HEADERS=libexpression-config.h.in libexpression-config.h.in~


# Read package version and name from external file.
# Also test if that file exists and contains valid data.
VERSION_FILE=acversion.m4
PACKAGE=$(shell cat $(VERSION_FILE) |grep ac_PACKAGE |sed 's/.*\[.*\[//' |sed 's/].*//')
VERSION=$(shell cat $(VERSION_FILE) |grep ac_VERSION |sed 's/.*\[.*\[//' |sed 's/].*//')
ifeq ($(PACKAGE),)
$(error Version file `$(VERSION_FILE)' does not exist or does not contain valid data)
endif
ifeq ($(VERSION),)
$(error Version file `$(VERSION_FILE)' does not exist or does not contain valid data)
endif


# Test if Autoconf is installed.
# If it is not installed, throw error and exit
ifneq (autoconf,$(shell which autoconf >/dev/null 2>/dev/null && echo autoconf ))
$(error GNU Autoconf and libtool must be installed in order to create configuration script for this package)
endif
targets=configure


# Test if Doxygen is installed.
# If Doxygen is installed, then generate documentation for the project
Doxyfile=Doxyfile
ifeq (doxygen,$(shell [ -e $(Doxyfile) ] && which doxygen >/dev/null 2>/dev/null && echo doxygen ))
targets+=html
else
$(warning Doxygen is not installed, documentation will not be processed)
endif


all: $(targets)

configure: configure.ac acinclude.m4 acversion.m4 autoconf.mk
	aclocal && autoheader && libtoolize --copy --install && autoconf -f && rm -Rf autom4te.cache

html:
	doxygen $(Doxyfile) >/dev/null

$(PACKAGE)-$(VERSION).tgz: $(PACKAGE)-$(VERSION) $(targets) cleanbuild
	cp -R `ls |grep -v -e '\.tgz$$' -e '\.tar\.gz$$' -e '$<'` $</ && tar -zc $< >$@ && rm -R $</

$(PACKAGE)-$(VERSION):
	mkdir -p $@

tgz: $(PACKAGE)-$(VERSION).tgz

package: $(PACKAGE)-$(VERSION).tgz

cleanbuild:
	if [ -e Makefile ]; then make distclean; else true; fi

clean: cleanbuild
	rm -rf autom4te.cache tools html && rm -f configure configure.scan autoscan.log aclocal.m4 $(AUTO_HEADERS) $(PACKAGE)-$(VERSION).tgz

.PHONY: all clean cleanbuild tgz package
