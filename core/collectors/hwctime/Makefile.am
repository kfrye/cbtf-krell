##############################################################################
# Copyright (c) 2010-2012 Krell Institute All Rights Reserved.
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

CFLAGS = -g -O0

xmldir = $(datadir)/KrellInstitute/xml
plugindir = $(libdir)/KrellInstitute/Components
pkglibdir = $(libdir)/KrellInstitute/Collectors
staticpkglibdir = $(libdir)/KrellInstitute/Collectors/static

BUILT_FILES = hwctime.xml

nobase_dist_xml_DATA = \
        hwctime.xml


pkglib_LTLIBRARIES =
staticpkglib_LTLIBRARIES =

if !BUILD_TARGETED

if HAVE_LIBMONITOR
pkglib_LTLIBRARIES += hwctime-collector-monitor-fileio.la
staticpkglib_LTLIBRARIES += libhwctime-monitor-fileio.la

hwctime_collector_monitor_fileio_la_CFLAGS = \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_FILEIO \
	-DUSE_FASTTRACE \
	@CBTF_SERVICES_CPPFLAGS@ \
	@LIBUNWIND_CPPFLAGS@ \
	@PAPI_CPPFLAGS@ \
	@LIBMONITOR_CPPFLAGS@

hwctime_collector_monitor_fileio_la_LDFLAGS = \
	@CBTF_SERVICES_LDFLAGS@ \
	@LIBMONITOR_LDFLAGS@ \
	@LIBUNWIND_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwctime_collector_monitor_fileio_la_LIBADD = \
	@LIBMONITOR_LIBS@ \
	@LIBUNWIND_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-fileio \
	-lpthread -lrt

hwctime_collector_monitor_fileio_la_SOURCES = \
	collector.c

libhwctime_monitor_fileio_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_FILEIO \
	-DUSE_FASTTRACE \
	@CBTF_SERVICES_CPPFLAGS@ \
	@LIBUNWIND_CPPFLAGS@ \
	@MESSAGES_CPPFLAGS@ \
	@PAPI_CPPFLAGS@ \
	@LIBMONITOR_CPPFLAGS@

libhwctime_monitor_fileio_la_LDFLAGS = \
	-static \
	@CBTF_SERVICES_LDFLAGS@ \
	@LIBMONITOR_LDFLAGS@ \
	@LIBUNWIND_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_fileio_la_LIBADD = \
	@LIBMONITOR_LIBS@ \
	@LIBUNWIND_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-fileio \
	-lpthread -lrt

libhwctime_monitor_fileio_la_SOURCES = \
	collector.c

if HAVE_MRNET

pkglib_LTLIBRARIES += hwctime-collector-monitor-mrnet.la
staticpkglib_LTLIBRARIES += libhwctime-monitor-mrnet.la

hwctime_collector_monitor_mrnet_la_CFLAGS = \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DUSE_FASTTRACE \
	@CBTF_SERVICES_CPPFLAGS@ \
	@LIBMONITOR_CPPFLAGS@ \
	@LIBUNWIND_CPPFLAGS@ \
	@MESSAGES_CPPFLAGS@ \
	@PAPI_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

hwctime_collector_monitor_mrnet_la_LDFLAGS = \
	@CBTF_SERVICES_LDFLAGS@ \
	@LIBMONITOR_LDFLAGS@ \
	@LIBUNWIND_LDFLAGS@ \
	@MRNET_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwctime_collector_monitor_mrnet_la_LIBADD = \
	@LIBMONITOR_LIBS@ \
	@LIBUNWIND_LIBS@ \
	@MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-mrnet \
	-lpthread -lrt

hwctime_collector_monitor_mrnet_la_SOURCES = \
	collector.c

libhwctime_monitor_mrnet_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DUSE_FASTTRACE \
	@CBTF_SERVICES_CPPFLAGS@ \
	@LIBMONITOR_CPPFLAGS@ \
	@LIBUNWIND_CPPFLAGS@ \
	@MESSAGES_CPPFLAGS@ \
	@PAPI_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

libhwctime_monitor_mrnet_la_LDFLAGS = \
	-static \
	@CBTF_SERVICES_LDFLAGS@ \
	@LIBMONITOR_LDFLAGS@ \
	@LIBUNWIND_LDFLAGS@ \
	@MRNET_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_mrnet_la_LIBADD = \
	@LIBMONITOR_LIBS@ \
	@LIBUNWIND_LIBS@ \
	@MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-mrnet \
	-lpthread -lrt

libhwctime_monitor_mrnet_la_SOURCES = \
	collector.c


