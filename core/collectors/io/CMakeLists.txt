################################################################################
# Copyright (c) 2014-2019 Krell Institute. All Rights Reserved.
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

include(CheckIncludeFile)

# FIXME:  dpm comments about changes in this area
# Step wise refinement. I really think refactoring the build is going 
# to be required to handle this more cleanly.  ie. the top level CMakeLists.txt 
# files may likely be more complex, but lower level CMakeLists.txt files should really
# be as clean as possible.  For example, the collectors currently build the xml files. 
# As it turns out we have a build case where we want to separate those out based on some 
# build option. So now we have all this if else logic.
# The better solution is to move the building of the xml to a separate directory 
# from the collector C code.  eg.
# core/collectors/pcsamp/CMakeLists.txt    
#              This one decides to build both src and xml, or just the src or just the xml.
# core/collectors/pcsamp/src/CMakeLists.txt  
#              This has just the collector runtime source.
# core/collectors/pcsamp/xml/CMakeLists.txt  
#              This one just builds the xml from the template or uses it's own specific xml 
#              rules or xml file. (mem is an example soon). 

if (BUILD_FE_USING_CN_RUNTIMES)
    # This is the BUILD_FE_USING_CN_RUNTIMES section where we create the xml that includes both the traditional
    # frontend build based components for the FE node and the targeted compute node components for
    # the targeted commnode.
    if (CBTF_FILTERS_RUN_ON MATCHES "host")
       set(cn_component_location ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/KrellInstitute/Components)
    else()
       set(cn_component_location ${CBTF_KRELL_CN_RUNTIME_DIR}/lib${LIB_SUFFIX}/KrellInstitute/Components)
    endif()

    set(fe_component_location ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/KrellInstitute/Components)

    set(cbtflib_location ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
    set(aggregation_plugin "AggregationPlugin.so")
    set(aggregation_type "AddressAggregator")

    # Create and install xml for the experiment into the target cbtf-krell based location
    set(collector_name "io")
    configure_file(../template.xml.in io.xml @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/io.xml
        DESTINATION ${CBTF_KRELL_CN_RUNTIME_DIR}/share/KrellInstitute/xml)

    # Create and install xml for the experiment into the target cbtf-krell based location
    set(collector_name "iot")
    configure_file(../template.xml.in iot.xml @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/iot.xml
        DESTINATION ${CBTF_KRELL_CN_RUNTIME_DIR}/share/KrellInstitute/xml)

    # Create and install xml for the experiment into the target cbtf-krell based location
    set(collector_name "iop")
    configure_file(../template.xml.in iop.xml @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/iop.xml
        DESTINATION ${CBTF_KRELL_CN_RUNTIME_DIR}/share/KrellInstitute/xml)
else ()

if (LIBMONITOR_FOUND)

set(WRAPPER_SOURCES
	wrappers.c 
)

set(COLLECTOR_SOURCES
	collector.c
)

add_library(io-collector-monitor-fileio MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(io-collector-monitor-fileio PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(io-collector-monitor-fileio
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-fileio.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/fileio/libcbtf-services-fileio.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	-Wl,--no-whole-archive
)

set_target_properties(io-collector-monitor-fileio PROPERTIES PREFIX "")
set_target_properties(io-collector-monitor-fileio PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_FILEIO;__USE_GNU")

install(TARGETS io-collector-monitor-fileio
        LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(io-monitor-fileio STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(io-monitor-fileio PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(io-monitor-fileio
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-fileio-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(io-monitor-fileio PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_FILEIO;CBTF_SERVICE_BUILD_STATIC;__USE_GNU")

install(TARGETS io-monitor-fileio
        ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)
#
# iot experiment libraries 
#
add_library(iot-collector-monitor-fileio MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iot-collector-monitor-fileio PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iot-collector-monitor-fileio
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-fileio.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/fileio/libcbtf-services-fileio.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	-Wl,--no-whole-archive
)

set_target_properties(iot-collector-monitor-fileio PROPERTIES PREFIX "")
set_target_properties(iot-collector-monitor-fileio PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_FILEIO;EXTENDEDTRACE;__USE_GNU")

install(TARGETS iot-collector-monitor-fileio
        LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(iot-monitor-fileio STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iot-monitor-fileio PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iot-monitor-fileio
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-fileio-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(iot-monitor-fileio PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_FILEIO;CBTF_SERVICE_BUILD_STATIC;EXTENDEDTRACE;__USE_GNU")

install(TARGETS iot-monitor-fileio
        ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)
#
# iop experiment libraries 
#
add_library(iop-collector-monitor-fileio MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iop-collector-monitor-fileio PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iop-collector-monitor-fileio
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-fileio.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/fileio/libcbtf-services-fileio.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	-Wl,--no-whole-archive
)

set_target_properties(iop-collector-monitor-fileio PROPERTIES PREFIX "")
set_target_properties(iop-collector-monitor-fileio PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_FILEIO;PROFILE;__USE_GNU")

install(TARGETS iop-collector-monitor-fileio
        LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(iop-monitor-fileio STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iop-monitor-fileio PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iop-monitor-fileio
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-fileio-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(iop-monitor-fileio PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_FILEIO;CBTF_SERVICE_BUILD_STATIC;PROFILE;__USE_GNU")

install(TARGETS iop-monitor-fileio
        ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)

if (MRNET_FOUND)
#
# io experiment libraries with MRNET but not MPI
#
add_library(io-collector-monitor-mrnet MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(io-collector-monitor-mrnet PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(io-collector-monitor-mrnet
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	${MRNet_LWR_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/mrnet/libcbtf-services-mrnet.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-mrnet.a
	-Wl,--no-whole-archive
)

set_target_properties(io-collector-monitor-mrnet PROPERTIES PREFIX "")
set_target_properties(io-collector-monitor-mrnet PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS io-collector-monitor-mrnet
    LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(io-monitor-mrnet STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(io-monitor-mrnet PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(io-monitor-mrnet
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-data-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-mrnet-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	${MRNet_LWR_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(io-monitor-mrnet PROPERTIES VERSION 1.1.0)
set_target_properties(io-monitor-mrnet PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_BUILD_STATIC;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS io-monitor-mrnet
	ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)

#
# io experiment libraries with MRNET and MPI
#
add_library(io-collector-monitor-mrnet-mpi MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(io-collector-monitor-mrnet-mpi PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(io-collector-monitor-mrnet-mpi
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	${MRNet_LWR_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/mrnet/libcbtf-services-mrnet.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-mrnet-mpi.a
	-Wl,--no-whole-archive
)

set_target_properties(io-collector-monitor-mrnet-mpi PROPERTIES PREFIX "")
set_target_properties(io-collector-monitor-mrnet-mpi PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_USE_MRNET_MPI;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS io-collector-monitor-mrnet-mpi
    LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(io-monitor-mrnet-mpi STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(io-monitor-mrnet-mpi PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(io-monitor-mrnet-mpi
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-data-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-mrnet-mpi-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	${MRNet_LWR_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(io-monitor-mrnet-mpi PROPERTIES VERSION 1.1.0)
set_target_properties(io-monitor-mrnet-mpi PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_USE_MRNET_MPI;CBTF_SERVICE_BUILD_STATIC;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS io-monitor-mrnet-mpi
    ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)
#
# iop experiment libraries with MRNET but not MPI
#
add_library(iop-collector-monitor-mrnet MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iop-collector-monitor-mrnet PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iop-collector-monitor-mrnet
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	${MRNet_LWR_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/mrnet/libcbtf-services-mrnet.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-mrnet.a
	-Wl,--no-whole-archive
)

set_target_properties(iop-collector-monitor-mrnet PROPERTIES PREFIX "")
set_target_properties(iop-collector-monitor-mrnet PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;PROFILE;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS iop-collector-monitor-mrnet
	LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(iop-monitor-mrnet STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iop-monitor-mrnet PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iop-monitor-mrnet
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-data-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-mrnet-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	${MRNet_LWR_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(iop-monitor-mrnet PROPERTIES VERSION 1.1.0)
set_target_properties(iop-monitor-mrnet PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_BUILD_STATIC;PROFILE;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS iop-monitor-mrnet
	ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)
#
# iop experiment libraries with MRNET and MPI
#
add_library(iop-collector-monitor-mrnet-mpi MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iop-collector-monitor-mrnet-mpi PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iop-collector-monitor-mrnet-mpi
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	${MRNet_LWR_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/mrnet/libcbtf-services-mrnet.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-mrnet-mpi.a
	-Wl,--no-whole-archive
)

set_target_properties(iop-collector-monitor-mrnet-mpi PROPERTIES PREFIX "")
set_target_properties(iop-collector-monitor-mrnet-mpi PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_USE_MRNET_MPI;PROFILE;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS iop-collector-monitor-mrnet-mpi
    LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(iop-monitor-mrnet-mpi STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iop-monitor-mrnet-mpi PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iop-monitor-mrnet-mpi
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-data-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-mrnet-mpi-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	${MRNet_LWR_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(iop-monitor-mrnet-mpi PROPERTIES VERSION 1.1.0)
set_target_properties(iop-monitor-mrnet-mpi PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_USE_MRNET_MPI;CBTF_SERVICE_BUILD_STATIC;PROFILE;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS iop-monitor-mrnet-mpi
	ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)
#
# iot experiment libraries with MRNET but not MPI
#
add_library(iot-collector-monitor-mrnet MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iot-collector-monitor-mrnet PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iot-collector-monitor-mrnet
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	${MRNet_LWR_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/mrnet/libcbtf-services-mrnet.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-mrnet.a
	-Wl,--no-whole-archive
)

set_target_properties(iot-collector-monitor-mrnet PROPERTIES PREFIX "")
set_target_properties(iot-collector-monitor-mrnet PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;EXTENDEDTRACE;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS iot-collector-monitor-mrnet
    LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(iot-monitor-mrnet STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iot-monitor-mrnet PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iot-monitor-mrnet
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-data-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-mrnet-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	${MRNet_LWR_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(iot-monitor-mrnet PROPERTIES VERSION 1.1.0)
set_target_properties(iot-monitor-mrnet PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_BUILD_STATIC;EXTENDEDTRACE;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS iot-monitor-mrnet
	ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)
#
# iot experiment libraries with MRNET and MPI
#
add_library(iot-collector-monitor-mrnet-mpi MODULE
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iot-collector-monitor-mrnet-mpi PUBLIC
        ${Libtirpc_INCLUDE_DIRS}
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iot-collector-monitor-mrnet-mpi
        -Wl,--no-as-needed
	cbtf-services-monitor
	${LibMonitor_SHARED_LIBRARIES}
	${LibUnwind_SHARED_LIBRARIES}
	${MRNet_LWR_SHARED_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
	-Wl,--whole-archive
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata/libcbtf-messages-perfdata.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events/libcbtf-messages-events.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base/libcbtf-messages-base.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/mrnet/libcbtf-services-mrnet.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/common/libcbtf-services-common.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/offline/libcbtf-services-offline.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/unwind/libcbtf-services-unwind.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/data/libcbtf-services-data.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/src/xdr/libcbtf-services-send.a
	${CMAKE_CURRENT_BINARY_DIR}/../../../services/collector/libcbtf-services-collector-monitor-mrnet-mpi.a
	-Wl,--no-whole-archive
)

set_target_properties(iot-collector-monitor-mrnet-mpi PROPERTIES PREFIX "")
set_target_properties(iot-collector-monitor-mrnet-mpi PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_USE_MRNET_MPI;EXTENDEDTRACE;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS iot-collector-monitor-mrnet-mpi
	LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors
)

# Create and install static versions

add_library(iot-monitor-mrnet-mpi STATIC
	${COLLECTOR_SOURCES}
	${WRAPPER_SOURCES}
)

target_include_directories(iot-monitor-mrnet-mpi PUBLIC
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/messages/include
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/collector
	${MRNet_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/base
)

target_link_libraries(iot-monitor-mrnet-mpi
        -Wl,--no-as-needed
	cbtf-services-common-static
	cbtf-services-monitor-static
	cbtf-services-offline-static
	cbtf-services-data-static
	cbtf-services-unwind-static
	cbtf-services-collector-monitor-mrnet-mpi-static
	${LibMonitor_STATIC_LIBRARIES}
	${LibUnwind_STATIC_LIBRARIES}
	${MRNet_LWR_STATIC_LIBRARIES}
	pthread
	rt
	${CMAKE_DL_LIBS}
)

set_target_properties(iot-monitor-mrnet-mpi PROPERTIES VERSION 1.1.0)
set_target_properties(iot-monitor-mrnet-mpi PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};CBTF_SERVICE_USE_OFFLINE;CBTF_SERVICE_USE_MRNET;CBTF_SERVICE_USE_MRNET_MPI;CBTF_SERVICE_BUILD_STATIC;EXTENDEDTRACE;__USE_GNU;${MRNet_DEFINES}")

install(TARGETS iot-monitor-mrnet-mpi
	ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/Collectors/static
)

endif() # if (MRNET_FOUND)

endif() # if (LIBMONITOR_FOUND)

if (NOT RUNTIME_ONLY)
    # If we are building the targeted runtimes/collectors, then don't build the xml
    # We will do that as part of the targeted build of the FE (BUILD_FE_USING_CN_RUNTIMES) 
    # See the top of the file for the BUILD_FE_USING_CN_RUNTIMES section of
    # code where we build the xml for the experiment.
    #
    # Immediately below is the normal cluster - non-targeted build of the xml file.
    #
    set(fe_component_location ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/KrellInstitute/Components)
    set(cn_component_location ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/KrellInstitute/Components)
    set(cbtflib_location ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
    set(aggregation_plugin "AggregationPlugin.so")
    set(aggregation_type "AddressAggregator")

    # Create and install xml for the experiment
    set(collector_name "io")
    configure_file(../template.xml.in io.xml @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/io.xml
        DESTINATION share/KrellInstitute/xml)

    # Create and install xml for the experiment
    set(collector_name "iot")
    configure_file(../template.xml.in iot.xml @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/iot.xml
        DESTINATION share/KrellInstitute/xml)

    # Create and install xml for the experiment
    set(collector_name "iop")
    configure_file(../template.xml.in iop.xml @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/iop.xml
        DESTINATION share/KrellInstitute/xml)
endif()

endif() # end of non BUILD_FE_USING_CN_RUNTIMES section of code
