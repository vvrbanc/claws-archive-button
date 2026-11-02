# Copyright 1999-2015 the Claws Mail team.
# This file is part of Claws Mail package, and distributed under the
# terms of the General Public License version 3 (or later).
# See COPYING file for license details.

include $(srcdir)/../win_plugin.mk

IFLAGS = \
	-I$(top_builddir)/src \
	-I$(top_builddir)/src/common \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/gtk

plugindir = $(pkglibdir)/plugins

if BUILD_ARCHIVE_BUTTON_PLUGIN
plugin_LTLIBRARIES = archive_button.la
endif

archive_button_la_SOURCES = \
	archive_button.c archive_button.h \
	archive_button_prefs.c archive_button_prefs.h

archive_button_la_LDFLAGS = \
	$(plugin_ldflags) \
	-avoid-version -module -z noexecstack

EXTRA_archive_button_la_DEPENDENCIES = $(plugin_extra_deps)

archive_button_la_LIBADD = $(plugin_libadd) \
	$(GTK_LIBS)

archive_button_la_CPPFLAGS = \
	$(IFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(ENCHANT_CFLAGS)

.PHONY: test
