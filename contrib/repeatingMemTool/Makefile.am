################################################################################
# Copyright (c) 2011-2015 Krell Institute. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

contribxmldir = $(datadir)/KrellInstitute/contrib/xml
contribbindir = $(datadir)/KrellInstitute/contrib/bin
contribplugindir = $(datadir)/KrellInstitute/contrib/lib

BUILT_FILES = repeatMem.xml

nobase_dist_contribxml_DATA = \
        repeatMem.xml

ACLOCAL_AMFLAGS = -I m4

contribbin_PROGRAMS = repeatMemtool
repeatMemtool_SOURCES = repeatMem_tool.cpp \
			Memory.cpp

repeatMemtool_CXXFLAGS = \
	-DBUILDDIR="\"$(abs_builddir)\"" \
	-DLIBDIR="\"$(libdir)\"" \
	-DTOPDIR="\"$(top_srcdir)\"" \
	-DXMLDIR="\"$(contribxmldir)\"" \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ @CBTF_XML_CPPFLAGS@ \
	@LIBXERCES_C_CPPFLAGS@ \
	@MRNET_CPPFLAGS@ 

repeatMemtool_LDFLAGS = \
	-L$(top_srcdir) \
	@BOOST_LDFLAGS@ \
	@CBTF_LDFLAGS@ @CBTF_XML_LDFLAGS@ \
	@BINUTILS_LDFLAGS@ \
	@LIBXERCES_C_LDFLAGS@ \
	@MRNET_LDFLAGS@ 

repeatMemtool_LDADD = \
	-lcbtf-xml \
	-lcbtf-mrnet \
	-lcbtf-core \
	@BOOST_PROGRAM_OPTIONS_LIB@ \
	@BOOST_FILESYSTEM_LIB@ \
	@BOOST_SYSTEM_LIB@ \
	@BOOST_THREAD_LIB@ \
	@CBTF_LIBS@ @CBTF_XML_LIBS@ \
	@BINUTILS_BFD_LIB@ \
	@LIBXERCES_C@ \
	@MRNET_LIBS@ 
  	
contribplugin_LTLIBRARIES = repeatMemPlugin.la mrnetPlugin.la statisticsPlugin.la

repeatMemPlugin_la_SOURCES = repeatMemPlugin.cpp \
			     Memory.cpp

repeatMemPlugin_la_CXXFLAGS = \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@

repeatMemPlugin_la_LDFLAGS = \
	-L$(top_srcdir) \
	-module -avoid-version -shared -rpath /ForceShared \
	@CBTF_LDFLAGS@

repeatMemPlugin_la_LIBADD = \
	-lcbtf-core \
	@CBTF_LIBS@

mrnetPlugin_la_SOURCES = mrnetPlugin.cpp \
			 Memory.cpp

mrnetPlugin_la_CXXFLAGS = \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

mrnetPlugin_la_LDFLAGS = \
        -L$(top_srcdir) \
	-module -avoid-version -shared -rpath /ForceShared \
        @CBTF_LDFLAGS@ \
	@MRNET_LDFLAGS@

mrnetPlugin_la_LIBADD = \
	-lcbtf-core \
	-lcbtf-mrnet \
	@CBTF_LIBS@ \
	@MRNET_LIBS@

statisticsPlugin_la_SOURCES = statisticsPlugin.cpp \
			 Memory.cpp

statisticsPlugin_la_CXXFLAGS = \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

statisticsPlugin_la_LDFLAGS = \
        -L$(top_srcdir) \
	-module -avoid-version -shared -rpath /ForceShared \
        @CBTF_LDFLAGS@ \
	@MRNET_LDFLAGS@

statisticsPlugin_la_LIBADD = \
	-lcbtf-core \
	-lcbtf-mrnet \
	@CBTF_LIBS@ \
	@MRNET_LIBS@

CLEANFILES = $(BUILT_FILES)

repeatMem.xml: repeatMem.xml.in

	@rm -f repeatMem.xml
	@sed \
                -e 's,@contriblib_location@,$(contribplugindir),' \
                -e 's,@cbtflib_location@,$(libdir),' \
                $(srcdir)/repeatMem.xml.in > repeatMem.xml

EXTRA_DIST = repeatMem.xml.in
