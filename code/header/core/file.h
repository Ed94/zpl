// file: header/core/file.h

/** @file file.c
@brief File handling
@defgroup fileio File handling

File I/O operations as well as path and folder structure manipulation methods. With threading enabled, it also offers async read/write methods.

@{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef u32 file_mode;

typedef enum file_mode_flag
{
	ZPL_FILE_MODE_READ   = ZPL_BIT( 0 ),
	ZPL_FILE_MODE_WRITE  = ZPL_BIT( 1 ),
	ZPL_FILE_MODE_APPEND = ZPL_BIT( 2 ),
	ZPL_FILE_MODE_RW     = ZPL_BIT( 3 ),
	ZPL_FILE_MODES       = ZPL_FILE_MODE_READ | ZPL_FILE_MODE_WRITE | ZPL_FILE_MODE_APPEND | ZPL_FILE_MODE_RW,
} file_mode_flag;

// NOTE: Only used internally and for the file operations
typedef enum seek_whence_type
{
	ZPL_SEEK_WHENCE_BEGIN   = 0,
	ZPL_SEEK_WHENCE_CURRENT = 1,
	ZPL_SEEK_WHENCE_END     = 2,
} seek_whence_type;

typedef enum file_error
{
	ZPL_FILE_ERROR_NONE,
	ZPL_FILE_ERROR_INVALID,
	ZPL_FILE_ERROR_INVALID_FILENAME,
	ZPL_FILE_ERROR_EXISTS,
	ZPL_FILE_ERROR_NOT_EXISTS,
	ZPL_FILE_ERROR_PERMISSION,
	ZPL_FILE_ERROR_TRUNCATION_FAILURE,
	ZPL_FILE_ERROR_NOT_EMPTY,
	ZPL_FILE_ERROR_NAME_TOO_LONG,
	ZPL_FILE_ERROR_UNKNOWN,
} file_error;

typedef union file_descriptor
{
	void* p;
	sptr  i;
	uptr  u;
} file_descriptor;

typedef struct file_operations file_operations;

#define ZPL_FILE_OPEN_PROC( name )     file_error name( file_descriptor* fd, file_operations* ops, file_mode mode, char const * filename )
#define ZPL_FILE_READ_AT_PROC( name )  b32 name( file_descriptor fd, void* buffer, sw size, s64 offset, sw* bytes_read, b32 stop_at_newline )
#define ZPL_FILE_WRITE_AT_PROC( name ) b32 name( file_descriptor fd, void const * buffer, sw size, s64 offset, sw* bytes_written )
#define ZPL_FILE_SEEK_PROC( name )     b32 name( file_descriptor fd, s64 offset, seek_whence_type whence, s64* new_offset )
#define ZPL_FILE_CLOSE_PROC( name )    void name( file_descriptor fd )

typedef ZPL_FILE_OPEN_PROC( file_open_proc );
typedef ZPL_FILE_READ_AT_PROC( file_read_proc );
typedef ZPL_FILE_WRITE_AT_PROC( file_write_proc );
typedef ZPL_FILE_SEEK_PROC( file_seek_proc );
typedef ZPL_FILE_CLOSE_PROC( file_close_proc );

struct file_operations
{
	file_read_proc*  read_at;
	file_write_proc* write_at;
	file_seek_proc*  seek;
	file_close_proc* close;
};

extern file_operations const default_file_operations;

typedef u64 file_time;
typedef enum dir_type
{
	ZPL_DIR_TYPE_FILE,
	ZPL_DIR_TYPE_FOLDER,
	ZPL_DIR_TYPE_UNKNOWN,
} dir_type;

struct dir_info;

typedef struct dir_entry
{
	char const *     filename;
	struct dir_info* dir_info;
	u8               type;
} dir_entry;

typedef struct dir_info
{
	char const * fullpath;
	dir_entry*   entries; // zpl_array

	// Internals
	char** filenames; // zpl_array
	string buf;
} dir_info;

typedef struct zpl_file
{
	file_operations ops;
	file_descriptor fd;
	b32             is_temp;

	char const * filename;
	file_time    last_write_time;
	dir_entry*   dir;
} zpl_file;

typedef enum file_standard_type
{
	ZPL_FILE_STANDARD_INPUT,
	ZPL_FILE_STANDARD_OUTPUT,
	ZPL_FILE_STANDARD_ERROR,

	ZPL_FILE_STANDARD_COUNT,
} file_standard_type;

/**
 * Get standard file I/O.
 * @param  std Check file_standard_type
 * @return     File handle to standard I/O
 */
