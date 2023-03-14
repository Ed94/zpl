// a_file: source/core/file_misc.c


#if defined( ZPL_SYSTEM_UNIX ) || defined( ZPL_SYSTEM_MACOS )
#	include <dirent.h>
#endif

#if defined( ZPL_SYSTEM_UNIX ) && ! defined( ZPL_SYSTEM_FREEBSD ) && ! defined( ZPL_SYSTEM_OPENBSD ) && ! defined( ZPL_SYSTEM_CYGWIN ) && ! defined( ZPL_SYSTEM_EMSCRIPTEN )
#	include <sys/sendfile.h>
#endif

#if defined( ZPL_SYSTEM_WINDOWS )
#	include <direct.h>
#	include <io.h>
#endif

#if defined( ZPL_SYSTEM_CYGWIN )
#	include <dirent.h>
#	include <io.h>
#	include <windows.h>
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS


#if defined( ZPL_SYSTEM_WINDOWS ) || defined( ZPL_SYSTEM_CYGWIN )
file_time fs_last_write_time( char const* filepath )
{
	ULARGE_INTEGER            li              = { 0 };
	FILETIME                  last_write_time = { 0 };
	WIN32_FILE_ATTRIBUTE_DATA data            = { 0 };
	allocator                 a               = heap_allocator();

	wchar_t* w_text = zpl__alloc_utf8_to_ucs2( a, filepath, NULL );
	if ( w_text == NULL )
	{
		return 0;
	}
	if ( GetFileAttributesExW( w_text, GetFileExInfoStandard, &data ) )
		last_write_time = data.ftLastWriteTime;

	free( a, w_text );

	li.LowPart  = last_write_time.dwLowDateTime;
	li.HighPart = last_write_time.dwHighDateTime;
	return zpl_cast( file_time ) li.QuadPart;
}

b32 fs_copy( char const* existing_filename, char const* new_filename, b32 fail_if_exists )
{
	b32       result = false;
	allocator a      = heap_allocator();

	wchar_t* w_old = zpl__alloc_utf8_to_ucs2( a, existing_filename, NULL );
	if ( w_old == NULL )
	{
		return false;
	}

	wchar_t* w_new = zpl__alloc_utf8_to_ucs2( a, new_filename, NULL );
	if ( w_new != NULL )
	{
		result = CopyFileW( w_old, w_new, fail_if_exists );
	}

	free( a, w_old );
	free( a, w_new );
	return result;
}

b32 fs_move( char const* existing_filename, char const* new_filename )
{
	b32       result = false;
	allocator a      = heap_allocator();

	wchar_t* w_old = zpl__alloc_utf8_to_ucs2( a, existing_filename, NULL );
	if ( w_old == NULL )
	{
		return false;
	}

	wchar_t* w_new = zpl__alloc_utf8_to_ucs2( a, new_filename, NULL );
	if ( w_new != NULL )
	{
		result = MoveFileW( w_old, w_new );
	}

	free( a, w_old );
	free( a, w_new );
	return result;
}

b32 fs_remove( char const* filename )
{
	b32       result = false;
	allocator a      = heap_allocator();

	wchar_t* w_filename = zpl__alloc_utf8_to_ucs2( a, filename, NULL );
	if ( w_filename == NULL )
	{
		return false;
	}

	result = DeleteFileW( w_filename );

	free( a, w_filename );
	return result;
}

#else

file_time fs_last_write_time( char const* filepath )
{
	time_t      result = 0;
	struct stat file_stat;

	if ( stat( filepath, &file_stat ) )
		result = file_stat.st_mtime;

	return zpl_cast( file_time ) result;
}

#	if defined( ZPL_SYSTEM_FREEBSD )
#		include <sys/socket.h>
#		include <sys/types.h>
#		include <sys/uio.h>
#	endif


