// a_file: header/core/file_misc.h


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#ifndef ZPL_PATH_SEPARATOR
#	if defined( ZPL_SYSTEM_WINDOWS )
#		define ZPL_PATH_SEPARATOR '\\'
#	else
#		define ZPL_PATH_SEPARATOR '/'
#	endif
#endif

#ifndef ZPL_MAX_PATH
#	if defined( ZPL_SYSTEM_WINDOWS )
#		define ZPL_MAX_PATH MAX_PATH
#	elif defined( ZPL_SYSTEM_UNIX ) && ! defined( ZPL_SYSTEM_EMSCRIPTEN )
#		define ZPL_MAX_PATH PATH_MAX
#	else
#		define ZPL_MAX_PATH 4096
#	endif
#endif

/**
 * Checks if a_file/directory exists
 * @param  filepath
 */
ZPL_DEF b32 fs_exists( char const* filepath );

/**
 * Retrieves node's type (a_file, folder, ...)
 * @param  path
 */
ZPL_DEF u8 fs_get_type( char const* path );

/**
 * Retrieves a_file's last write time
 * @param  filepath
 */
ZPL_DEF file_time fs_last_write_time( char const* filepath );

/**
 * Copies the a_file to a directory
 * @param  existing_filename
 * @param  new_filename
 * @param  fail_if_exists
 */
ZPL_DEF b32 fs_copy( char const* existing_filename, char const* new_filename, b32 fail_if_exists );

/**
 * Moves the a_file to a directory
 * @param  existing_filename
 * @param  new_filename
 */
ZPL_DEF b32 fs_move( char const* existing_filename, char const* new_filename );

/**
 * Removes a a_file from a directory
 * @param  filename
 */
ZPL_DEF b32 fs_remove( char const* filename );

ZPL_DEF_INLINE b32 path_is_absolute( char const* path );
ZPL_DEF_INLINE b32 path_is_relative( char const* path );
ZPL_DEF_INLINE b32 path_is_root( char const* path );

ZPL_DEF_INLINE char const* path_base_name( char const* path );
ZPL_DEF_INLINE char const* path_extension( char const* path );

ZPL_DEF void path_fix_slashes( char* path );

ZPL_DEF file_error path_mkdir( char const* path, s32 mode );
ZPL_DEF sw         path_mkdir_recursive( char const* path, s32 mode );
ZPL_DEF file_error path_rmdir( char const* path );

ZPL_DEF char* path_get_full_name( allocator a, char const* path );

/**
 * Returns a_file paths terminated by newline (\n)
 * @param  a_allocator   [description]
 * @param  dirname [description]
 * @param  recurse [description]
 * @return         [description]
 */
ZPL_DEF /*string*/ char* path_dirlist( allocator a_allocator, char const* dirname, b32 recurse );

/**
 * Initialize dirinfo from specified path
 * @param dir  [description]
 * @param path [description]
 */
ZPL_DEF void dirinfo_init( dir_info* dir, char const* path );
ZPL_DEF void dirinfo_free( dir_info* dir );

/**
 * Analyze the entry's dirinfo
 * @param dir_entry [description]
 */
ZPL_DEF void dirinfo_step( dir_entry* dir_entry );

/* inlines */

ZPL_IMPL_INLINE b32 path_is_absolute( char const* path )
{
	b32 result = false;
	ZPL_ASSERT_NOT_NULL( path );
#if defined( ZPL_SYSTEM_WINDOWS )
	result = ( strlen( path ) > 2 ) && char_is_alpha( path[ 0 ] ) && ( path[ 1 ] == ':' && path[ 2 ] == ZPL_PATH_SEPARATOR );
#else
	result = ( strlen( path ) > 0 && path[ 0 ] == ZPL_PATH_SEPARATOR );
#endif
	return result;
}

ZPL_IMPL_INLINE b32 path_is_relative( char const* path )
{
	return ! path_is_absolute( path );
}

ZPL_IMPL_INLINE b32 path_is_root( char const* path )
{
	b32 result = false;
	ZPL_ASSERT_NOT_NULL( path );
#if defined( ZPL_SYSTEM_WINDOWS )
	result = path_is_absolute( path ) && ( strlen( path ) == 3 );
#else
	result = path_is_absolute( path ) && ( strlen( path ) == 1 );
#endif
	return result;
}

ZPL_IMPL_INLINE char const* path_base_name( char const* path )
{
	char const* ls;
	ZPL_ASSERT_NOT_NULL( path );
	path_fix_slashes( ( char* )path );
	ls = char_last_occurence( path, ZPL_PATH_SEPARATOR );
	return ( ls == NULL ) ? path : ls + 1;
}

ZPL_IMPL_INLINE char const* path_extension( char const* path )
{
	char const* ld;
	ZPL_ASSERT_NOT_NULL( path );
	ld = char_last_occurence( path, '.' );
	return ( ld == NULL ) ? NULL : ld + 1;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
