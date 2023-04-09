// file: source/core/file_tar.c



ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct
{
	char name[ 100 ];
	char mode[ 8 ];
	char owner[ 8 ];
	char group[ 8 ];
	char size[ 12 ];
	char mtime[ 12 ];
	char checksum[ 8 ];
	char type;
	char linkname[ 100 ];
	char _padding[ 255 ];
} _tar_header;

internal uw _tar_checksum( _tar_header* hr )
{
	uw  i;
	uw  res = 256;
	u8* p   = zpl_cast( u8* )( hr );
	for ( i = 0; i < zpl_cast( uw ) offset_of( _tar_header, checksum ); i++ )
		res += p[ i ];
	for ( i = zpl_cast( uw ) offset_of( _tar_header, type ); i < zpl_cast( uw ) size_of( _tar_header ); i++ )
		res += p[ i ];
	return res;
}

internal b32 _tar_write_null( FileInfo* archive, sw cnt )
{
	char* out = bprintf( "%*r", cnt, '\0' );
	if ( ! file_write( archive, out, cnt ) )
		return 0;
	return 1;
}

sw tar_pack( FileInfo* archive, char const** paths, sw paths_len )
{
	ZPL_ASSERT_NOT_NULL( archive );
	ZPL_ASSERT_NOT_NULL( paths );

	for ( sw i = 0; i < paths_len; i++ )
	{
		ZPL_ASSERT_NOT_NULL( paths[ i ] );
		_tar_header hr = { 0 };
		FileInfo    file;
		FileError   ferr = file_open_mode( &file, EFileMode_READ, paths[ i ] );
		if ( ferr == EFileError_NOT_EXISTS )
		{
			return -( ZPL_TAR_ERROR_FILE_NOT_FOUND );
		}
		else if ( ferr != EFileError_NONE )
		{
			return -( ZPL_TAR_ERROR_IO_ERROR );
		}

		s64 file_size = file_size( &file );
		snprintf( hr.name, 12, "%s", paths[ i ] );
		snprintf( hr.size, 12, "%o", file_size );
		snprintf( hr.mode, 8, "%o", 0664 );
		snprintf( hr.mtime, 12, "%o", fs_last_write_time( paths[ i ] ) );
		hr.type = ZPL_TAR_TYPE_REGULAR;
		snprintf( hr.checksum, 8, "%o", _tar_checksum( &hr ) );

		file_write( archive, zpl_cast( void* )( &hr ), size_of( _tar_header ) );

		// write data
		{
			s64  remaining_data = file_size;
			s64  total_data     = align_forward_i64( remaining_data, 512 );
			s64  padding        = ( total_data - file_size );
			char buf[ 4096 ]    = { 0 };
			s64  pos            = 0;
			sw   bytes_read     = 0;
			do
			{
				if ( ! file_read_at_check( &file, buf, 4096, pos, &bytes_read ) )
				{
					file_close( &file );
					return -( ZPL_TAR_ERROR_IO_ERROR );
				}
				else if ( bytes_read == 0 )
				{
					break;
				}

				file_write( archive, buf, bytes_read );
				pos            += bytes_read;
				remaining_data -= bytes_read;
			} while ( remaining_data > 0 );

			if ( padding > 0 )
			{
				if ( ! _tar_write_null( archive, padding ) )
				{
					file_close( &file );
					return -( ZPL_TAR_ERROR_IO_ERROR );
				}
			}
		}

		file_close( &file );
	}

	if ( ! _tar_write_null( archive, size_of( _tar_header ) * 2 ) )
	{
		return -( ZPL_TAR_ERROR_IO_ERROR );
	}

	return 0;
}

sw tar_pack_dir( FileInfo* archive, char const* path, AllocatorInfo alloc )
{
	String       filelst = path_dirlist( alloc, path, true );
	char const** files   = zpl_cast( char const** ) str_split_lines( alloc, filelst, false );
	sw           err     = tar_pack( archive, files, array_count( files ) );
	string_free( filelst );
	array_free( files );
	return err;
}