b32 fs_copy( char const* existing_filename, char const* new_filename, b32 fail_if_exists )
{
	unused( fail_if_exists );
#	if defined( ZPL_SYSTEM_OSX )
	return copyfile( existing_filename, new_filename, NULL, COPYFILE_DATA ) == 0;
#	elif defined( ZPL_SYSTEM_OPENBSD )
	ZPL_NOT_IMPLEMENTED;
	return 0;
#	elif defined( ZPL_SYSTEM_EMSCRIPTEN )
	ZPL_NOT_IMPLEMENTED;
	return 0;
#	else
	int         existing_fd = open( existing_filename, O_RDONLY, 0 );
	struct stat stat_existing;
	fstat( existing_fd, &stat_existing );

	sw  size;
	int new_fd = open( new_filename, O_WRONLY | O_CREAT, stat_existing.st_mode );

#		if defined( ZPL_SYSTEM_FREEBSD )
	size       = sendfile( new_fd, existing_fd, 0, stat_existing.st_size, NULL, 0, 0 );
#		else
	size = sendfile( new_fd, existing_fd, 0, stat_existing.st_size );
#		endif

	close( new_fd );
	close( existing_fd );

	return size == stat_existing.st_size;
#	endif
}

b32 fs_move( char const* existing_filename, char const* new_filename )
{
	if ( link( existing_filename, new_filename ) == 0 )
	{
		return ( unlink( existing_filename ) != -1 );
	}
	return false;
}

b32 fs_remove( char const* filename )
{
#	if defined( ZPL_SYSTEM_OSX ) || defined( ZPL_SYSTEM_EMSCRIPTEN )
	return ( unlink( filename ) != -1 );
#	else
	return ( remove( filename ) == 0 );
#	endif
}

#endif

char* path_get_full_name( allocator a, char const* path )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	wchar_t* w_path     = NULL;
	wchar_t* w_fullpath = NULL;
	sw       w_len      = 0;
	sw       new_len    = 0;
	sw       new_len1   = 0;
	char*    new_path   = 0;

	w_path = zpl__alloc_utf8_to_ucs2( heap_allocator(), path, NULL );
	if ( w_path == NULL )
	{
		return NULL;
	}

	w_len = GetFullPathNameW( w_path, 0, NULL, NULL );
	if ( w_len == 0 )
	{
		return NULL;
	}

	w_fullpath = alloc_array( heap_allocator(), wchar_t, w_len + 1 );
	GetFullPathNameW( w_path, zpl_cast( int ) w_len, w_fullpath, NULL );
	w_fullpath[ w_len ] = 0;

	free( heap_allocator(), w_path );

	new_len = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, w_fullpath, zpl_cast( int ) w_len, NULL, 0, NULL, NULL );

	if ( new_len == 0 )
	{
		free( heap_allocator(), w_fullpath );
		return NULL;
	}

	new_path = alloc_array( a, char, new_len );
	new_len1 = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, w_fullpath, zpl_cast( int ) w_len, new_path, zpl_cast( int ) new_len, NULL, NULL );

	if ( new_len1 == 0 )
	{
		free( heap_allocator(), w_fullpath );
		free( a, new_path );
		return NULL;
	}

	new_path[ new_len ] = 0;
	return new_path;
#else
	char *p, *result, *fullpath = NULL;
	sw    len;
	p        = realpath( path, NULL );
	fullpath = p;
	if ( p == NULL )
	{
		// NOTE(bill): File does not exist
		fullpath = zpl_cast( char* ) path;
	}

	len = strlen( fullpath );

	result = alloc_array( a, char, len + 1 );
	memmove( result, fullpath, len );
	result[ len ] = 0;
	free( a, p );

	return result;
#endif
}

file_error path_mkdir( char const* path, s32 mode )
{
	s32 error = 0;
#if defined( ZPL_SYSTEM_WINDOWS )
	error = _wmkdir( ( const wchar_t* )utf8_to_ucs2_buf( ( const u8* )path ) );
#else
	error                      = mkdir( path, ( mode_t )mode );
#endif

	if ( error == 0 )
	{
		return ZPL_FILE_ERROR_NONE;
	}

	switch ( errno )
	{
		case EPERM :
		case EACCES :
			return ZPL_FILE_ERROR_PERMISSION;
		case EEXIST :
			return ZPL_FILE_ERROR_EXISTS;
		case ENAMETOOLONG :
			return ZPL_FILE_ERROR_NAME_TOO_LONG;
	}

	return ZPL_FILE_ERROR_UNKNOWN;
}

sw path_mkdir_recursive( char const* path, s32 mode )
{
	char  tmp[ ZPL_MAX_PATH ] = { 0 };
	char* p                   = 0;
	sw    len                 = strlen( path );

	if ( len > size_of( tmp ) - 1 )
	{
		return -1;
	}
	strcpy( tmp, path );
	path_fix_slashes( tmp );
	for ( p = tmp + 1; *p; p++ )
	{
		if ( *p == ZPL_PATH_SEPARATOR )
		{
			*p = 0;
			path_mkdir( tmp, mode );
			*p = ZPL_PATH_SEPARATOR;
		}
	}
	path_mkdir( tmp, mode );
	return 0;
}

