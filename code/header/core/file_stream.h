// file: header/core/file_stream.h

/** @file file_stream.c
@brief File stream
@defgroup fileio File stream

File streaming operations on memory.

@{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef enum
{
	/* Allows us to write to the buffer directly. Beware: you can not append a new data! */
	ZPL_FILE_STREAM_WRITABLE = ZPL_BIT( 0 ),

	/* Clones the input buffer so you can write (file_write*) data into it. */
	/* Since we work with a clone, the buffer size can dynamically grow as well. */
	ZPL_FILE_STREAM_CLONE_WRITABLE = ZPL_BIT( 1 ),
} file_stream_flags;

/**
 * Opens a new memory stream
 * @param file
 * @param allocator
 */
ZPL_DEF b8 file_stream_new( zpl_file* file, zpl_allocator allocator );

/**
 * Opens a memory stream over an existing buffer
 * @param  file
 * @param  allocator
 * @param  buffer   Memory to create stream from
 * @param  size     Buffer's size
 * @param  flags
 */
ZPL_DEF b8 file_stream_open( zpl_file* file, zpl_allocator allocator, u8* buffer, sw size, file_stream_flags flags );

/**
 * Retrieves the stream's underlying buffer and buffer size.
 * @param file memory stream
 * @param size (Optional) buffer size
 */
ZPL_DEF u8* file_stream_buf( zpl_file* file, sw* size );

extern file_operations const memory_file_operations;

//! @}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