ZPL_DEF zpl_file* file_get_standard( file_standard_type std );

/**
 * Connects a system handle to a zpl file.
 * @param  file   Pointer to zpl file
 * @param  handle Low-level OS handle to connect
 */
ZPL_DEF void file_connect_handle( zpl_file* file, void* handle );

/**
 * Creates a new file
 * @param  file
 * @param  filename
 */
ZPL_DEF file_error file_create( zpl_file* file, char const * filename );

/**
 * Opens a file
 * @param  file
 * @param  filename
 */
ZPL_DEF file_error file_open( zpl_file* file, char const * filename );

/**
 * Opens a file using a specified mode
 * @param  file
 * @param  mode     Access mode to use
 * @param  filename
 */
ZPL_DEF file_error file_open_mode( zpl_file* file, file_mode mode, char const * filename );

/**
 * Constructs a new file from data
 * @param  file
 * @param  fd       Low-level file descriptor to use
 * @param  ops      File operations to rely upon
 * @param  filename
 */
ZPL_DEF file_error file_new( zpl_file* file, file_descriptor fd, file_operations ops, char const * filename );

/**
 * Returns a size of the file
 * @param  file
 * @return      File size
 */
ZPL_DEF s64 file_size( zpl_file* file );

/**
 * Returns the currently opened file's name
 * @param  file
 */
ZPL_DEF char const * file_name( zpl_file* file );

/**
 * Truncates the file by a specified size
 * @param  file
 * @param  size Size to truncate
 */
ZPL_DEF file_error file_truncate( zpl_file* file, s64 size );

/**
 * Checks whether a file's been changed since the last check
 * @param  file
 */
ZPL_DEF b32 file_has_changed( zpl_file* file );

/**
 * Retrieves a directory listing relative to the file
 * @param file
 */
ZPL_DEF void file_dirinfo_refresh( zpl_file* file );

/**
 * Creates a temporary file
 * @param  file
 */
file_error file_temp( zpl_file* file );

/**
 * Closes the file
 * @param  file
 */
ZPL_DEF file_error file_close( zpl_file* file );

/**
 * Reads file safely
 * @param  file
 * @param  buffer     Buffer to read to
 * @param  size       Size to read
 * @param  offset     Offset to read from
 * @param  bytes_read How much data we've actually read
 */
ZPL_DEF_INLINE b32 file_read_at_check( zpl_file* file, void* buffer, sw size, s64 offset, sw* bytes_read );

/**
 * Writes to file safely
 * @param  file
 * @param  buffer        Buffer to read from
 * @param  size          Size to write
 * @param  offset        Offset to write to
 * @param  bytes_written How much data we've actually written
 */
ZPL_DEF_INLINE b32 file_write_at_check( zpl_file* file, void const * buffer, sw size, s64 offset, sw* bytes_written );

/**
 * Reads file at a specific offset
 * @param  file
 * @param  buffer     Buffer to read to
 * @param  size       Size to read
 * @param  offset     Offset to read from
 * @param  bytes_read How much data we've actually read
 */
ZPL_DEF_INLINE b32 file_read_at( zpl_file* file, void* buffer, sw size, s64 offset );

/**
 * Writes to file at a specific offset
 * @param  file
 * @param  buffer        Buffer to read from
 * @param  size          Size to write
 * @param  offset        Offset to write to
 * @param  bytes_written How much data we've actually written
 */
ZPL_DEF_INLINE b32 file_write_at( zpl_file* file, void const * buffer, sw size, s64 offset );

/**
 * Seeks the file cursor from the beginning of file to a specific position
 * @param  file
 * @param  offset Offset to seek to
 */
ZPL_DEF_INLINE s64 file_seek( zpl_file* file, s64 offset );

/**
 * Seeks the file cursor to the end of the file
 * @param  file
 */
ZPL_DEF_INLINE s64 file_seek_to_end( zpl_file* file );

