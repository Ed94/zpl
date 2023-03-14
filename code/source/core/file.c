// a_file: source/core/file.c


////////////////////////////////////////////////////////////////
//
// File Handling
//
//
#include <sys/stat.h>

#ifdef ZPL_SYSTEM_MACOS
#	include <copyfile.h>
#endif

#ifdef ZPL_SYSTEM_CYGWIN
#	include <windows.h>
#endif

#if defined( ZPL_SYSTEM_WINDOWS ) && ! defined( ZPL_COMPILER_GCC )
#	include <io.h>
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#if defined( ZPL_SYSTEM_WINDOWS ) || defined( ZPL_SYSTEM_CYGWIN )

internal wchar_t* zpl__alloc_utf8_to_ucs2( allocator a, char const* text, sw* w_len_ )
{
	wchar_t* w_text = NULL;
	sw       len = 0, w_len = 0, w_len1 = 0;
	if ( text == NULL )
	{
		if ( w_len_ )
			*w_len_ = w_len;
		return NULL;
	}
	len = strlen( text );
	if ( len == 0 )
	{
		if ( w_len_ )
			*w_len_ = w_len;
		return NULL;
	}
	w_len = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, text, zpl_cast( int ) len, NULL, 0 );
	if ( w_len == 0 )
	{
		if ( w_len_ )
			*w_len_ = w_len;
		return NULL;
	}
	w_text = alloc_array( a, wchar_t, w_len + 1 );
	w_len1 = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, text, zpl_cast( int ) len, w_text, zpl_cast( int ) w_len );
	if ( w_len1 == 0 )
	{
		free( a, w_text );
		if ( w_len_ )
			*w_len_ = 0;
		return NULL;
	}
	w_text[ w_len ] = 0;
	if ( w_len_ )
		*w_len_ = w_len;
	return w_text;
}

internal ZPL_FILE_SEEK_PROC( zpl__win32_file_seek )
{
	LARGE_INTEGER li_offset;
	li_offset.QuadPart = offset;
	if ( ! SetFilePointerEx( fd.p, li_offset, &li_offset, whence ) )
	{
		return false;
	}

	if ( new_offset )
		*new_offset = li_offset.QuadPart;
	return true;
}

internal ZPL_FILE_READ_AT_PROC( zpl__win32_file_read )
{
	unused( stop_at_newline );
	b32 result = false;
	zpl__win32_file_seek( fd, offset, ZPL_SEEK_WHENCE_BEGIN, NULL );
	DWORD size_ = zpl_cast( DWORD )( size > ZPL_I32_MAX ? ZPL_I32_MAX : size );
	DWORD bytes_read_;
	if ( ReadFile( fd.p, buffer, size_, &bytes_read_, NULL ) )
	{
		if ( bytes_read )
			*bytes_read = bytes_read_;
		result = true;
	}

	return result;
}

internal ZPL_FILE_WRITE_AT_PROC( zpl__win32_file_write )
{
	DWORD size_ = zpl_cast( DWORD )( size > ZPL_I32_MAX ? ZPL_I32_MAX : size );
	DWORD bytes_written_;
	zpl__win32_file_seek( fd, offset, ZPL_SEEK_WHENCE_BEGIN, NULL );
	if ( WriteFile( fd.p, buffer, size_, &bytes_written_, NULL ) )
	{
		if ( bytes_written )
			*bytes_written = bytes_written_;
		return true;
	}
	return false;
}

internal ZPL_FILE_CLOSE_PROC( zpl__win32_file_close )
{
	CloseHandle( fd.p );
}

file_operations const default_file_operations = { zpl__win32_file_read, zpl__win32_file_write, zpl__win32_file_seek, zpl__win32_file_close };

