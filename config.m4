dnl $Id$
dnl config.m4 for extension swoole_chord

dnl  +----------------------------------------------------------------------+
dnl  | Swoole                                                               |
dnl  +----------------------------------------------------------------------+
dnl  | This source file is subject to version 2.0 of the Apache license,    |
dnl  | that is bundled with this package in the file LICENSE, and is        |
dnl  | available through the world-wide-web at the following url:           |
dnl  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
dnl  | If you did not receive a copy of the Apache2.0 license and are unable|
dnl  | to obtain it through the world-wide-web, please send a note to       |
dnl  | license@swoole.com so we can mail you a copy immediately.            |
dnl  +----------------------------------------------------------------------+
dnl  | Author: Tianfeng Han  <mikan.tenny@gmail.com>                        |
dnl  +----------------------------------------------------------------------+

PHP_ARG_ENABLE(swoole_chord, swoole_chord support,
[  --enable-swoole_chord           Enable swoole_chord support], [enable_swoole_chord="yes"])

PHP_ARG_ENABLE(asan, whether to enable asan,
[  --enable-asan             Enable asan], no, no)


PHP_ARG_WITH(libpq_dir, dir of libpq,
[  --with-libpq-dir[=DIR]      Include libpq support (requires libpq >= 9.5)], no, no)


if test "$PHP_swoole_chord" != "no"; then

    PHP_ADD_LIBRARY(pthread)
    PHP_SUBST(SWOOLE_CHORD_SHARED_LIBADD)

    CFLAGS="-Wall -pthread $CFLAGS"
    LDFLAGS="$LDFLAGS -lpthread"

    PHP_ADD_LIBRARY(pthread, 1, SWOOLE_CHORD_SHARED_LIBADD)

    swoole_source_file="swoole_chord_coro.cpp"

    PHP_NEW_EXTENSION(swoole_chord, $swoole_source_file, $ext_shared,,, cxx)

    PHP_ADD_INCLUDE([$ext_srcdir])
    PHP_ADD_INCLUDE([$ext_srcdir/include])
    PHP_ADD_INCLUDE([$phpincludedir/ext/swoole])
    PHP_ADD_INCLUDE([$phpincludedir/ext/swoole/include])
    
    PHP_ADD_EXTENSION_DEP(swoole_chord, swoole)

    PHP_REQUIRE_CXX()
    
    CXXFLAGS="$CXXFLAGS -Wall -Wno-unused-function -Wno-deprecated -Wno-deprecated-declarations -std=c++11"
fi
