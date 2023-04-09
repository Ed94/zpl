// file: source/core/file_stream.c


////////////////////////////////////////////////////////////////
//
// Memory streaming
//
//

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct
{
	u8            magic;
	u8*           buf;    //< zpl_array OR plain buffer if we can't write
	sw            cursor;
	AllocatorInfo alloc;

	file_stream_flags flags;
	sw                cap;
} _memory_fd;

#define ZPL__FILE_STREAM_FD_MAGIC 37

ZPL_DEF_INLINE file_descriptor _file_stream_fd_make( _memory_fd* d );
ZPL_DEF_INLINE _memory_fd*     _file_stream_from_fd( file_descriptor fd );

ZPL_IMPL_INLINE file_descriptor _file_stream_fd_make( _memory_fd* d )
{
	file_descriptor fd = { 0 };
	fd.p               = ( void* )d;
	return fd;
}

ZPL_IMPL_INLINE _memory_fd* _file_stream_from_fd( file_descriptor fd )
{
	_memory_fd* d = ( _memory_fd* )fd.p;
	ZPL_ASSERT( d->magic == ZPL__FILE_STREAM_FD_MAGIC );
	return d;
}

b8 file_stream_new( FileInfo* file, AllocatorInfo allocator )
{
	ZPL_ASSERT_NOT_NULL( file );
	_memory_fd* d = ( _memory_fd* )alloc( allocator, size_of( _memory_fd ) );
	if ( ! d )
		return false;
	zero_item( file );
	d->magic = ZPL__FILE_STREAM_FD_MAGIC;
	d->alloc = allocator;
	d->flags = ZPL_FILE_STREAM_CLONE_WRITABLE;
	d->cap   = 0;
	if ( ! array_init( d->buf, allocator ) )
		return false;
	file->ops             = memory_file_operations;
	file->fd              = _file_stream_fd_make( d );
	file->dir             = NULL;
	file->last_write_time = 0;
	file->filename        = NULL;
	file->is_temp         = true;
	return true;
}

b8 file_stream_open( FileInfo* file, AllocatorInfo allocator, u8* buffer, sw size, file_stream_flags flags )
{
	ZPL_ASSERT_NOT_NULL( file );
	_memory_fd* d = ( _memory_fd* )alloc( allocator, size_of( _memory_fd ) );
	if ( ! d )
		return false;
	zero_item( file );
	d->magic = ZPL__FILE_STREAM_FD_MAGIC;
	d->alloc = allocator;
	d->flags = flags;
	if ( d->flags & ZPL_FILE_STREAM_CLONE_WRITABLE )
	{
		if ( ! array_init_reserve( d->buf, allocator, size ) )
			return false;
		memcopy( d->buf, buffer, size );
		d->cap = array_count( d->buf ) = size;
	}
	else
	{
		d->buf = buffer;
		d->cap = size;
	}
	file->ops             = memory_file_operations;
	file->fd              = _file_stream_fd_make( d );
	file->dir             = NULL;
	file->last_write_time = 0;
	file->filename        = NULL;
	file->is_temp         = true;
	return true;
}

u8* file_stream_buf( FileInfo* file, sw* size )
{
	ZPL_ASSERT_NOT_NULL( file );
	_memory_fd* d = _file_stream_from_fd( file->fd );
	if ( size )
		*size = d->cap;
	return d->buf;
}

internal ZPL_FILE_SEEK_PROC( _memory_file_seek )
{
	_memory_fd* d      = _file_stream_from_fd( fd );
	sw          buflen = d->cap;

	if ( whence == ZPL_SEEK_WHENCE_BEGIN )
		d->cursor = 0;
	else if ( whence == ZPL_SEEK_WHENCE_END )
		d->cursor = buflen;

	d->cursor = max( 0, clamp( d->cursor + offset, 0, buflen ) );
	if ( new_offset )
		*new_offset = d->cursor;
	return true;
}

internal ZPL_FILE_READ_AT_PROC( _memory_file_read )
{
	unused( stop_at_newline );
	_memory_fd* d = _file_stream_from_fd( fd );
	memcopy( buffer, d->buf + offset, size );
	if ( bytes_read )
		*bytes_read = size;
	return true;
}

internal ZPL_FILE_WRITE_AT_PROC( _memory_file_write )
{
	_memory_fd* d = _file_stream_from_fd( fd );
	if ( ! ( d->flags & ( ZPL_FILE_STREAM_CLONE_WRITABLE | ZPL_FILE_STREAM_WRITABLE ) ) )
		return false;
	sw buflen   = d->cap;
	sw extralen = max( 0, size - ( buflen - offset ) );
	sw rwlen    = size - extralen;
	sw new_cap  = buflen + extralen;
	if ( d->flags & ZPL_FILE_STREAM_CLONE_WRITABLE )
	{
		if ( array_capacity( d->buf ) < new_cap )
		{
			if ( ! array_grow( d->buf, ( s64 )( new_cap ) ) )
				return false;
		}
	}
	memcopy( d->buf + offset, buffer, rwlen );

	if ( ( d->flags & ZPL_FILE_STREAM_CLONE_WRITABLE ) && extralen > 0 )
	{
		memcopy( d->buf + offset + rwlen, ptr_add_const( buffer, rwlen ), extralen );
		d->cap = array_count( d->buf ) = new_cap;
	}
	else
	{
		extralen = 0;
	}

	if ( bytes_written )
		*bytes_written = ( rwlen + extralen );
	return true;
}

internal ZPL_FILE_CLOSE_PROC( _memory_file_close )
{
	_memory_fd*   d     = _file_stream_from_fd( fd );
	AllocatorInfo alloc = d->alloc;
	if ( d->flags & ZPL_FILE_STREAM_CLONE_WRITABLE )
		array_free( d->buf );
	free( alloc, d );
}

file_operations const memory_file_operations = { _memory_file_read, _memory_file_write, _memory_file_seek, _memory_file_close };

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
