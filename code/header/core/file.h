// a_file: header/core/file.h

/** @a_file file.c
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

// NOTE: Only used internally and for the a_file operations
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

#define ZPL_FILE_OPEN_PROC( name )     file_error name( file_descriptor* fd, file_operations* ops, file_mode mode, char const* filename )
#define ZPL_FILE_READ_AT_PROC( name )  b32 name( file_descriptor fd, void* buffer, sw size, s64 offset, sw* bytes_read, b32 stop_at_newline )
#define ZPL_FILE_WRITE_AT_PROC( name ) b32 name( file_descriptor fd, void const* buffer, sw size, s64 offset, sw* bytes_written )
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
	char const*      filename;
	struct dir_info* dir_info;
	u8               type;
} dir_entry;

typedef struct dir_info
{
	char const* fullpath;
	dir_entry*  entries;    // zpl_array

	// Internals
	char** filenames;    // zpl_array
	string buf;
} dir_info;

typedef struct file
{
	file_operations ops;
	file_descriptor fd;
	b32             is_temp;

	char const* filename;
	file_time   last_write_time;
	dir_entry*  dir;
} file;

typedef enum file_standard_type
{
	ZPL_FILE_STANDARD_INPUT,
	ZPL_FILE_STANDARD_OUTPUT,
	ZPL_FILE_STANDARD_ERROR,

	ZPL_FILE_STANDARD_COUNT,
} file_standard_type;

/**
 * Get standard a_file I/O.
 * @param  std Check file_standard_type
 * @return     File handle to standard I/O
 */
ZPL_DEF file* file_get_standard( file_standard_type std );

/**
 * Connects a system handle to a zpl a_file.
 * @param  a_file   Pointer to zpl a_file
 * @param  handle Low-level OS handle to connect
 */
ZPL_DEF void file_connect_handle( file* a_file, void* handle );

/**
 * Creates a new a_file
 * @param  a_file
 * @param  filename
 */
ZPL_DEF file_error file_create( file* a_file, char const* filename );

/**
 * Opens a a_file
 * @param  a_file
 * @param  filename
 */
ZPL_DEF file_error file_open( file* a_file, char const* filename );

/**
 * Opens a a_file using a specified mode
 * @param  a_file
 * @param  mode     Access mode to use
 * @param  filename
 */
ZPL_DEF file_error file_open_mode( file* a_file, file_mode mode, char const* filename );

/**
 * Constructs a new a_file from data
 * @param  a_file
 * @param  fd       Low-level a_file descriptor to use
 * @param  ops      File operations to rely upon
 * @param  filename
 */
ZPL_DEF file_error file_new( file* a_file, file_descriptor fd, file_operations ops, char const* filename );

/**
 * Returns a size of the a_file
 * @param  a_file
 * @return      File size
 */
ZPL_DEF s64 file_size( file* a_file );

/**
 * Returns the currently opened a_file's name
 * @param  a_file
 */
ZPL_DEF char const* file_name( file* a_file );

/**
 * Truncates the a_file by a specified size
 * @param  a_file
 * @param  size Size to truncate
 */
ZPL_DEF file_error file_truncate( file* a_file, s64 size );

/**
 * Checks whether a a_file's been changed since the last check
 * @param  a_file
 */
ZPL_DEF b32 file_has_changed( file* a_file );

/**
 * Retrieves a directory listing relative to the a_file
 * @param a_file
 */
ZPL_DEF void file_dirinfo_refresh( file* a_file );

/**
 * Creates a temporary a_file
 * @param  a_file
 */
file_error file_temp( file* a_file );

/**
 * Closes the a_file
 * @param  a_file
 */
ZPL_DEF file_error file_close( file* a_file );

/**
 * Reads a_file safely
 * @param  a_file
 * @param  buffer     Buffer to read to
 * @param  size       Size to read
 * @param  offset     Offset to read from
 * @param  bytes_read How much data we've actually read
 */
ZPL_DEF_INLINE b32 file_read_at_check( file* a_file, void* buffer, sw size, s64 offset, sw* bytes_read );

/**
 * Writes to a_file safely
 * @param  a_file
 * @param  buffer        Buffer to read from
 * @param  size          Size to write
 * @param  offset        Offset to write to
 * @param  bytes_written How much data we've actually written
 */
ZPL_DEF_INLINE b32 file_write_at_check( file* a_file, void const* buffer, sw size, s64 offset, sw* bytes_written );


/**
 * Reads a_file at a specific offset
 * @param  a_file
 * @param  buffer     Buffer to read to
 * @param  size       Size to read
 * @param  offset     Offset to read from
 * @param  bytes_read How much data we've actually read
 */
ZPL_DEF_INLINE b32 file_read_at( file* a_file, void* buffer, sw size, s64 offset );

