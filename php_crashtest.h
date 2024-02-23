/* crashtest extension for PHP */

#ifndef PHP_CRASHTEST_H
# define PHP_CRASHTEST_H

extern zend_module_entry crashtest_module_entry;
# define phpext_crashtest_ptr &crashtest_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

# define PHP_CRASHTEST_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_CRASHTEST)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

ZEND_BEGIN_MODULE_GLOBALS(crashtest)
   	void* (*custom_malloc)(size_t);
	void (*custom_free)(void*);
	void* (*custom_realloc)(void *, size_t);
    zend_mm_heap* heap;
    int lineno;
ZEND_END_MODULE_GLOBALS(crashtest)
extern ZEND_DECLARE_MODULE_GLOBALS(crashtest)
#define CT_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(crashtest, v)

#endif	/* PHP_CRASHTEST_H */
