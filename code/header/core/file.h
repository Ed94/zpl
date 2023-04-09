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
	EFileMode_READ   = ZPL_BIT( 0 ),
	EFileMode_WRITE  = ZPL_BIT( 1 ),
	EFileMode_APPEND = ZPL_BIT( 2 ),
	EFileMode_RW     = ZPL_BIT( 3 ),
	ZPL_FILE_MODES   = EFileMode_READ | EFileMode_WRITE | EFileMode_APPEND | EFileMode_RW,
} file_mode_flag;

// NOTE: Only used internally and for the file operations
typedef enum seek_whence_type
{
	ZPL_SEEK_WHENCE_BEGIN   = 0,
	ZPL_SEEK_WHENCE_CURRENT = 1,
	ZPL_SEEK_WHENCE_END     = 2,
} seek_whence_type;

typedef enum FileError
{
	EFileError_NONE,
	EFileError_INVALID,
	EFileError_INVALID_FILENAME,
	EFileError_EXISTS,
	EFileError_NOT_EXISTS,
	EFileError_PERMISSION,
	EFileError_TRUNCATION_FAILURE,
	EFileError_NOT_EMPTY,
	EFileError_NAME_TOO_LONG,
	EFileError_UNKNOWN,
} FileError;

typedef union file_descriptor
{
	void* p;
	sptr  i;
	uptr  u;
} file_descriptor;

typedef struct file_operations file_operations;

#define ZPL_FILE_OPEN_PROC( name )     FileError name( file_descriptor* fd, file_operations* ops, file_mode mode, char const* filename )
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
	String buf;
} dir_info;

typedef struct FileInfo
{
	file_operations ops;
	file_descriptor fd;
	b32             is_temp;

	char const* filename;
	file_time   last_write_time;
	dir_entry*  dir;
} FileInfo;

typedef enum file_standard_type
{
	ZPL_FILE_STANDARD_INPUT,
	ZPL_FILE_STANDARD_OUTPUT,
	ZPL_FILE_STANDARD_ERROR,

	ZPL_FILE_STANDARD_COUNT,
} file_standard_type;

/**
 * Get standard file I/O.
 * @param  std Check zpl_file_standard_type
 * @return     File handle to standard I/O
 */
ZPL_DEF FileInfo* file_get_standard( file_standard_type std );

/**
 * Connects a system handle to a zpl file.
 * @param  file   Pointer to zpl file
 * @param  handle Low-level OS handle to connect
 */
ZPL_DEF void file_connect_handle( FileInfo* file, void* handle );

/**
 * Creates a new file
 * @param  file
 * @param  filename
 */
ZPL_DEF FileError file_create( FileInfo* file, char const* filename );

/**
 * Opens a file
 * @param  file
 * @param  filename
 */
ZPL_DEF FileError file_open( FileInfo* file, char const* filename );

/**
 * Opens a file using a specified mode
 * @param  file
 * @param  mode     Access mode to use
 * @param  filename
 */
ZPL_DEF FileError file_open_mode( FileInfo* file, file_mode mode, char const* filename );

/**
 * Constructs a new file from data
 * @param  file
 * @param  fd       Low-level file descriptor to use
 * @param  ops      File operations to rely upon
 * @param  filename
 */
ZPL_DEF FileError file_new( FileInfo* file, file_descriptor fd, file_operations ops, char const* filename );

/**
 * Returns a size of the file
 * @param  file
 * @return      File size
 */
ZPL_DEF s64 file_size( FileInfo* file );

/**
 * Returns the currently opened file's name
 * @param  file
 */
ZPL_DEF char const* file_name( FileInfo* file );

/**
 * Truncates the file by a specified size
 * @param  file
 * @param  size Size to truncate
 */
ZPL_DEF FileError file_truncate( FileInfo* file, s64 size );

/**
 * Checks whether a file's been changed since the last check
 * @param  file
 */
ZPL_DEF b32 file_has_changed( FileInfo* file );

/**
 * Retrieves a directory listing relative to the file
 * @param file
 */
ZPL_DEF void file_dirinfo_refresh( FileInfo* file );

/**
 * Creates a temporary file
 * @param  file
 */
FileError file_temp( FileInfo* file );

/**
 * Closes the file
 * @param  file
 */
ZPL_DEF FileError file_close( FileInfo* file );

/**
 * Reads file safely
 * @param  file
 * @param  buffer     Buffer to read to
 * @param  size       Size to read
 * @param  offset     Offset to read from
 * @param  bytes_read How much data we've actually read
 */
ZPL_DEF_INLINE b32 file_read_at_check( FileInfo* file, void* buffer, sw size, s64 offset, sw* bytes_read );

/**
 * Writes to file safely
 * @param  file
 * @param  buffer        Buffer to read from
 * @param  size          Size to write
 * @param  offset        Offset to write to
 * @param  bytes_written How much data we've actually written
 */
ZPL_DEF_INLINE b32 file_write_at_check( FileInfo* file, void const* buffer, sw size, s64 offset, sw* bytes_written );


