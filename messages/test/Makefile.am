################################################################################
# Copyright (c) 2011-2013 Krell Institute. All Rights Reserved.
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

noinst_PROGRAMS = testXDR

testXDR_CXXFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../../tools/messages/include \
	-I$(top_srcdir)/../../tools/messages/src/perfdata \
	-I$(top_srcdir)/../../tools/services/include \
	-I$(top_srcdir)/../../tools/core/include \
	-I$(top_srcdir)/../../framework/libcbtf-xml/include \
	-DBUILDDIR="\"$(abs_builddir)\"" \
	-DTOPDIR="\"$(top_srcdir)\"" \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

testXDR_LDFLAGS = \
	-L$(top_srcdir)/src \
	-L$(top_srcdir)/../../tools/messages/src/base/.libs \
	-L$(top_srcdir)/../../tools/core/src/.libs \
	-L$(top_srcdir)/../../framework/libcbtf-xml/src/.libs \
	@BOOST_LDFLAGS@ \
	@CBTF_LDFLAGS@ \
	@MRNET_LDFLAGS@


testXDR_LDADD = \
	-lcbtf-mrnet \
	-lcbtf-xml \
        -lcbtf-messages-base -lcbtf-messages-converters-base \
	-lcbtf-core \
	-lboost_unit_test_framework-mt \
	@CBTF_LIBS@ \
	@MRNET_LIBS@

testXDR_SOURCES = \
	testXDR.cpp
