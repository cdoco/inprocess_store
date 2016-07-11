/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_INPROCESS_STORE_H
#define PHP_INPROCESS_STORE_H

extern zend_module_entry inprocess_store_module_entry;
#define phpext_inprocess_store_ptr &inprocess_store_module_entry

#define PHP_INPROCESS_STORE_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_INPROCESS_STORE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_INPROCESS_STORE_API __attribute__ ((visibility("default")))
#else
#	define PHP_INPROCESS_STORE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

typedef struct _inproc_content {
    zval zv;
    int expire; //expire
    int type; //type
} inproc_content;


PHP_MINIT_FUNCTION(inprocess_store);
PHP_MSHUTDOWN_FUNCTION(inprocess_store);
PHP_RINIT_FUNCTION(inprocess_store);
PHP_RSHUTDOWN_FUNCTION(inprocess_store);
PHP_MINFO_FUNCTION(inprocess_store);

PHP_FUNCTION(inproc_get);
PHP_FUNCTION(inproc_set);
PHP_FUNCTION(inproc_inc);
PHP_FUNCTION(inproc_del);
PHP_FUNCTION(inproc_exists);
PHP_FUNCTION(inproc_define);

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(inprocess_store)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(inprocess_store)
*/

/* In every utility function you add that needs to use variables
   in php_inprocess_store_globals, call TSRMLS_FETCH(); after declaring other
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as INPROCESS_STORE_G(variable).  You are
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define INPROCESS_STORE_G(v) TSRMG(inprocess_store_globals_id, zend_inprocess_store_globals *, v)
#else
#define INPROCESS_STORE_G(v) (inprocess_store_globals.v)
#endif

#endif	/* PHP_INPROCESS_STORE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
