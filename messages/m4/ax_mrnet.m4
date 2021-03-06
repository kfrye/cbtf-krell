################################################################################
# Copyright (c) 2010-2015 Krell Institute. All Rights Reserved.
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

AC_DEFUN([AX_MRNET], [

    foundMRNET=0

    AC_ARG_WITH(mrnet-version,
                AC_HELP_STRING([--with-mrnet-version=VERS],
                               [mrnet-version installation @<:@4.0.0@:>@]),
                mrnet_vers=$withval, mrnet_vers="4.0.0")

    AC_ARG_WITH(mrnet,
                AC_HELP_STRING([--with-mrnet=DIR],
                               [MRNet installation @<:@/usr@:>@]),
                mrnet_dir=$withval, mrnet_dir="/usr")


    AC_ARG_WITH([mrnet-libdir],
                AS_HELP_STRING([--with-mrnet-libdir=LIB_DIR],
                [Force given directory for mrnet libraries. Note that this will overwrite library path detection, so use this parameter only if default library detection fails and you know exactly where your mrnet libraries are located.]),
                [
                if test -d $withval
                then
                        ac_mrnet_lib_path="$withval"
                else
                        AC_MSG_ERROR(--with-mrnet-libdir expected directory name)
                fi ],
                [ac_mrnet_lib_path=""])

    if test "x$ac_mrnet_lib_path" == "x"; then
       MRNET_LDFLAGS="-L$mrnet_dir/$abi_libdir"
       MRNET_LIBDIR="$mrnet_dir/$abi_libdir"
    else
       MRNET_LDFLAGS="-L$ac_mrnet_lib_path"
       MRNET_LIBDIR="$ac_mrnet_lib_path"
    fi

    if test -f $mrnet_dir/$abi_libdir/mrnet-$mrnet_vers/include/mrnet_config.h && \
       test -f $mrnet_dir/$abi_libdir/xplat-$mrnet_vers/include/xplat_config.h ; then
       MRNET_CPPFLAGS="-I$mrnet_dir/include -I$mrnet_dir/$abi_libdir/mrnet-$mrnet_vers/include \
                       -I$mrnet_dir/$abi_libdir/xplat-$mrnet_vers/include  -Dos_linux"
    elif test -f $mrnet_dir/$alt_abi_libdir/mrnet-$mrnet_vers/include/mrnet_config.h && \
         test -f $mrnet_dir/$alt_abi_libdir/xplat-$mrnet_vers/include/xplat_config.h ; then
       MRNET_CPPFLAGS="-I$mrnet_dir/include -I$mrnet_dir/$alt_abi_libdir/mrnet-$mrnet_vers/include \
                       -I$mrnet_dir/$alt_abi_libdir/xplat-$mrnet_vers/include  -Dos_linux"
       MRNET_LDFLAGS="-L$mrnet_dir/$alt_abi_libdir"
       MRNET_LIBDIR="$mrnet_dir/$alt_abi_libdir"
    elif test -f $mrnet_dir/$abi_libdir/mrnet_config.h ; then
       MRNET_CPPFLAGS="-I$mrnet_dir/include -I$mrnet_dir/$abi_libdir -Dos_linux"
    elif test -f $mrnet_dir/$alt_abi_libdir/mrnet_config.h ; then
       MRNET_CPPFLAGS="-I$mrnet_dir/include -I$mrnet_dir/$alt_abi_libdir -Dos_linux"
       MRNET_LDFLAGS="-L$mrnet_dir/$alt_abi_libdir"
       MRNET_LIBDIR="$mrnet_dir/$alt_abi_libdir"
    else
       MRNET_CPPFLAGS="-I$mrnet_dir/include -Dos_linux"
    fi

    MRNET_LIBS="-Wl,--whole-archive -lmrnet -lxplat -Wl,--no-whole-archive"
    MRNET_LIBS="$MRNET_LIBS -lpthread -ldl"
    MRNET_LW_LIBS="-Wl,--whole-archive -lmrnet_lightweight -lxplat_lightweight -Wl,--no-whole-archive"
    MRNET_LW_LIBS="$MRNET_LW_LIBS -lpthread -ldl"

    MRNET_LWR_LIBS="-Wl,--whole-archive -lmrnet_lightweight_r -lxplat_lightweight_r -Wl,--no-whole-archive"
    MRNET_LWR_LIBS="$MRNET_LWR_LIBS -lpthread -ldl"

    MRNET_DIR="$mrnet_dir"

    if [ test -z "$SYSROOT_DIR" ]; then
      # SYSROOT_DIR was not set
      if [ test -d /usr/lib/alps ] && [ test -f $MRNET_LIBDIR/libmrnet.so -o -f $MRNET_LIBDIR/libmrnet.a ]; then
        MRNET_LDFLAGS="$MRNET_LDFLAGS -L/usr/lib/alps"
        MRNET_LDFLAGS="$MRNET_LDFLAGS -L/usr/lib64"
        MRNET_LIBS="$MRNET_LIBS -lalps -lalpslli -lalpsutil"
        MRNET_LIBS="$MRNET_LIBS -lxmlrpc-epi"
      fi
    else
      if [ test -d $SYSROOT_DIR/usr/lib/alps ] && [ test -f $MRNET_LIBDIR/libmrnet.so -o -f $MRNET_LIBDIR/libmrnet.a ]; then
        MRNET_LDFLAGS="$MRNET_LDFLAGS -L$SYSROOT_DIR/usr/lib/alps"
        MRNET_LDFLAGS="$MRNET_LDFLAGS -L$SYSROOT_DIR/usr/lib64"
        MRNET_LIBS="$MRNET_LIBS -lalps -lalpslli -lalpsutil"
        MRNET_LIBS="$MRNET_LIBS -lxmlrpc-epi"
      fi
    fi

    AC_LANG_PUSH(C++)
    AC_REQUIRE_CPP

    mrnet_saved_CPPFLAGS=$CPPFLAGS
    mrnet_saved_LDFLAGS=$LDFLAGS
    mrnet_saved_LIBS=$LIBS

    CPPFLAGS="$CPPFLAGS $MRNET_CPPFLAGS $BOOST_CPPFLAGS"
    LDFLAGS="$CXXFLAGS $MRNET_LDFLAGS"
    LIBS="$MRNET_LIBS"

    AC_MSG_CHECKING([for MRNet library and headers])

    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
        #include <mrnet/MRNet.h>
        ]], [[
        MRN::set_OutputLevel(0);
        ]])], [ 
            AC_MSG_RESULT(yes)

	    foundMRNET=1

        ], [
            AC_MSG_RESULT(no)
            #AC_MSG_ERROR([MRNet could not be found.])

	    foundMRNET=0

        ])

    CPPFLAGS=$mrnet_saved_CPPFLAGS
    LDFLAGS=$mrnet_saved_LDFLAGS
    LIBS=$mrnet_saved_LIBS

    AC_LANG_POP(C++)

    AC_SUBST(MRNET_CPPFLAGS)
    AC_SUBST(MRNET_LDFLAGS)
    AC_SUBST(MRNET_LIBS)
    AC_SUBST(MRNET_LW_LIBS)
    AC_SUBST(MRNET_LWR_LIBS)
    AC_SUBST(MRNET_DIR)
    AC_SUBST(MRNET_LIBDIR)

    if test $foundMRNET == 1; then
        AM_CONDITIONAL(HAVE_MRNET, true)
        AC_DEFINE(HAVE_MRNET, 1, [Define to 1 if you have MRNet.])
    else
        AM_CONDITIONAL(HAVE_MRNET, false)
    fi

])