/**
 * Writes to a_file at a specific offset
 * @param  a_file
 * @param  buffer        Buffer to read from
 * @param  size          Size to write
 * @param  offset        Offset to write to
 * @param  bytes_written How much data we've actually written
 */
ZPL_DEF_INLINE b32 file_write_at( file* a_file, void const* buffer, sw size, s64 offset );

/**
 * Seeks the a_file cursor from the beginning of a_file to a specific position
 * @param  a_file
 * @param  offset Offset to seek to
 */
ZPL_DEF_INLINE s64 file_seek( file* a_file, s64 offset );

/**
 * Seeks the a_file cursor to the end of the a_file
 * @param  a_file
 */
ZPL_DEF_INLINE s64 file_seek_to_end( file* a_file );

/**
 * Skips N bytes at the current position
 * @param  a_file
 * @param  bytes Bytes to skip
 */
ZPL_DEF_INLINE s64 file_skip( file* a_file, s64 bytes );    // NOTE: Skips a certain amount of bytes

/**
 * Returns the length from the beginning of the a_file we've read so far
 * @param  a_file
 * @return      Our current position in a_file
 */
ZPL_DEF_INLINE s64 file_tell( file* a_file );

/**
 * Reads from a a_file
 * @param  a_file
 * @param  buffer Buffer to read to
 * @param  size   Size to read
 */
ZPL_DEF_INLINE b32 file_read( file* a_file, void* buffer, sw size );

/**
 * Writes to a a_file
 * @param  a_file
 * @param  buffer Buffer to read from
 * @param  size   Size to read
 */
ZPL_DEF_INLINE b32 file_write( file* a_file, void const* buffer, sw size );

typedef struct file_contents
{
	allocator allocator;
	void*     data;
	sw        size;
} file_contents;

/**
 * Reads the whole a_file contents
 * @param  a              Allocator to use
 * @param  zero_terminate End the read data with null terminator
 * @param  filepath       Path to the a_file
 * @return                File contents data
 */
ZPL_DEF file_contents file_read_contents( allocator a, b32 zero_terminate, char const* filepath );

/**
 * Frees the a_file content data previously read
 * @param  fc
 */
ZPL_DEF void file_free_contents( file_contents* fc );

/**
 * Writes content to a a_file
 */
ZPL_DEF b32 file_write_contents( char const* filepath, void const* buffer, sw size, file_error* err );

/**
 * Reads the a_file as array of lines
 *
 * Make sure you free both the returned buffer and the lines (zpl_array)
 * @param  a_allocator            Allocator to use
 * @param  lines            Reference to zpl_array container we store lines to
 * @param  filename         Path to the a_file
 * @param  strip_whitespace Strip whitespace when we split to lines?
 * @return                  File content we've read itself
 */
ZPL_DEF char* file_read_lines( allocator a_allocator, zpl_array( char* ) * lines, char const* filename, b32 strip_whitespace );

//! @}

/* inlines */


ZPL_IMPL_INLINE b32 file_read_at_check( file* f, void* buffer, sw size, s64 offset, sw* bytes_read )
{
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	return f->ops.read_at( f->fd, buffer, size, offset, bytes_read, false );
}

ZPL_IMPL_INLINE b32 file_write_at_check( file* f, void const* buffer, sw size, s64 offset, sw* bytes_written )
{
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	return f->ops.write_at( f->fd, buffer, size, offset, bytes_written );
}

ZPL_IMPL_INLINE b32 file_read_at( file* f, void* buffer, sw size, s64 offset )
{
	return file_read_at_check( f, buffer, size, offset, NULL );
}

ZPL_IMPL_INLINE b32 file_write_at( file* f, void const* buffer, sw size, s64 offset )
{
	return file_write_at_check( f, buffer, size, offset, NULL );
}

ZPL_IMPL_INLINE s64 file_seek( file* f, s64 offset )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, offset, ZPL_SEEK_WHENCE_BEGIN, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE s64 file_seek_to_end( file* f )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, 0, ZPL_SEEK_WHENCE_END, &new_offset );
	return new_offset;
}

// NOTE: Skips a certain amount of bytes
ZPL_IMPL_INLINE s64 file_skip( file* f, s64 bytes )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, bytes, ZPL_SEEK_WHENCE_CURRENT, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE s64 file_tell( file* f )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, 0, ZPL_SEEK_WHENCE_CURRENT, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE b32 file_read( file* f, void* buffer, sw size )
{
	s64 cur_offset = file_tell( f );
	b32 result     = file_read_at( f, buffer, size, file_tell( f ) );
	file_seek( f, cur_offset + size );
	return result;
}

ZPL_IMPL_INLINE b32 file_write( file* f, void const* buffer, sw size )
{
	s64 cur_offset = file_tell( f );
	b32 result     = file_write_at( f, buffer, size, file_tell( f ) );
	file_seek( f, cur_offset + size );
	return result;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