file_error path_rmdir( char const* path )
{
	s32 error = 0;
#if defined( ZPL_SYSTEM_WINDOWS )
	error = _wrmdir( ( const wchar_t* )utf8_to_ucs2_buf( ( const u8* )path ) );
#else
	error                      = rmdir( path );
#endif

	if ( error == 0 )
	{
		return ZPL_FILE_ERROR_NONE;
	}

	switch ( errno )
	{
		case EPERM :
		case EACCES :
			return ZPL_FILE_ERROR_PERMISSION;
		case ENOENT :
			return ZPL_FILE_ERROR_NOT_EXISTS;
		case ENOTEMPTY :
			return ZPL_FILE_ERROR_NOT_EMPTY;
		case ENAMETOOLONG :
			return ZPL_FILE_ERROR_NAME_TOO_LONG;
	}

	return ZPL_FILE_ERROR_UNKNOWN;
}

void zpl__file_direntry( allocator a_allocator, char const* dirname, string* output, b32 recurse )
{
#if defined( ZPL_SYSTEM_UNIX ) || defined( ZPL_SYSTEM_OSX )
	DIR *          d, *cd;
	struct dirent* dir;
	d = opendir( dirname );

	if ( d )
	{
		while ( ( dir = readdir( d ) ) )
		{
			if ( dir == 0 )
				break;
			if ( ! str_compare( dir->d_name, "..", 2 ) )
				continue;
			if ( dir->d_name[ 0 ] == '.' && dir->d_name[ 1 ] == 0 )
				continue;

			string dirpath = string_make( a_allocator, dirname );
			dirpath        = string_appendc( dirpath, "/" );
			dirpath        = string_appendc( dirpath, dir->d_name );

			*output = string_appendc( *output, dirpath );
			*output = string_appendc( *output, "\n" );

			if ( recurse && ( cd = opendir( dirpath ) ) != NULL && dir->d_type == DT_DIR )
			{
				zpl__file_direntry( a_allocator, dirpath, output, recurse );
			}
			string_free( dirpath );
		}
	}
#elif defined( ZPL_SYSTEM_WINDOWS )
	uw                  length = strlen( dirname );
	struct _wfinddata_t data;
	sptr                findhandle;

	char directory[ MAX_PATH ] = { 0 };
	strncpy( directory, dirname, length );

	// keeping it native
	for ( uw i = 0; i < length; i++ )
	{
		if ( directory[ i ] == '/' )
			directory[ i ] = '\\';
	}

	// remove trailing slashses
	if ( directory[ length - 1 ] == '\\' )
	{
		directory[ length - 1 ] = '\0';
	}

	// attach search pattern
	string findpath = string_make( a_allocator, directory );
	findpath        = string_appendc( findpath, "\\" );
	findpath        = string_appendc( findpath, "*" );

	findhandle = _wfindfirst( ( const wchar_t* )utf8_to_ucs2_buf( ( const u8* )findpath ), &data );
	string_free( findpath );

	if ( findhandle != -1 )
	{
		do
		{
			char* filename = ( char* )ucs2_to_utf8_buf( ( const u16* )data.name );
			if ( ! str_compare( filename, "..", 2 ) )
				continue;
			if ( filename[ 0 ] == '.' && filename[ 1 ] == 0 )
				continue;

			string dirpath = string_make( a_allocator, directory );
			dirpath        = string_appendc( dirpath, "\\" );
			dirpath        = string_appendc( dirpath, filename );
			DWORD attrs    = GetFileAttributesW( ( const wchar_t* )utf8_to_ucs2_buf( ( const u8* )dirpath ) );

			*output = string_appendc( *output, dirpath );
			*output = string_appendc( *output, "\n" );

			if ( recurse && ( data.attrib & _A_SUBDIR ) && ! ( attrs & FILE_ATTRIBUTE_REPARSE_POINT ) )
			{
				zpl__file_direntry( a_allocator, dirpath, output, recurse );
			}

			string_free( dirpath );
		} while ( _wfindnext( findhandle, &data ) != -1 );
		_findclose( findhandle );
	}
#else
	// TODO: Implement other OSes
#endif
}