sw tar_unpack( FileInfo* archive, tar_unpack_proc* unpack_proc, void* user_data )
{
	ZPL_ASSERT_NOT_NULL( archive );
	ZPL_ASSERT_NOT_NULL( unpack_proc );

	s64         pos = file_tell( archive );
	_tar_header hr  = { 0 };
	sw          err = ZPL_TAR_ERROR_NONE;

	do
	{
		if ( ! file_read( archive, zpl_cast( void* ) & hr, size_of( hr ) ) )
		{
			err = ZPL_TAR_ERROR_IO_ERROR;
			break;
		}
		else if ( *hr.checksum == 0 )
		{
			break;
		}
		pos = file_tell( archive );

		tar_record rec = { 0 };
		rec.type       = hr.type;
		rec.path       = hr.name;
		rec.offset     = pos;
		rec.length     = str_to_i64( hr.size, 0, 8 );
		rec.error      = ZPL_TAR_ERROR_NONE;

		uw checksum1 = zpl_cast( uw )( str_to_i64( hr.checksum, 0, 8 ) );
		uw checksum2 = _tar_checksum( &hr );
		rec.error    = ( checksum1 != checksum2 ) ? zpl_cast( sw ) ZPL_TAR_ERROR_BAD_CHECKSUM : rec.error;

		rec.error = unpack_proc( archive, &rec, user_data );

		if ( rec.error > 0 )
		{
			err = ZPL_TAR_ERROR_INTERRUPTED;
			break;
		}

		/* tar rounds files to 512 byte boundary */
		file_seek( archive, pos + align_forward_i64( rec.length, 512 ) );
	} while ( err == ZPL_TAR_ERROR_NONE );

	return -( err );
}

ZPL_TAR_UNPACK_PROC( tar_default_list_file )
{
	( void )archive;
	( void )user_data;
	if ( file->error != ZPL_TAR_ERROR_NONE )
		return 0; /* skip file */

	if ( file->type != ZPL_TAR_TYPE_REGULAR )
		return 0; /* we only care about regular files */

	/* proceed as usual */
	printf( "name: %s, offset: %d, length: %d\n", file->path, file->offset, file->length );
	return 0;
}

ZPL_TAR_UNPACK_PROC( tar_default_unpack_file )
{
	if ( file->error != ZPL_TAR_ERROR_NONE )
		return 0; /* skip file */

	if ( file->type != ZPL_TAR_TYPE_REGULAR )
		return 0; /* we only care about regular files */

	if ( ! str_compare( file->path, "..", 2 ) )
		return 0;

	char  tmp[ ZPL_MAX_PATH ] = { 0 };
	char* base_path           = zpl_cast( char* ) user_data;
	sw    base_len            = strlen( base_path );
	sw    len                 = strlen( file->path );
	ZPL_ASSERT( base_len + len - 2 < ZPL_MAX_PATH ); /* todo: account for missing leading path sep */

	strcpy( tmp, base_path );
	path_fix_slashes( tmp ); /* todo: need to do twice as base_path is checked before concat */

	if ( *tmp && tmp[ base_len - 1 ] != ZPL_PATH_SEPARATOR )
	{
		char sep[ 2 ] = { ZPL_PATH_SEPARATOR, 0 };
		strcat( tmp, sep );
	}
	strcat( tmp, file->path );
	path_fix_slashes( tmp );

	const char* last_slash = char_last_occurence( tmp, ZPL_PATH_SEPARATOR );

	if ( last_slash )
	{
		sw i     = zpl_cast( sw )( last_slash - tmp );
		tmp[ i ] = 0;
		path_mkdir_recursive( tmp, 0755 );
		tmp[ i ] = ZPL_PATH_SEPARATOR;
	}

	FileInfo f;
	file_create( &f, tmp );
	{
		char buf[ 4096 ]    = { 0 };
		sw   remaining_data = file->length;
		sw   bytes_read     = 0;
		s64  pos            = file->offset;
		do
		{
			if ( ! file_read_at_check( archive, buf, min( 4096, remaining_data ), pos, &bytes_read ) )
			{
				file_close( &f );
				return 1;
			}
			else if ( bytes_read == 0 )
			{
				break;
			}

			file_write( &f, buf, bytes_read );
			pos            += bytes_read;
			remaining_data -= bytes_read;
		} while ( remaining_data > 0 );
	}
	file_close( &f );
	return 0;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
