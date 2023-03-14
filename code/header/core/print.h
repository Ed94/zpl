// file: header/core/print.h

/** @file print.c
@brief Printing methods
@defgroup print Printing methods

Various printing methods.
@{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#ifndef ZPL_PRINTF_MAXLEN
#define ZPL_PRINTF_MAXLEN 65536
#endif

ZPL_DEF sw zpl_printf( char const * fmt, ... );
ZPL_DEF sw zpl_printf_va( char const * fmt, va_list va );
ZPL_DEF sw zpl_printf_err( char const * fmt, ... );
ZPL_DEF sw zpl_printf_err_va( char const * fmt, va_list va );
ZPL_DEF sw zpl_fprintf( zpl_file* f, char const * fmt, ... );
ZPL_DEF sw zpl_fprintf_va( zpl_file* f, char const * fmt, va_list va );

// NOTE: A locally persisting buffer is used internally
ZPL_DEF char* bprintf( char const * fmt, ... );

// NOTE: A locally persisting buffer is used internally
ZPL_DEF char* bprintf_va( char const * fmt, va_list va );

ZPL_DEF sw asprintf( zpl_allocator allocator, char** buffer, char const * fmt, ... );
ZPL_DEF sw asprintf_va( zpl_allocator allocator, char** buffer, char const * fmt, va_list va );

ZPL_DEF sw zpl_snprintf( char* str, sw n, char const * fmt, ... );
ZPL_DEF sw zpl_snprintf_va( char* str, sw n, char const * fmt, va_list va );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
