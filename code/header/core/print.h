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
#	define ZPL_PRINTF_MAXLEN 65536
#endif

ZPL_DEF sw printf( char const* fmt, ... );
ZPL_DEF sw printf_va( char const* fmt, va_list va );
ZPL_DEF sw printf_err( char const* fmt, ... );
ZPL_DEF sw printf_err_va( char const* fmt, va_list va );
ZPL_DEF sw fprintf( FileInfo* f, char const* fmt, ... );
ZPL_DEF sw fprintf_va( FileInfo* f, char const* fmt, va_list va );

// NOTE: A locally persisting buffer is used internally
ZPL_DEF char* bprintf( char const* fmt, ... );

// NOTE: A locally persisting buffer is used internally
ZPL_DEF char* bprintf_va( char const* fmt, va_list va );

ZPL_DEF sw asprintf( AllocatorInfo allocator, char** buffer, char const* fmt, ... );
ZPL_DEF sw asprintf_va( AllocatorInfo allocator, char** buffer, char const* fmt, va_list va );

ZPL_DEF sw snprintf( char* str, sw n, char const* fmt, ... );
ZPL_DEF sw snprintf_va( char* str, sw n, char const* fmt, va_list va );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