string path_dirlist( allocator a_allocator, char const* dirname, b32 recurse )
{
	string buf = string_make_reserve( a_allocator, 4 );
	zpl__file_direntry( a_allocator, dirname, &buf, recurse );
	return buf;
}

void dirinfo_init( dir_info* dir, char const* path )
{
	ZPL_ASSERT_NOT_NULL( dir );

	dir_info dir_ = { 0 };
	*dir          = dir_;
	dir->fullpath = ( char const* )malloc( strlen( path ) );
	strcpy( ( char* )dir->fullpath, path );


	string dirlist = path_dirlist( heap(), path, false );
	char** files   = str_split_lines( heap(), dirlist, false );
	dir->filenames = files;
	dir->buf       = dirlist;

	array_init( dir->entries, heap() );

	for ( s32 i = 0; i < array_count( files ); ++i )
	{
		dir_entry entry = { 0 };
		entry.filename  = files[ i ];
		entry.type      = fs_get_type( entry.filename );

		array_append( dir->entries, entry );
	}
}

internal void zpl__dirinfo_free_entry( dir_entry* entry )
{
	if ( entry->dir_info )
	{
		dirinfo_free( entry->dir_info );
		mfree( entry->dir_info );
		entry->dir_info = NULL;
	}
}

void dirinfo_free( dir_info* dir )
{
	ZPL_ASSERT_NOT_NULL( dir );

	for ( sw i = 0; i < array_count( dir->entries ); ++i )
	{
		zpl__dirinfo_free_entry( dir->entries + i );
	}

	array_free( dir->entries );
	array_free( dir->filenames );
	string_free( dir->buf );
	mfree( ( void* )dir->fullpath );
}

u8 fs_get_type( char const* path )
{
#ifdef ZPL_SYSTEM_WINDOWS
	DWORD attrs = GetFileAttributesW( ( const wchar_t* )utf8_to_ucs2_buf( ( const u8* )path ) );

	if ( attrs == INVALID_FILE_ATTRIBUTES )
	{
		return ZPL_DIR_TYPE_UNKNOWN;
	}

	if ( attrs & FILE_ATTRIBUTE_DIRECTORY )
		return ZPL_DIR_TYPE_FOLDER;
	else
		return ZPL_DIR_TYPE_FILE;

#else
	struct stat s;
	if ( stat( path, &s ) == 0 )
	{
		if ( s.st_mode & S_IFDIR )
			return ZPL_DIR_TYPE_FOLDER;
		else
			return ZPL_DIR_TYPE_FILE;
	}
#endif

	return ZPL_DIR_TYPE_UNKNOWN;
}

void dirinfo_step( dir_entry* entry )
{
	if ( entry->dir_info )
	{
		zpl__dirinfo_free_entry( entry );
	}

	entry->dir_info  = ( dir_info* )malloc( sizeof( dir_info ) );
	dir_info dir_    = { 0 };
	*entry->dir_info = dir_;

	local_persist char buf[ 128 ] = { 0 };
	char const*        path       = entry->filename;

	if ( entry->type != ZPL_DIR_TYPE_FOLDER )
	{
		path_fix_slashes( ( char* )path );
		char const* slash = char_last_occurence( path, ZPL_PATH_SEPARATOR );
		strncpy( buf, path, slash - path );
		path = buf;
	}

	dirinfo_init( entry->dir_info, path );
}

void file_dirinfo_refresh( file* a_file )
{
	if ( a_file->is_temp )
		return;

	if ( a_file->dir )
	{
		zpl__dirinfo_free_entry( a_file->dir );
		mfree( a_file->dir );
		a_file->dir = NULL;
	}

	a_file->dir           = ( dir_entry* )malloc( sizeof( dir_entry ) );
	dir_entry dir_        = { 0 };
	*a_file->dir          = dir_;
	a_file->dir->filename = a_file->filename;
	a_file->dir->type     = ZPL_DIR_TYPE_FILE;

	dirinfo_step( a_file->dir );
}

void path_fix_slashes( char* path )
{
#ifdef ZPL_SYSTEM_WINDOWS
	char* p = path;

	while ( *p != '\0' )
	{
		if ( *p == '/' )
			*p = '\\';

		++p;
	}
#endif
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