pkglib_LTLIBRARIES += hwctime-collector-monitor-mrnet-mpi.la
staticpkglib_LTLIBRARIES += libhwctime-monitor-mrnet-mpi.la

hwctime_collector_monitor_mrnet_mpi_la_CFLAGS = \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DCBTF_SERVICE_USE_MRNET_MPI \
	-DUSE_FASTTRACE \
	@CBTF_SERVICES_CPPFLAGS@ \
	@LIBMONITOR_CPPFLAGS@ \
	@LIBUNWIND_CPPFLAGS@ \
	@MESSAGES_CPPFLAGS@ \
	@PAPI_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

hwctime_collector_monitor_mrnet_mpi_la_LDFLAGS = \
	@CBTF_SERVICES_LDFLAGS@ \
	@LIBMONITOR_LDFLAGS@ \
	@LIBUNWIND_LDFLAGS@ \
	@MRNET_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwctime_collector_monitor_mrnet_mpi_la_LIBADD = \
	@LIBMONITOR_LIBS@ \
	@LIBUNWIND_LIBS@ \
	@MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-data \
	-lcbtf-services-collector-monitor-mrnet-mpi \
	-lpthread -lrt

hwctime_collector_monitor_mrnet_mpi_la_SOURCES = \
	collector.c

libhwctime_monitor_mrnet_mpi_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DCBTF_SERVICE_USE_MRNET_MPI \
	-DUSE_FASTTRACE \
	@CBTF_SERVICES_CPPFLAGS@ \
	@LIBMONITOR_CPPFLAGS@ \
	@LIBUNWIND_CPPFLAGS@ \
	@MESSAGES_CPPFLAGS@ \
	@PAPI_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

libhwctime_monitor_mrnet_mpi_la_LDFLAGS = \
	-static \
	@CBTF_SERVICES_LDFLAGS@ \
	@LIBMONITOR_LDFLAGS@ \
	@LIBUNWIND_LDFLAGS@ \
	@MRNET_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_mrnet_mpi_la_LIBADD = \
	@LIBMONITOR_LIBS@ \
	@LIBUNWIND_LIBS@ \
	@MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-mrnet-mpi \
	-lpthread -lrt

libhwctime_monitor_mrnet_mpi_la_SOURCES = \
	collector.c

# endif for HAVE_MRNET
endif
# endif for HAVE_LIBMONITOR
endif

# endif for if not build targeted (below)
endif

if HAVE_TARGET_LIBMONITOR
if HAVE_TARGET_SHARED
pkglib_LTLIBRARIES += hwctime-collector-monitor-fileio.la
endif
staticpkglib_LTLIBRARIES += libhwctime-monitor-fileio.la

if HAVE_TARGET_SHARED
hwctime_collector_monitor_fileio_la_CFLAGS = \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_FILEIO \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@

hwctime_collector_monitor_fileio_la_LDFLAGS = \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwctime_collector_monitor_fileio_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lpthread -lrt

hwctime_collector_monitor_fileio_la_SOURCES = \
	collector.c

libhwctime_monitor_fileio_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_FILEIO \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_MESSAGES_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@

libhwctime_monitor_fileio_la_LDFLAGS = \
	-static \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_fileio_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-fileio \
	-lpthread -lrt

libhwctime_monitor_fileio_la_SOURCES = \
	collector.c
else

if HAVE_TARGET_STATIC
libhwctime_monitor_fileio_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_FILEIO \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_MESSAGES_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@

libhwctime_monitor_fileio_la_LDFLAGS = \
	-static \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_fileio_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-fileio \
	-lpthread -lrt

libhwctime_monitor_fileio_la_SOURCES = \
	collector.c
# endif TARGET_STATIC
endif

# endif for else for not TARGET_SHARED
endif

if HAVE_TARGET_MRNET
if HAVE_TARGET_SHARED
pkglib_LTLIBRARIES += hwctime-collector-monitor-mrnet.la
endif
staticpkglib_LTLIBRARIES += libhwctime-monitor-mrnet.la

if HAVE_TARGET_SHARED
hwctime_collector_monitor_mrnet_la_CFLAGS = \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_MESSAGES_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_MRNET_CPPFLAGS@

hwctime_collector_monitor_mrnet_la_LDFLAGS = \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	@TARGET_MRNET_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwctime_collector_monitor_mrnet_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	@TARGET_MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lpthread -lrt

hwctime_collector_monitor_mrnet_la_SOURCES = \
	collector.c

libhwctime_monitor_mrnet_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_MESSAGES_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_MRNET_CPPFLAGS@

