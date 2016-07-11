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
  | Author: ZiHang Gao <gaozihang@zhangyue.com>                          |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h" /* for serialize */
#include "ext/standard/php_smart_str.h" /* for smart_str */
#include "php_inprocess_store.h"

static int le_inprocess_store;

HashTable inprocess_store_ht;
long curr_time; /* curr_time is updated when a new request comes */

//serialize array or object
static inline int inproc_serializer(zval *pzval, smart_str *buf TSRMLS_DC) {
    php_serialize_data_t var_hash;

    PHP_VAR_SERIALIZE_INIT(var_hash);
    php_var_serialize(buf, &pzval, &var_hash TSRMLS_CC);
    PHP_VAR_SERIALIZE_DESTROY(var_hash);

    return 1;
}

//unserialize array or object
zval * inproc_unserializer(char *ct, size_t len TSRMLS_DC) {
    zval *pval;
    const unsigned char *p;
    php_unserialize_data_t var_hash;
    p = (const unsigned char*)ct;

    MAKE_STD_ZVAL(pval);
    ZVAL_FALSE(pval);
    PHP_VAR_UNSERIALIZE_INIT(var_hash);
    if (!php_var_unserialize(&pval, &p, p + len,  &var_hash TSRMLS_CC)) {
        zval_ptr_dtor(&pval);
        PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
        return NULL;
    }
    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);

    return pval;
}

//if expired return false, else return true
static inline int inproc_check_ttl(char *key, int key_len, long expire) {

    if (expire > 0) {
        if (curr_time > expire) { //exired
            zend_hash_del(&inprocess_store_ht, key, key_len);
            return SUCCESS;
        }
	}
	return FAILURE;
}

//replace zv_pp with duplicated memory poiter
static inline int inproc_dup_zval(zval **zv_pp){
    zval *zdest_p;

    switch(Z_TYPE_PP(zv_pp)) {
        case IS_NULL:
        case IS_BOOL:
        case IS_LONG:
        case IS_DOUBLE:
            return 0;
        case IS_CONSTANT:
        case IS_STRING:
            MAKE_STD_ZVAL(zdest_p);
            ZVAL_STRINGL(
                zdest_p,
                pestrndup(Z_STRVAL_PP(zv_pp), Z_STRLEN_PP(zv_pp), 1),
                Z_STRLEN_PP(zv_pp),
                0
            );
            *zv_pp = zdest_p;
            return 0;
        case IS_ARRAY:
#ifdef IS_CONSTANT_ARRAY
		case IS_CONSTANT_ARRAY:
#endif
        case IS_OBJECT:
            {
                smart_str buf = {0};
                if (inproc_serializer(*zv_pp, &buf TSRMLS_CC)) {

                    MAKE_STD_ZVAL(zdest_p);
                    ZVAL_STRINGL(
                        zdest_p,
                        pestrndup(buf.c, buf.len, 1),
                        buf.len,
                        0
                    );
                    *zv_pp = zdest_p;
                }
                smart_str_free(&buf);
            }
            return 0;
        case IS_RESOURCE:
        default:
            php_error(E_ERROR, "inprocess_store wrong zval type");
            break;
    }

    return -1;
}

//release the memory allocated by inproc_dup_zval
static void inproc_dtor_zval(void *ct){
    inproc_content *t = (inproc_content *)ct;

    switch(Z_TYPE_P(&t->zv)) {
        case IS_NULL:
        case IS_BOOL:
        case IS_LONG:
        case IS_DOUBLE:
            break;
        case IS_CONSTANT:
        case IS_STRING:
        case IS_ARRAY:
#ifdef IS_CONSTANT_ARRAY
		case IS_CONSTANT_ARRAY:
#endif
        case IS_OBJECT:
            pefree(Z_STRVAL_P(&t->zv), 1);
            break;
        case IS_RESOURCE:
        default:
            php_error(E_ERROR, "inprocess_store wrong zval type");
            break;
    }

    return;
}

const zend_function_entry inprocess_store_functions[] = {
    PHP_FE(inproc_get,	NULL)
    PHP_FE(inproc_set,	NULL)
    PHP_FE(inproc_inc,	NULL)
    PHP_FE(inproc_del,	NULL)
    PHP_FE(inproc_exists,	NULL)
    PHP_FE(inproc_define,	NULL)
    PHP_FE_END	/* Must be the last line in inprocess_store_functions[] */
};


zend_module_entry inprocess_store_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "inprocess_store",
    inprocess_store_functions,
    PHP_MINIT(inprocess_store),
    PHP_MSHUTDOWN(inprocess_store),
    PHP_RINIT(inprocess_store),
    NULL,	/* PHP_RSHUTDOWN(inprocess_store) */
    PHP_MINFO(inprocess_store),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_INPROCESS_STORE_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_INPROCESS_STORE
ZEND_GET_MODULE(inprocess_store)
#endif

PHP_MINIT_FUNCTION(inprocess_store)
{
	zend_hash_init(&inprocess_store_ht, 0, NULL, (void (*)(void *))inproc_dtor_zval, 1);

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(inprocess_store)
{
    zend_hash_destroy(&inprocess_store_ht);

    return SUCCESS;
}

PHP_RINIT_FUNCTION(inprocess_store)
{
    curr_time = time((time_t*)NULL);
    return SUCCESS;
}

PHP_MINFO_FUNCTION(inprocess_store)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Inprocess Store Support", "enabled");
    php_info_print_table_row(2, "Version", "0.0.1");
    php_info_print_table_end();
}

