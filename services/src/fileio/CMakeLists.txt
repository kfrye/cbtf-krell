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

set(SERVICES_FILEIO_SOURCES
        SendToFile.c
	send.c
)

include_directories(
	${Libtirpc_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/services/include
	${PROJECT_SOURCE_DIR}/services/src/data
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/events
	${CMAKE_CURRENT_BINARY_DIR}/../../../messages/src/perfdata
)

add_library(cbtf-services-fileio SHARED
	${SERVICES_FILEIO_SOURCES}
)

target_link_libraries(cbtf-services-fileio
        -Wl,--no-as-needed
	${CMAKE_DL_LIBS}
    )

set_target_properties(cbtf-services-fileio PROPERTIES VERSION 1.1.0)
set_target_properties(cbtf-services-fileio PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES}")
set_target_properties(cbtf-services-fileio PROPERTIES POSITION_INDEPENDENT_CODE ON)


install(TARGETS cbtf-services-fileio
	LIBRARY DESTINATION lib${LIB_SUFFIX}
)

# Install static version

add_library(cbtf-services-fileio-static STATIC
	${SERVICES_FILEIO_SOURCES}
)

target_link_libraries(cbtf-services-fileio-static
        -Wl,--no-as-needed
	${CMAKE_DL_LIBS}
    )

set_target_properties(cbtf-services-fileio-static PROPERTIES VERSION 1.1.0)
set_target_properties(cbtf-services-fileio-static PROPERTIES OUTPUT_NAME cbtf-services-fileio)
set_target_properties(cbtf-services-fileio-static PROPERTIES
        COMPILE_DEFINITIONS "${TLS_DEFINES}")
set_target_properties(cbtf-services-fileio-static PROPERTIES POSITION_INDEPENDENT_CODE ON)

install(TARGETS cbtf-services-fileio-static
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
)
