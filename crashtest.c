/* crashtest extension for PHP */

#include <stdint.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_crashtest.h"
#include "Zend/zend_alloc.h"

uint32_t lineno;

void * __attribute__((optnone)) allocation_tracking_malloc(size_t len)
{
    if (EG(current_execute_data) &&
        EG(current_execute_data)->func &&
        ZEND_USER_CODE(EG(current_execute_data)->func->type) &&
        EG(current_execute_data)->opline
    ) {
        lineno = EG(current_execute_data)->opline->lineno;
    }
    return _zend_mm_alloc(zend_mm_get_heap(), len);
}

void __attribute__((optnone)) allocation_tracking_free(void *ptr)
{
    if (EG(current_execute_data) &&
        EG(current_execute_data)->func &&
        ZEND_USER_CODE(EG(current_execute_data)->func->type) &&
        EG(current_execute_data)->opline
    ) {
        lineno = EG(current_execute_data)->opline->lineno;
    }
    _zend_mm_free(zend_mm_get_heap(), ptr);
}

void * __attribute__((optnone)) allocation_tracking_realloc(void * ptr, size_t len)
{
    if (EG(current_execute_data) &&
        EG(current_execute_data)->func &&
        ZEND_USER_CODE(EG(current_execute_data)->func->type) &&
        EG(current_execute_data)->opline
    ) {
        lineno = EG(current_execute_data)->opline->lineno;
    }
    return _zend_mm_realloc(zend_mm_get_heap(), ptr, len);
}

PHP_RINIT_FUNCTION(crashtest)
{
#if defined(ZTS) && defined(COMPILE_DL_CRASHTEST)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    zend_mm_set_custom_handlers(
        zend_mm_get_heap(),
        allocation_tracking_malloc,
        allocation_tracking_free,
        allocation_tracking_realloc
    );

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(crashtest)
{
    zend_mm_set_custom_handlers(
        zend_mm_get_heap(),
        NULL,
        NULL,
        NULL
    );
    return SUCCESS;
}

/* {{{ crashtest_module_entry */
zend_module_entry crashtest_module_entry = {
    STANDARD_MODULE_HEADER,
    "crashtest",                    /* Extension name */
    NULL,                            /* zend_function_entry */
    NULL,                            /* PHP_MINIT - Module initialization */
    NULL,                            /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(crashtest),            /* PHP_RINIT - Request initialization */
    PHP_RSHUTDOWN(crashtest),        /* PHP_RSHUTDOWN - Request shutdown */
    NULL,                            /* PHP_MINFO - Module info */
    PHP_CRASHTEST_VERSION,        /* Version */
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CRASHTEST
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(crashtest)
#endif