ZPL_NEVER_INLINE ZPL_FILE_OPEN_PROC( zpl__win32_file_open )
{
	DWORD    desired_access;
	DWORD    creation_disposition;
	void*    handle;
	wchar_t* w_text;

	switch ( mode & ZPL_FILE_MODES )
	{
		case ZPL_FILE_MODE_READ :
			desired_access       = GENERIC_READ;
			creation_disposition = OPEN_EXISTING;
			break;
		case ZPL_FILE_MODE_WRITE :
			desired_access       = GENERIC_WRITE;
			creation_disposition = CREATE_ALWAYS;
			break;
		case ZPL_FILE_MODE_APPEND :
			desired_access       = GENERIC_WRITE;
			creation_disposition = OPEN_ALWAYS;
			break;
		case ZPL_FILE_MODE_READ | ZPL_FILE_MODE_RW :
			desired_access       = GENERIC_READ | GENERIC_WRITE;
			creation_disposition = OPEN_EXISTING;
			break;
		case ZPL_FILE_MODE_WRITE | ZPL_FILE_MODE_RW :
			desired_access       = GENERIC_READ | GENERIC_WRITE;
			creation_disposition = CREATE_ALWAYS;
			break;
		case ZPL_FILE_MODE_APPEND | ZPL_FILE_MODE_RW :
			desired_access       = GENERIC_READ | GENERIC_WRITE;
			creation_disposition = OPEN_ALWAYS;
			break;
		default :
			ZPL_PANIC( "Invalid a_file mode" );
			return ZPL_FILE_ERROR_INVALID;
	}

	w_text = zpl__alloc_utf8_to_ucs2( heap_allocator(), filename, NULL );
	handle = CreateFileW( w_text, desired_access, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL );

	free( heap_allocator(), w_text );

	if ( handle == INVALID_HANDLE_VALUE )
	{
		DWORD err = GetLastError();
		switch ( err )
		{
			case ERROR_FILE_NOT_FOUND :
				return ZPL_FILE_ERROR_NOT_EXISTS;
			case ERROR_FILE_EXISTS :
				return ZPL_FILE_ERROR_EXISTS;
			case ERROR_ALREADY_EXISTS :
				return ZPL_FILE_ERROR_EXISTS;
			case ERROR_ACCESS_DENIED :
				return ZPL_FILE_ERROR_PERMISSION;
		}
		return ZPL_FILE_ERROR_INVALID;
	}

	if ( mode & ZPL_FILE_MODE_APPEND )
	{
		LARGE_INTEGER offset = { 0 };
		if ( ! SetFilePointerEx( handle, offset, NULL, ZPL_SEEK_WHENCE_END ) )
		{
			CloseHandle( handle );
			return ZPL_FILE_ERROR_INVALID;
		}
	}

	fd->p = handle;
	*ops  = default_file_operations;
	return ZPL_FILE_ERROR_NONE;
}

#else    // POSIX
#	include <fcntl.h>

internal ZPL_FILE_SEEK_PROC( zpl__posix_file_seek )
{
#	if defined( ZPL_SYSTEM_OSX )
	s64 res = lseek( fd.i, offset, whence );
#	else    // TODO(ZaKlaus): @fixme lseek64
	s64 res = lseek( fd.i, offset, whence );
#	endif
	if ( res < 0 )
		return false;
	if ( new_offset )
		*new_offset = res;
	return true;
}

internal ZPL_FILE_READ_AT_PROC( zpl__posix_file_read )
{
	unused( stop_at_newline );
	sw res = pread( fd.i, buffer, size, offset );
	if ( res < 0 )
		return false;
	if ( bytes_read )
		*bytes_read = res;
	return true;
}

internal ZPL_FILE_WRITE_AT_PROC( zpl__posix_file_write )
{
	sw  res;
	s64 curr_offset = 0;
	zpl__posix_file_seek( fd, 0, ZPL_SEEK_WHENCE_CURRENT, &curr_offset );
	if ( curr_offset == offset )
	{
		// NOTE: Writing to stdout et al. doesn't like pwrite for numerous reasons
		res = write( zpl_cast( int ) fd.i, buffer, size );
	}
	else
	{
		res = pwrite( zpl_cast( int ) fd.i, buffer, size, offset );
	}
	if ( res < 0 )
		return false;
	if ( bytes_written )
		*bytes_written = res;
	return true;
}