/**
 * Skips N bytes at the current position
 * @param  file
 * @param  bytes Bytes to skip
 */
ZPL_DEF_INLINE s64 file_skip( zpl_file* file, s64 bytes ); // NOTE: Skips a certain amount of bytes

/**
 * Returns the length from the beginning of the file we've read so far
 * @param  file
 * @return      Our current position in file
 */
ZPL_DEF_INLINE s64 file_tell( zpl_file* file );

/**
 * Reads from a file
 * @param  file
 * @param  buffer Buffer to read to
 * @param  size   Size to read
 */
ZPL_DEF_INLINE b32 file_read( zpl_file* file, void* buffer, sw size );

/**
 * Writes to a file
 * @param  file
 * @param  buffer Buffer to read from
 * @param  size   Size to read
 */
ZPL_DEF_INLINE b32 file_write( zpl_file* file, void const * buffer, sw size );

typedef struct file_contents
{
	zpl_allocator allocator;
	void*         data;
	sw            size;
} file_contents;

/**
 * Reads the whole file contents
 * @param  a              Allocator to use
 * @param  zero_terminate End the read data with null terminator
 * @param  filepath       Path to the file
 * @return                File contents data
 */
ZPL_DEF file_contents file_read_contents( zpl_allocator a, b32 zero_terminate, char const * filepath );

/**
 * Frees the file content data previously read
 * @param  fc
 */
ZPL_DEF void file_free_contents( file_contents* fc );

/**
 * Writes content to a file
 */
ZPL_DEF b32 file_write_contents( char const * filepath, void const * buffer, sw size, file_error* err );

/**
 * Reads the file as array of lines
 *
 * Make sure you free both the returned buffer and the lines (zpl_array)
 * @param  allocator            Allocator to use
 * @param  lines            Reference to zpl_array container we store lines to
 * @param  filename         Path to the file
 * @param  strip_whitespace Strip whitespace when we split to lines?
 * @return                  File content we've read itself
 */
ZPL_DEF char* file_read_lines( zpl_allocator allocator, zpl_array( char* ) * lines, char const * filename, b32 strip_whitespace );

//! @}

/* inlines */

ZPL_IMPL_INLINE b32 file_read_at_check( zpl_file* f, void* buffer, sw size, s64 offset, sw* bytes_read )
{
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	return f->ops.read_at( f->fd, buffer, size, offset, bytes_read, false );
}

ZPL_IMPL_INLINE b32 file_write_at_check( zpl_file* f, void const * buffer, sw size, s64 offset, sw* bytes_written )
{
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	return f->ops.write_at( f->fd, buffer, size, offset, bytes_written );
}

ZPL_IMPL_INLINE b32 file_read_at( zpl_file* f, void* buffer, sw size, s64 offset )
{
	return file_read_at_check( f, buffer, size, offset, NULL );
}

ZPL_IMPL_INLINE b32 file_write_at( zpl_file* f, void const * buffer, sw size, s64 offset )
{
	return file_write_at_check( f, buffer, size, offset, NULL );
}

ZPL_IMPL_INLINE s64 file_seek( zpl_file* f, s64 offset )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, offset, ZPL_SEEK_WHENCE_BEGIN, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE s64 file_seek_to_end( zpl_file* f )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, 0, ZPL_SEEK_WHENCE_END, &new_offset );
	return new_offset;
}

// NOTE: Skips a certain amount of bytes
ZPL_IMPL_INLINE s64 file_skip( zpl_file* f, s64 bytes )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, bytes, ZPL_SEEK_WHENCE_CURRENT, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE s64 file_tell( zpl_file* f )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, 0, ZPL_SEEK_WHENCE_CURRENT, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE b32 file_read( zpl_file* f, void* buffer, sw size )
{
	s64 cur_offset = file_tell( f );
	b32 result     = file_read_at( f, buffer, size, file_tell( f ) );
	file_seek( f, cur_offset + size );
	return result;
}

ZPL_IMPL_INLINE b32 file_write( zpl_file* f, void const * buffer, sw size )
{
	s64 cur_offset = file_tell( f );
	b32 result     = file_write_at( f, buffer, size, file_tell( f ) );
	file_seek( f, cur_offset + size );
	return result;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
