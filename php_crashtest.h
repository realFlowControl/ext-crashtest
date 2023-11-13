/* crashtest extension for PHP */

#ifndef PHP_CRASHTEST_H
# define PHP_CRASHTEST_H

extern zend_module_entry crashtest_module_entry;
# define phpext_crashtest_ptr &crashtest_module_entry

# define PHP_CRASHTEST_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_CRASHTEST)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_CRASHTEST_H */
