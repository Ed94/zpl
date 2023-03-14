// a_file: source/process.c

////////////////////////////////////////////////////////////////
//
// Process creation and manipulation methods
//
//

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

static ZPL_ALWAYS_INLINE void zpl__pr_close_file_handle( file* f )
{
	ZPL_ASSERT_NOT_NULL( f );
	f->fd.p = NULL;
}

static ZPL_ALWAYS_INLINE void zpl__pr_close_file_handles( pr* process )
{
	ZPL_ASSERT_NOT_NULL( process );

	zpl__pr_close_file_handle( &process->in );
	zpl__pr_close_file_handle( &process->out );
	zpl__pr_close_file_handle( &process->err );

	process->f_stdin = process->f_stdout = process->f_stderr = NULL;

#ifdef ZPL_SYSTEM_WINDOWS
	process->win32_handle = NULL;
#else
	ZPL_NOT_IMPLEMENTED;
#endif
}

enum
{
	ZPL_PR_HANDLE_MODE_READ,
	ZPL_PR_HANDLE_MODE_WRITE,
	ZPL_PR_HANDLE_MODES,
};

void* zpl__pr_open_handle( u8 type, const char* mode, void** handle )
{
#ifdef ZPL_SYSTEM_WINDOWS
	void* pipes[ ZPL_PR_HANDLE_MODES ];
	s32   fd;

	const u32           flag_inherit = 0x00000001;
	SECURITY_ATTRIBUTES sa           = { size_of( sa ), 0, 1 };

	if ( ! CreatePipe( &pipes[ 0 ], &pipes[ 1 ], zpl_cast( LPSECURITY_ATTRIBUTES ) & sa, 0 ) )
	{
		return NULL;
	}

	if ( ! SetHandleInformation( pipes[ type ], flag_inherit, 0 ) )
	{
		return NULL;
	}

	fd = _open_osfhandle( zpl_cast( sptr ) pipes[ type ], 0 );

	if ( fd != -1 )
	{
		*handle = pipes[ 1 - type ];
		return _fdopen( fd, mode );
	}

	return NULL;
#else
	ZPL_NOT_IMPLEMENTED;
	return NULL;
#endif
}

s32 pr_create( pr* process, const char** args, sw argc, pr_si si, pr_opts options )
{
	ZPL_ASSERT_NOT_NULL( process );
	zero_item( process );

#ifdef ZPL_SYSTEM_WINDOWS
	string              cli, env;
	b32                 c_env           = false;
	STARTUPINFOW        psi             = { 0 };
	PROCESS_INFORMATION pi              = { 0 };
	s32                 err_code        = 0;
	allocator           a               = heap();
	const u32           use_std_handles = 0x00000100;

	psi.cb      = size_of( psi );
	psi.dwFlags = use_std_handles | si.flags;

	if ( options & ZPL_PR_OPTS_CUSTOM_ENV )
	{
		env   = string_join( heap(), zpl_cast( const char** ) si.env, si.env_count, "\0\0" );
		env   = string_appendc( env, "\0" );
		c_env = true;
	}
	else if ( ! ( options & ZPL_PR_OPTS_INHERIT_ENV ) )
	{
		env = ( string ) "\0\0\0\0";
	}
	else
	{
		env = ( string )NULL;
	}

	process->f_stdin  = zpl__pr_open_handle( ZPL_PR_HANDLE_MODE_WRITE, "wb", &psi.hStdInput );
	process->f_stdout = zpl__pr_open_handle( ZPL_PR_HANDLE_MODE_READ, "rb", &psi.hStdOutput );

	if ( options & ZPL_PR_OPTS_COMBINE_STD_OUTPUT )
	{
		process->f_stderr = process->f_stdout;
		psi.hStdError     = psi.hStdOutput;
	}
	else
	{
		process->f_stderr = zpl__pr_open_handle( ZPL_PR_HANDLE_MODE_READ, "rb", &psi.hStdError );
	}

	cli = string_join( heap(), args, argc, " " );

	psi.dwX             = si.posx;
	psi.dwY             = si.posy;
	psi.dwXSize         = si.resx;
	psi.dwYSize         = si.resy;
	psi.dwXCountChars   = si.bufx;
	psi.dwYCountChars   = si.bufy;
	psi.dwFillAttribute = si.fill_attr;
	psi.wShowWindow     = si.show_window;

	wchar_t* w_cli     = zpl__alloc_utf8_to_ucs2( a, cli, NULL );
	wchar_t* w_workdir = zpl__alloc_utf8_to_ucs2( a, si.workdir, NULL );

	if ( ! CreateProcessW( NULL, w_cli, NULL, NULL, 1, 0, env, w_workdir, zpl_cast( LPSTARTUPINFOW ) & psi, zpl_cast( LPPROCESS_INFORMATION ) & pi ) )
	{
		err_code = -1;
		goto pr_free_data;
	}

	process->win32_handle = pi.hProcess;
	CloseHandle( pi.hThread );

	file_connect_handle( &process->in, process->f_stdin );
	file_connect_handle( &process->out, process->f_stdout );
	file_connect_handle( &process->err, process->f_stderr );

pr_free_data:
	string_free( cli );
	free( a, w_cli );
	free( a, w_workdir );

	if ( c_env )
		string_free( env );

	return err_code;

#else
	ZPL_NOT_IMPLEMENTED;
	return -1;
#endif
}

s32 pr_join( pr* process )
{
	s32 ret_code;

	ZPL_ASSERT_NOT_NULL( process );

#ifdef ZPL_SYSTEM_WINDOWS
	if ( process->f_stdin )
	{
		fclose( zpl_cast( FILE* ) process->f_stdin );
	}

	WaitForSingleObject( process->win32_handle, INFINITE );

	if ( ! GetExitCodeProcess( process->win32_handle, zpl_cast( LPDWORD ) & ret_code ) )
	{
		pr_destroy( process );
		return -1;
	}

	pr_destroy( process );

	return ret_code;
#else
	ZPL_NOT_IMPLEMENTED;
	ret_code = -1;
	return ret_code;
#endif
}

void pr_destroy( pr* process )
{
	ZPL_ASSERT_NOT_NULL( process );

#ifdef ZPL_SYSTEM_WINDOWS
	if ( process->f_stdin )
	{
		fclose( zpl_cast( FILE* ) process->f_stdin );
	}

	fclose( zpl_cast( FILE* ) process->f_stdout );

	if ( process->f_stderr != process->f_stdout )
	{
		fclose( zpl_cast( FILE* ) process->f_stderr );
	}

	CloseHandle( process->win32_handle );

	zpl__pr_close_file_handles( process );
#else
	ZPL_NOT_IMPLEMENTED;
#endif
}

void pr_terminate( pr* process, s32 err_code )
{
	ZPL_ASSERT_NOT_NULL( process );

#ifdef ZPL_SYSTEM_WINDOWS
	TerminateProcess( process->win32_handle, zpl_cast( UINT ) err_code );
	pr_destroy( process );
#else
	ZPL_NOT_IMPLEMENTED;
#endif
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