internal ZPL_FILE_CLOSE_PROC( zpl__posix_file_close )
{
	close( fd.i );
}

file_operations const default_file_operations = { zpl__posix_file_read, zpl__posix_file_write, zpl__posix_file_seek, zpl__posix_file_close };

ZPL_NEVER_INLINE ZPL_FILE_OPEN_PROC( zpl__posix_file_open )
{
	s32 os_mode;
	switch ( mode & ZPL_FILE_MODES )
	{
		case ZPL_FILE_MODE_READ :
			os_mode = O_RDONLY;
			break;
		case ZPL_FILE_MODE_WRITE :
			os_mode = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case ZPL_FILE_MODE_APPEND :
			os_mode = O_WRONLY | O_APPEND | O_CREAT;
			break;
		case ZPL_FILE_MODE_READ | ZPL_FILE_MODE_RW :
			os_mode = O_RDWR;
			break;
		case ZPL_FILE_MODE_WRITE | ZPL_FILE_MODE_RW :
			os_mode = O_RDWR | O_CREAT | O_TRUNC;
			break;
		case ZPL_FILE_MODE_APPEND | ZPL_FILE_MODE_RW :
			os_mode = O_RDWR | O_APPEND | O_CREAT;
			break;
		default :
			ZPL_PANIC( "Invalid a_file mode" );
			return ZPL_FILE_ERROR_INVALID;
	}

	fd->i = open( filename, os_mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
	if ( fd->i < 0 )
	{
		// TODO: More a_file errors
		return ZPL_FILE_ERROR_INVALID;
	}

	*ops = default_file_operations;
	return ZPL_FILE_ERROR_NONE;
}

#endif

file_error file_new( file* f, file_descriptor fd, file_operations ops, char const* filename )
{
	file_error err = ZPL_FILE_ERROR_NONE;
	sw         len = strlen( filename );

	f->ops             = ops;
	f->fd              = fd;
	f->dir             = NULL;
	f->last_write_time = 0;
	f->filename        = alloc_array( heap_allocator(), char, len + 1 );
	memcopy( zpl_cast( char* ) f->filename, zpl_cast( char* ) filename, len + 1 );

	return err;
}

file_error file_open_mode( file* f, file_mode mode, char const* filename )
{
	file file_ = { 0 };
	*f         = file_;
	file_error err;
#if defined( ZPL_SYSTEM_WINDOWS ) || defined( ZPL_SYSTEM_CYGWIN )
	err = zpl__win32_file_open( &f->fd, &f->ops, mode, filename );
#else
	err = zpl__posix_file_open( &f->fd, &f->ops, mode, filename );
#endif
	if ( err == ZPL_FILE_ERROR_NONE )
		return file_new( f, f->fd, f->ops, filename );
	return err;
}

internal void zpl__dirinfo_free_entry( dir_entry* entry );

file_error file_close( file* f )
{
	if ( ! f )
		return ZPL_FILE_ERROR_INVALID;

	if ( f->filename )
		free( heap_allocator(), zpl_cast( char* ) f->filename );

#if defined( ZPL_SYSTEM_WINDOWS )
	if ( f->fd.p == INVALID_HANDLE_VALUE )
		return ZPL_FILE_ERROR_INVALID;
#else
	if ( f->fd.i < 0 )
		return ZPL_FILE_ERROR_INVALID;
#endif

	if ( f->is_temp )
	{
		f->ops.close( f->fd );
		return ZPL_FILE_ERROR_NONE;
	}

	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.close( f->fd );

	if ( f->dir )
	{
		zpl__dirinfo_free_entry( f->dir );
		mfree( f->dir );
		f->dir = NULL;
	}

	return ZPL_FILE_ERROR_NONE;
}

file_error file_create( file* f, char const* filename )
{
	return file_open_mode( f, ZPL_FILE_MODE_WRITE | ZPL_FILE_MODE_RW, filename );
}

file_error file_open( file* f, char const* filename )
{
	return file_open_mode( f, ZPL_FILE_MODE_READ, filename );
}

char const* file_name( file* f )
{
	return f->filename ? f->filename : "";
}

b32 file_has_changed( file* f )
{
	if ( f->is_temp )
		return false;
	b32       result          = false;
	file_time last_write_time = fs_last_write_time( f->filename );
	if ( f->last_write_time != last_write_time )
	{
		result             = true;
		f->last_write_time = last_write_time;
	}
	return result;
}

// TODO: Is this a bad idea?
global b32  zpl__std_file_set                         = false;
global file zpl__std_files[ ZPL_FILE_STANDARD_COUNT ] = { { 0 } };

#if defined( ZPL_SYSTEM_WINDOWS ) || defined( ZPL_SYSTEM_CYGWIN )

file* file_get_standard( file_standard_type std )
{
	if ( ! zpl__std_file_set )
	{
#	define ZPL__SET_STD_FILE( type, v )                                                                                                                                           \
		zpl__std_files[ type ].fd.p = v;                                                                                                                                           \
		zpl__std_files[ type ].ops  = default_file_operations
		ZPL__SET_STD_FILE( ZPL_FILE_STANDARD_INPUT, GetStdHandle( STD_INPUT_HANDLE ) );
		ZPL__SET_STD_FILE( ZPL_FILE_STANDARD_OUTPUT, GetStdHandle( STD_OUTPUT_HANDLE ) );
		ZPL__SET_STD_FILE( ZPL_FILE_STANDARD_ERROR, GetStdHandle( STD_ERROR_HANDLE ) );
#	undef ZPL__SET_STD_FILE
		zpl__std_file_set = true;
	}
	return &zpl__std_files[ std ];
}

void file_connect_handle( file* a_file, void* handle )
{
	ZPL_ASSERT_NOT_NULL( a_file );
	ZPL_ASSERT_NOT_NULL( handle );

	if ( a_file->is_temp )
		return;

	zero_item( a_file );

	a_file->fd.p = handle;
	a_file->ops  = default_file_operations;
}

file_error file_truncate( file* f, s64 size )
{
	file_error err         = ZPL_FILE_ERROR_NONE;
	s64        prev_offset = file_tell( f );
	file_seek( f, size );
	if ( ! SetEndOfFile( f ) )
		err = ZPL_FILE_ERROR_TRUNCATION_FAILURE;
	file_seek( f, prev_offset );
	return err;
}

b32 fs_exists( char const* name )
{
	WIN32_FIND_DATAW data;
	wchar_t*         w_text;
	void*            handle;
	b32              found = false;
	allocator        a     = heap_allocator();

	w_text = zpl__alloc_utf8_to_ucs2( a, name, NULL );
	if ( w_text == NULL )
	{
		return false;
	}
	handle = FindFirstFileW( w_text, &data );
	free( a, w_text );
	found = handle != INVALID_HANDLE_VALUE;
	if ( found )
		FindClose( handle );
	return found;
}

#else    // POSIX

file* file_get_standard( file_standard_type std )
{
	if ( ! zpl__std_file_set )
	{
#	define ZPL__SET_STD_FILE( type, v )                                                                                                                                           \
		zpl__std_files[ type ].fd.i = v;                                                                                                                                           \
		zpl__std_files[ type ].ops  = default_file_operations
		ZPL__SET_STD_FILE( ZPL_FILE_STANDARD_INPUT, 0 );
		ZPL__SET_STD_FILE( ZPL_FILE_STANDARD_OUTPUT, 1 );
		ZPL__SET_STD_FILE( ZPL_FILE_STANDARD_ERROR, 2 );
#	undef ZPL__SET_STD_FILE
		zpl__std_file_set = true;
	}
	return &zpl__std_files[ std ];
}

file_error file_truncate( file* f, s64 size )
{
	file_error err = ZPL_FILE_ERROR_NONE;
	int        i   = ftruncate( f->fd.i, size );
	if ( i != 0 )
		err = ZPL_FILE_ERROR_TRUNCATION_FAILURE;
	return err;
}

b32 fs_exists( char const* name )
{
	return access( name, F_OK ) != -1;
}

#endif

s64 file_size( file* f )
{
	s64 size        = 0;
	s64 prev_offset = file_tell( f );
	file_seek_to_end( f );
	size = file_tell( f );
	file_seek( f, prev_offset );
	return size;
}

file_error file_temp( file* a_file )
{
	zero_item( a_file );
	FILE* fd = NULL;

#if ( defined( ZPL_SYSTEM_WINDOWS ) && ! defined( ZPL_SYSTEM_TINYC ) ) && ! defined( ZPL_COMPILER_GCC )
	errno_t errcode = tmpfile_s( &fd );

	if ( errcode != 0 )
	{
		fd = NULL;
	}
#else
	fd           = tmpfile();
#endif

	if ( fd == NULL )
	{
		return ZPL_FILE_ERROR_INVALID;
	}

#if defined( ZPL_SYSTEM_WINDOWS ) && ! defined( ZPL_COMPILER_GCC )
	a_file->fd.i = _get_osfhandle( _fileno( fd ) );
#else
	a_file->fd.i = fileno( fd );
#endif
	a_file->ops     = default_file_operations;
	a_file->is_temp = true;
	return ZPL_FILE_ERROR_NONE;
}

file_contents file_read_contents( allocator a, b32 zero_terminate, char const* filepath )
{
	file_contents result = { 0 };
	file          a_file = { 0 };

	result.allocator = a;

	if ( file_open( &a_file, filepath ) == ZPL_FILE_ERROR_NONE )
	{
		sw fsize = zpl_cast( sw ) file_size( &a_file );
		if ( fsize > 0 )
		{
			result.data = alloc( a, zero_terminate ? fsize + 1 : fsize );
			result.size = fsize;
			file_read_at( &a_file, result.data, result.size, 0 );
			if ( zero_terminate )
			{
				u8* str      = zpl_cast( u8* ) result.data;
				str[ fsize ] = '\0';
			}
		}
		file_close( &a_file );
	}

	return result;
}

void file_free_contents( file_contents* fc )
{
	ZPL_ASSERT_NOT_NULL( fc->data );
	free( fc->allocator, fc->data );
	fc->data = NULL;
	fc->size = 0;
}

b32 file_write_contents( char const* filepath, void const* buffer, sw size, file_error* err )
{
	file       f = { 0 };
	file_error open_err;
	b32        write_ok;
	open_err = file_open_mode( &f, ZPL_FILE_MODE_WRITE, filepath );

	if ( open_err != ZPL_FILE_ERROR_NONE )
	{
		if ( err )
			*err = open_err;

		return false;
	}

	write_ok = file_write( &f, buffer, size );
	file_close( &f );
	return write_ok;
}

char* file_read_lines( allocator a_allocator, zpl_array( char* ) * lines, char const* filename, b32 strip_whitespace )
{
	file f = { 0 };
	file_open( &f, filename );
	sw fsize = ( sw )file_size( &f );

	char* contents = ( char* )alloc( a_allocator, fsize + 1 );
	file_read( &f, contents, fsize );
	contents[ fsize ] = 0;
	*lines            = str_split_lines( a_allocator, contents, strip_whitespace );
	file_close( &f );

	return contents;
}

#if ! defined( _WINDOWS_ ) && defined( ZPL_SYSTEM_WINDOWS )
ZPL_IMPORT DWORD WINAPI GetFullPathNameA( char const* lpFileName, DWORD nBufferLength, char* lpBuffer, char** lpFilePart );
ZPL_IMPORT DWORD WINAPI GetFullPathNameW( wchar_t const* lpFileName, DWORD nBufferLength, wchar_t* lpBuffer, wchar_t** lpFilePart );
#endif

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
