dnl $Id$
dnl config.m4 for extension inprocess_store

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(inprocess_store, for inprocess_store support,
dnl Make sure that the comment is aligned:
dnl [  --with-inprocess_store             Include inprocess_store support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(inprocess_store, whether to enable inprocess_store support,
Make sure that the comment is aligned:
[  --enable-inprocess_store           Enable inprocess_store support])

if test "$PHP_INPROCESS_STORE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-inprocess_store -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/inprocess_store.h"  # you most likely want to change this
  dnl if test -r $PHP_INPROCESS_STORE/$SEARCH_FOR; then # path given as parameter
  dnl   INPROCESS_STORE_DIR=$PHP_INPROCESS_STORE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for inprocess_store files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       INPROCESS_STORE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$INPROCESS_STORE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the inprocess_store distribution])
  dnl fi

  dnl # --with-inprocess_store -> add include path
  dnl PHP_ADD_INCLUDE($INPROCESS_STORE_DIR/include)

  dnl # --with-inprocess_store -> check for lib and symbol presence
  dnl LIBNAME=inprocess_store # you may want to change this
  dnl LIBSYMBOL=inprocess_store # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $INPROCESS_STORE_DIR/lib, INPROCESS_STORE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_INPROCESS_STORELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong inprocess_store lib version or lib not found])
  dnl ],[
  dnl   -L$INPROCESS_STORE_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(INPROCESS_STORE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(inprocess_store, inprocess_store.c, $ext_shared)
fi