/**
 * Reads file at a specific offset
 * @param  file
 * @param  buffer     Buffer to read to
 * @param  size       Size to read
 * @param  offset     Offset to read from
 * @param  bytes_read How much data we've actually read
 */
ZPL_DEF_INLINE b32 file_read_at( FileInfo* file, void* buffer, sw size, s64 offset );

/**
 * Writes to file at a specific offset
 * @param  file
 * @param  buffer        Buffer to read from
 * @param  size          Size to write
 * @param  offset        Offset to write to
 * @param  bytes_written How much data we've actually written
 */
ZPL_DEF_INLINE b32 file_write_at( FileInfo* file, void const* buffer, sw size, s64 offset );

/**
 * Seeks the file cursor from the beginning of file to a specific position
 * @param  file
 * @param  offset Offset to seek to
 */
ZPL_DEF_INLINE s64 file_seek( FileInfo* file, s64 offset );

/**
 * Seeks the file cursor to the end of the file
 * @param  file
 */
ZPL_DEF_INLINE s64 file_seek_to_end( FileInfo* file );

/**
 * Skips N bytes at the current position
 * @param  file
 * @param  bytes Bytes to skip
 */
ZPL_DEF_INLINE s64 file_skip( FileInfo* file, s64 bytes );    // NOTE: Skips a certain amount of bytes

/**
 * Returns the length from the beginning of the file we've read so far
 * @param  file
 * @return      Our current position in file
 */
ZPL_DEF_INLINE s64 file_tell( FileInfo* file );

/**
 * Reads from a file
 * @param  file
 * @param  buffer Buffer to read to
 * @param  size   Size to read
 */
ZPL_DEF_INLINE b32 file_read( FileInfo* file, void* buffer, sw size );

/**
 * Writes to a file
 * @param  file
 * @param  buffer Buffer to read from
 * @param  size   Size to read
 */
ZPL_DEF_INLINE b32 file_write( FileInfo* file, void const* buffer, sw size );

typedef struct file_contents
{
	AllocatorInfo allocator;
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
ZPL_DEF file_contents file_read_contents( AllocatorInfo a, b32 zero_terminate, char const* filepath );

/**
 * Frees the file content data previously read
 * @param  fc
 */
ZPL_DEF void file_free_contents( file_contents* fc );

/**
 * Writes content to a file
 */
ZPL_DEF b32 file_write_contents( char const* filepath, void const* buffer, sw size, FileError* err );

/**
 * Reads the file as array of lines
 *
 * Make sure you free both the returned buffer and the lines (zpl_array)
 * @param  alloc            Allocator to use
 * @param  lines            Reference to zpl_array container we store lines to
 * @param  filename         Path to the file
 * @param  strip_whitespace Strip whitespace when we split to lines?
 * @return                  File content we've read itself
 */
ZPL_DEF char* file_read_lines( AllocatorInfo alloc, Array( char* ) * lines, char const* filename, b32 strip_whitespace );

//! @}

/* inlines */


ZPL_IMPL_INLINE b32 file_read_at_check( FileInfo* f, void* buffer, sw size, s64 offset, sw* bytes_read )
{
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	return f->ops.read_at( f->fd, buffer, size, offset, bytes_read, false );
}

ZPL_IMPL_INLINE b32 file_write_at_check( FileInfo* f, void const* buffer, sw size, s64 offset, sw* bytes_written )
{
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	return f->ops.write_at( f->fd, buffer, size, offset, bytes_written );
}

ZPL_IMPL_INLINE b32 file_read_at( FileInfo* f, void* buffer, sw size, s64 offset )
{
	return file_read_at_check( f, buffer, size, offset, NULL );
}

ZPL_IMPL_INLINE b32 file_write_at( FileInfo* f, void const* buffer, sw size, s64 offset )
{
	return file_write_at_check( f, buffer, size, offset, NULL );
}

ZPL_IMPL_INLINE s64 file_seek( FileInfo* f, s64 offset )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, offset, ZPL_SEEK_WHENCE_BEGIN, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE s64 file_seek_to_end( FileInfo* f )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, 0, ZPL_SEEK_WHENCE_END, &new_offset );
	return new_offset;
}

// NOTE: Skips a certain amount of bytes
ZPL_IMPL_INLINE s64 file_skip( FileInfo* f, s64 bytes )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, bytes, ZPL_SEEK_WHENCE_CURRENT, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE s64 file_tell( FileInfo* f )
{
	s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = default_file_operations;
	f->ops.seek( f->fd, 0, ZPL_SEEK_WHENCE_CURRENT, &new_offset );
	return new_offset;
}

ZPL_IMPL_INLINE b32 file_read( FileInfo* f, void* buffer, sw size )
{
	s64 cur_offset = file_tell( f );
	b32 result     = file_read_at( f, buffer, size, file_tell( f ) );
	file_seek( f, cur_offset + size );
	return result;
}

ZPL_IMPL_INLINE b32 file_write( FileInfo* f, void const* buffer, sw size )
{
	s64 cur_offset = file_tell( f );
	b32 result     = file_write_at( f, buffer, size, file_tell( f ) );
	file_seek( f, cur_offset + size );
	return result;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