libhwctime_monitor_mrnet_la_LDFLAGS = \
	-static \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	@TARGET_MRNET_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_mrnet_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	@TARGET_MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-mrnet \
	-lpthread -lrt

libhwctime_monitor_mrnet_la_SOURCES = \
	collector.c
else
# not target shared case (else clause)
if HAVE_TARGET_STATIC
libhwctime_monitor_mrnet_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_MESSAGES_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_MRNET_CPPFLAGS@

libhwctime_monitor_mrnet_la_LDFLAGS = \
	-static \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	@TARGET_MRNET_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_mrnet_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	@TARGET_MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-mrnet \
	-lpthread -lrt

libhwctime_monitor_mrnet_la_SOURCES = \
	collector.c
# endif TARGET_STATIC
endif
# endif for else for not TARGET_SHARED
endif


if HAVE_TARGET_SHARED
pkglib_LTLIBRARIES += hwctime-collector-monitor-mrnet-mpi.la
endif
staticpkglib_LTLIBRARIES += libhwctime-monitor-mrnet-mpi.la

if HAVE_TARGET_SHARED
hwctime_collector_monitor_mrnet_mpi_la_CFLAGS = \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DCBTF_SERVICE_USE_MRNET_MPI \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_MESSAGES_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_MRNET_CPPFLAGS@

hwctime_collector_monitor_mrnet_mpi_la_LDFLAGS = \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	@TARGET_MRNET_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwctime_collector_monitor_mrnet_mpi_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	@TARGET_MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-timer \
	-lcbtf-services-data \
	-lpthread -lrt

hwctime_collector_monitor_mrnet_mpi_la_SOURCES = \
	collector.c

libhwctime_monitor_mrnet_mpi_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DCBTF_SERVICE_USE_MRNET_MPI \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_MESSAGES_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_MRNET_CPPFLAGS@

libhwctime_monitor_mrnet_mpi_la_LDFLAGS = \
	-static \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	@TARGET_MRNET_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_mrnet_mpi_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	@TARGET_MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-mrnet-mpi \
	-lpthread -lrt

libhwctime_monitor_mrnet_mpi_la_SOURCES = \
	collector.c
else

if HAVE_TARGET_STATIC
libhwctime_monitor_mrnet_mpi_la_CFLAGS = \
	-static \
	-DCBTF_SERVICE_USE_OFFLINE \
	-DCBTF_SERVICE_USE_MRNET \
	-DCBTF_SERVICE_USE_MRNET_MPI \
	-DUSE_FASTTRACE \
	@TARGET_CBTF_SERVICES_CPPFLAGS@ \
	@TARGET_LIBMONITOR_CPPFLAGS@ \
	@TARGET_LIBUNWIND_CPPFLAGS@ \
	@TARGET_MESSAGES_CPPFLAGS@ \
	@TARGET_PAPI_CPPFLAGS@ \
	@TARGET_MRNET_CPPFLAGS@

libhwctime_monitor_mrnet_mpi_la_LDFLAGS = \
	-static \
	@TARGET_CBTF_SERVICES_LDFLAGS@ \
	@TARGET_LIBMONITOR_LDFLAGS@ \
	@TARGET_LIBUNWIND_LDFLAGS@ \
	@TARGET_MRNET_LDFLAGS@ \
	-no-undefined

libhwctime_monitor_mrnet_mpi_la_LIBADD = \
	@TARGET_LIBMONITOR_LIBS@ \
	@TARGET_LIBUNWIND_LIBS@ \
	@TARGET_MRNET_LWR_LIBS@ \
	-lcbtf-services-common \
	-lcbtf-services-monitor \
	-lcbtf-services-offline \
	-lcbtf-services-unwind \
	-lcbtf-services-timer \
	-lcbtf-services-collector-monitor-mrnet-mpi \
	-lpthread -lrt

libhwctime_monitor_mrnet_mpi_la_SOURCES = \
	collector.c
# endif TARGET_STATIC
endif
# endif for else for not TARGET_SHARED
endif

# endif for else for not TARGET_MRNET
endif
# endif for else for not TARGET_LIBMONITOR
endif


CLEANFILES = $(BUILT_FILES)

hwctime.xml: hwctime.xml.in

	@rm -f hwctime.xml
	@sed \
		-e 's,@@component_location@@,$(plugindir),' \
		-e 's,@@cbtflib_location@@,$(libdir),' \
		$(srcdir)/hwctime.xml.in > hwctime.xml

EXTRA_DIST = hwctime.xml.in