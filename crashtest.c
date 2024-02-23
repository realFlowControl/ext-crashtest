/* crashtest extension for PHP */

#include <stdint.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_crashtest.h"
#include "Zend/zend_alloc.h"

ZEND_DECLARE_MODULE_GLOBALS(crashtest)

static bool has_opline(zend_execute_data *execute_data)
{
    return execute_data
        && execute_data->func
        && ZEND_USER_CODE(execute_data->func->type)
#if CFG_STRICT_OPLINE
#else
        && execute_data->opline
#endif
    ;
}

int crashtest_prepare_heap(zend_mm_heap *heap)
{
	int custom_heap;
	memcpy(&custom_heap, heap, sizeof(int));
	memset(heap, ZEND_MM_CUSTOM_HEAP_NONE, sizeof(int));
	return custom_heap;
}

void crashtest_restore_heap(zend_mm_heap *heap, int custom_heap)
{
	memset(heap, custom_heap, sizeof(int));
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void * __attribute__((optnone)) crashtest_malloc(size_t len)
{
    if (has_opline(EG(current_execute_data))) {
        CT_G(lineno) = EG(current_execute_data)->opline->lineno;
    }
	void * ptr;
	int custom_heap = crashtest_prepare_heap(CT_G(heap));
	if (CT_G(custom_malloc)) {
		ptr = CT_G(custom_malloc)(len);
	} else {
		ptr = _zend_mm_alloc(CT_G(heap), len);
	}
	crashtest_restore_heap(CT_G(heap), custom_heap);
	return ptr;
}

void __attribute__((optnone)) crashtest_free(void *ptr)
{
    if (has_opline(EG(current_execute_data))) {
        CT_G(lineno) = EG(current_execute_data)->opline->lineno;
    }
	int custom_heap = crashtest_prepare_heap(CT_G(heap));
	if (CT_G(custom_free)) {
		CT_G(custom_free)(ptr);
	} else {
		_zend_mm_free(CT_G(heap), ptr);
	}
	crashtest_restore_heap(CT_G(heap), custom_heap);
	return;
}

void * __attribute__((optnone)) crashtest_realloc(void * ptr, size_t len)
{
    if (has_opline(EG(current_execute_data))) {
        CT_G(lineno) = EG(current_execute_data)->opline->lineno;
    }
	void * newptr;
	int custom_heap = crashtest_prepare_heap(CT_G(heap));
	if (CT_G(custom_realloc)) {
		newptr = CT_G(custom_realloc)(ptr, len);
	} else {
		newptr = _zend_mm_realloc(CT_G(heap), ptr, len);
	}
	crashtest_restore_heap(CT_G(heap), custom_heap);
	return newptr;
}
#pragma GCC pop_options

PHP_RINIT_FUNCTION(crashtest)
{
#if defined(ZTS) && defined(COMPILE_DL_CRASHTEST)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

	CT_G(heap) = zend_mm_get_heap();
	if (zend_mm_is_custom_heap(CT_G(heap))) {
		zend_mm_get_custom_handlers(
			CT_G(heap),
			&CT_G(custom_malloc),
			&CT_G(custom_free),
			&CT_G(custom_realloc)
		);
	}
	zend_mm_set_custom_handlers(
		CT_G(heap),
		crashtest_malloc,
		crashtest_free,
		crashtest_realloc
	);

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(crashtest)
{
	zend_mm_set_custom_handlers(
		CT_G(heap),
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
  	PHP_MODULE_GLOBALS(crashtest),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_CRASHTEST
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(crashtest)
#endif
