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

volatile uint32_t lineno = 0;

void* (*custom_malloc)(size_t) = NULL;
void (*custom_free)(void*) =  NULL;
void* (*custom_realloc)(void *, size_t) = NULL;

zend_mm_heap* heap = NULL;

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

int crashtest_prepare_heap()
{
	heap = zend_mm_get_heap();
	int custom_heap;
	memcpy(&custom_heap, heap, sizeof(int));
	memset(heap, ZEND_MM_CUSTOM_HEAP_NONE, sizeof(int));
	return custom_heap;
}

void crashtest_restore_heap(int custom_heap)
{
	heap = zend_mm_get_heap();
	memset(heap, custom_heap, sizeof(int));
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void * __attribute__((optnone)) crashtest_malloc(size_t len)
{
    if (has_opline(EG(current_execute_data))) {
        lineno = EG(current_execute_data)->opline->lineno;
    }
	void * ptr;
	int custom_heap = crashtest_prepare_heap();
	if (custom_malloc) {
		ptr = custom_malloc(len);
	} else {
		ptr = _zend_mm_alloc(heap, len);
	}
	crashtest_restore_heap(custom_heap);
	return ptr;
}

void __attribute__((optnone)) crashtest_free(void *ptr)
{
    if (has_opline(EG(current_execute_data))) {
        lineno = EG(current_execute_data)->opline->lineno;
    }
	int custom_heap = crashtest_prepare_heap();
	if (custom_free) {
		custom_free(ptr);
	} else {
		_zend_mm_free(heap, ptr);
	}
	crashtest_restore_heap(custom_heap);
	return;
}

void * __attribute__((optnone)) crashtest_realloc(void * ptr, size_t len)
{
    if (has_opline(EG(current_execute_data))) {
        lineno = EG(current_execute_data)->opline->lineno;
    }
	void * newptr;
	int custom_heap = crashtest_prepare_heap();
	if (custom_realloc) {
		newptr = custom_realloc(ptr, len);
	} else {
		newptr = _zend_mm_realloc(heap, ptr, len);
	}
	crashtest_restore_heap(custom_heap);
	return newptr;
}
#pragma GCC pop_options

PHP_RINIT_FUNCTION(crashtest)
{
#if defined(ZTS) && defined(COMPILE_DL_CRASHTEST)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

	heap = zend_mm_get_heap();
	if (zend_mm_is_custom_heap(heap)) {
		zend_mm_get_custom_handlers(
			heap,
			&custom_malloc,
			&custom_free,
			&custom_realloc
		);
	}
	zend_mm_set_custom_handlers(
		heap,
		crashtest_malloc,
		crashtest_free,
		crashtest_realloc
	);

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(crashtest)
{
	zend_mm_set_custom_handlers(
		heap,
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
