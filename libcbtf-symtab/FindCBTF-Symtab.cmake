################################################################################
# Copyright (c) 2012 Argo Navis Technologies. All Rights Reserved.
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

include(FindPackageHandleStandardArgs)

find_library(CBTF_SYMTAB_LIBRARY NAMES libcbtf-symtab.so
    HINTS $ENV{CBTF_ROOT} $ENV{CBTF_PREFIX}
    PATH_SUFFIXES lib lib64
    )

find_path(CBTF_SYMTAB_INCLUDE_DIR KrellInstitute/SymbolTable/LinkedObject.hpp
    HINTS $ENV{CBTF_ROOT} $ENV{CBTF_PREFIX}
    PATH_SUFFIXES include
    )

find_package_handle_standard_args(
    CBTF-Symtab DEFAULT_MSG
    CBTF_SYMTAB_LIBRARY CBTF_SYMTAB_INCLUDE_DIR
    )

set(CBTF_SYMTAB_LIBRARIES ${CBTF_SYMTAB_LIBRARY})
set(CBTF_SYMTAB_INCLUDE_DIRS ${CBTF_SYMTAB_INCLUDE_DIR})

mark_as_advanced(CBTF_SYMTAB_LIBRARY CBTF_SYMTAB_INCLUDE_DIR)
