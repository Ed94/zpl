// file: source/essentials/debug.c


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

void assert_handler( char const* condition, char const* file, s32 line, char const* msg, ... )
{
	_printf_err( "%s:(%d): Assert Failure: ", file, line );

	if ( condition )
		_printf_err( "`%s` ", condition );

	if ( msg )
	{
		va_list va;
		va_start( va, msg );
		_printf_err_va( msg, va );
		va_end( va );
	}

	_printf_err( "%s", "\n" );
}

s32 assert_crash( char const* condition )
{
	ZPL_PANIC( condition );
	return 0;
}

#if defined( ZPL_SYSTEM_UNIX ) || defined( ZPL_SYSTEM_MACOS )
#	include <sched.h>
#endif

#if defined( ZPL_SYSTEM_WINDOWS )
void exit( u32 code )
{
	ExitProcess( code );
}
#else
#	include <stdlib.h>

void exit( u32 code )
{
	exit( code );
}
#endif

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
