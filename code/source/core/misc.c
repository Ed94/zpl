// file: source/core/misc.c

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

void yield( void )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	Sleep( 0 );
#else
	sched_yield();
#endif
}

const char* get_env( const char* name )
{
	char*       buffer = NULL;
	const char* ptr    = get_env_buf( name );

	if ( ptr == NULL )
	{
		return NULL;
	}

	sw ptr_size = zpl_strlen( ptr );
	buffer      = (char*)malloc( ptr_size * sizeof( char ) + 1 );
	zpl_memcopy( (char*)buffer, ptr, ptr_size + 1 );
	return buffer;
}

const char* get_env_buf( const char* name )
{
#ifdef ZPL_SYSTEM_WINDOWS
	local_persist wchar_t wbuffer[ 32767 ] = { 0 };
	local_persist char    buffer[ 32767 ]  = { 0 };

	if ( ! GetEnvironmentVariableW( zpl_cast( LPCWSTR ) utf8_to_ucs2_buf( zpl_cast( const u8* ) name ), zpl_cast( LPWSTR ) wbuffer, 32767 ) )
	{
		return NULL;
	}

	ucs2_to_utf8( zpl_cast( u8* ) buffer, 32767, zpl_cast( const u16* ) wbuffer );

	return (const char*)buffer;
#else
	return (const char*)getenv( name );
#endif
}

string get_env_str( const char* name )
{
	const char* buf = get_env_buf( name );

	if ( buf == NULL )
	{
		return NULL;
	}

	string str = string_make( heap(), buf );
	return str;
}

void set_env( const char* name, const char* value )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	SetEnvironmentVariableA( name, value );
#else
	setenv( name, value, 1 );
#endif
}

void unset_env( const char* name )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	SetEnvironmentVariableA( name, NULL );
#else
	unsetenv( name );
#endif
}

#if ! defined( ZPL_SYSTEM_WINDOWS )
extern char** environ;
#endif

u32 system_command( const char* command, uw buffer_len, char* buffer )
{
#if defined( ZPL_SYSTEM_EMSCRIPTEN )
	ZPL_PANIC( "system_command not supported" );
#else

#if defined( ZPL_SYSTEM_WINDOWS )
	FILE* handle = _popen( command, "r" );
#else
	FILE* handle = popen( command, "r" );
#endif

	if ( ! handle )
		return 0;

	int c;
	uw  i = 0;
	while ( ( c = getc( handle ) ) != EOF && i++ < buffer_len )
	{
		*buffer++ = c;
	}

#if defined( ZPL_SYSTEM_WINDOWS )
	_pclose( handle );
#else
	pclose( handle );
#endif

#endif

	return 1;
}

string system_command_str( const char* command, zpl_allocator backing )
{
#if defined( ZPL_SYSTEM_EMSCRIPTEN )
	ZPL_PANIC( "system_command not supported" );
#else

#if defined( ZPL_SYSTEM_WINDOWS )
	FILE* handle = _popen( command, "r" );
#else
	FILE* handle = popen( command, "r" );
#endif

	if ( ! handle )
		return NULL;

	string output = string_make_reserve( backing, 4 );

	int c;
	while ( ( c = getc( handle ) ) != EOF )
	{
		char ins[ 2 ] = { (char)c, 0 };
		output        = string_appendc( output, ins );
	}

#if defined( ZPL_SYSTEM_WINDOWS )
	_pclose( handle );
#else
	pclose( handle );
#endif
	return output;
#endif
	return NULL;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