PHP_FUNCTION(inproc_get)
{
    char *key = NULL;
    int key_len;
    inproc_content *ct = NULL;
    zval *zv_p;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    if (zend_hash_find(&inprocess_store_ht, key, key_len, (void **)&ct) == SUCCESS) {

        if (inproc_check_ttl(key, key_len, ct->expire) == SUCCESS){
            RETURN_FALSE;
        }

        switch(ct->type) {
            case IS_NULL:
            case IS_BOOL:
            case IS_LONG:
            case IS_DOUBLE:
            case IS_CONSTANT:
            case IS_STRING:
                zv_p = &ct->zv;
                break;
            case IS_ARRAY:
#ifdef IS_CONSTANT_ARRAY
            case IS_CONSTANT_ARRAY:
#endif
            case IS_OBJECT:
                zv_p = inproc_unserializer(Z_STRVAL_P(&ct->zv), Z_STRLEN_P(&ct->zv));
                break;
            case IS_RESOURCE:
            default:
                php_error(E_ERROR, "inprocess_store wrong zval type");
                break;
        }

        RETURN_ZVAL(zv_p, 1, 0);
    }

    RETURN_FALSE;
}

PHP_FUNCTION(inproc_set)
{
    char *key = NULL;
    int key_len;
    zval *zv_p = NULL;
    long expire = 0;
    inproc_content ct;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|l", &key, &key_len, &zv_p, &expire) == FAILURE) {
        return;
    }

    ct.type = Z_TYPE_P(zv_p);

    inproc_dup_zval(&zv_p);

    if (expire) {
        expire += curr_time;
    }

    memcpy(&ct.zv, zv_p, sizeof(zval));
    ct.expire = expire;

    if (zend_hash_update(&inprocess_store_ht, key, key_len, (void *)&ct, sizeof(ct), NULL) == FAILURE) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(inproc_inc)
{
    char *key = NULL;
    long step = 1;
    long expire = 0;
    zval *zv_p;
    inproc_content *ct = NULL;
    int key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &key, &key_len, &step, &expire) == FAILURE) {
        return;
    }

    if (zend_hash_find(&inprocess_store_ht, key, key_len, (void **)&ct) == SUCCESS) {

        if (inproc_check_ttl(key, key_len, ct->expire) == SUCCESS) {
            RETURN_FALSE;
        }

        if (Z_TYPE_P(&ct->zv) != IS_LONG) {
            php_error(E_ERROR, "inproc_inc require long zval");
            RETURN_FALSE;
        }

        Z_LVAL_P(&ct->zv) += step;

        if (expire) {
            ct->expire += curr_time;
        }

    } else {
        ct = (inproc_content *)emalloc(sizeof(inproc_content));

        ZVAL_LONG(&ct->zv, step);

        if (expire) {
            expire += curr_time;
        }

        ct->type = IS_LONG;
        ct->expire = expire;

        if (zend_hash_add(&inprocess_store_ht, key, key_len, (void *)ct, sizeof(inproc_content), NULL) == FAILURE) {
            efree(ct);
            RETURN_FALSE;
        }

        efree(ct);
    }

    zv_p = &ct->zv;
    RETURN_ZVAL(zv_p, 1, 0);
}

PHP_FUNCTION(inproc_exists)
{
    char *key = NULL;
    inproc_content *ct = NULL;
    int key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    if (zend_hash_find(&inprocess_store_ht, key, key_len, (void **)&ct) == SUCCESS) {

        if (inproc_check_ttl(key, key_len, ct->expire) == SUCCESS) {
            RETURN_FALSE;
        }

        RETURN_TRUE;
    }

    RETURN_FALSE;
}

PHP_FUNCTION(inproc_del)
{
    char *key = NULL;
    int key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    if (zend_hash_exists(&inprocess_store_ht, key, key_len)) {
        if (zend_hash_del(&inprocess_store_ht, key, key_len) == FAILURE) {
            RETURN_FALSE;
        }
    }

    RETURN_TRUE;
}

//persistent define
PHP_FUNCTION(inproc_define)
{
    char *key = NULL;
    int key_len;
    zval* zv_p;
    zend_constant c;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &key, &key_len, &zv_p) == FAILURE) {
        return;
    }
    
    c.name_len = key_len+1;
    c.name = zend_strndup(key, key_len);
    c.module_number = PHP_USER_CONSTANT;
    c.flags = CONST_CS | CONST_PERSISTENT;
    c.value = *zv_p;
    
    switch(Z_TYPE_P(zv_p)){
            case IS_NULL:
            case IS_BOOL:
            case IS_LONG:
            case IS_DOUBLE:
                break;
            case IS_CONSTANT:
            case IS_STRING:
                 Z_STRVAL_P(&c.value) = zend_strndup(Z_STRVAL_P(&c.value), Z_STRLEN_P(&c.value));
                break;
            default:
                php_error(E_ERROR, "inprocess_define wrong zval type");
                break;
    }

    if(zend_register_constant(&c TSRMLS_CC) == FAILURE) {
        php_error(E_WARNING, "inprocess_define register contants faild");
        if(Z_TYPE_P(zv_p) == IS_STRING){
            pefree(Z_STRVAL_P(&c.value), 1);
        }
        
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

