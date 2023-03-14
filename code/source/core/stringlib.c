// a_file: source/core/stringlib.c


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

string string_make_reserve( allocator a, sw capacity )
{
	sw    header_size = size_of( string_header );
	void* ptr         = alloc( a, header_size + capacity + 1 );

	string         str;
	string_header* header;

	if ( ptr == NULL )
		return NULL;
	zero_size( ptr, header_size + capacity + 1 );

	str               = zpl_cast( char* ) ptr + header_size;
	header            = ZPL_STRING_HEADER( str );
	header->allocator = a;
	header->length    = 0;
	header->capacity  = capacity;
	str[ capacity ]   = '\0';

	return str;
}

string string_make_length( allocator a, void const* init_str, sw num_bytes )
{
	sw    header_size = size_of( string_header );
	void* ptr         = alloc( a, header_size + num_bytes + 1 );

	string         str;
	string_header* header;

	if ( ptr == NULL )
		return NULL;
	if ( ! init_str )
		zero_size( ptr, header_size + num_bytes + 1 );

	str               = zpl_cast( char* ) ptr + header_size;
	header            = ZPL_STRING_HEADER( str );
	header->allocator = a;
	header->length    = num_bytes;
	header->capacity  = num_bytes;
	if ( num_bytes && init_str )
		memcopy( str, init_str, num_bytes );
	str[ num_bytes ] = '\0';

	return str;
}

string string_sprintf_buf( allocator a, const char* fmt, ... )
{
	local_persist thread_local char buf[ ZPL_PRINTF_MAXLEN ] = { 0 };
	va_list                         va;
	va_start( va, fmt );
	snprintf_va( buf, ZPL_PRINTF_MAXLEN, fmt, va );
	va_end( va );

	return string_make( a, buf );
}

string string_sprintf( allocator a, char* buf, sw num_bytes, const char* fmt, ... )
{
	va_list va;
	va_start( va, fmt );
	snprintf_va( buf, num_bytes, fmt, va );
	va_end( va );

	return string_make( a, buf );
}

string string_append_length( string str, void const* other, sw other_len )
{
	if ( other_len > 0 )
	{
		sw curr_len = string_length( str );

		str = string_make_space_for( str, other_len );
		if ( str == NULL )
			return NULL;

		memcopy( str + curr_len, other, other_len );
		str[ curr_len + other_len ] = '\0';
		zpl__set_string_length( str, curr_len + other_len );
	}
	return str;
}

ZPL_ALWAYS_INLINE string string_appendc( string str, const char* other )
{
	return string_append_length( str, other, strlen( other ) );
}

ZPL_ALWAYS_INLINE string string_join( allocator a, const char** parts, sw count, const char* glue )
{
	string ret;
	sw     i;

	ret = string_make( a, NULL );

	for ( i = 0; i < count; ++i )
	{
		ret = string_appendc( ret, parts[ i ] );

		if ( ( i + 1 ) < count )
		{
			ret = string_appendc( ret, glue );
		}
	}

	return ret;
}

string string_set( string str, const char* cstr )
{
	sw len = strlen( cstr );
	if ( string_capacity( str ) < len )
	{
		str = string_make_space_for( str, len - string_length( str ) );
		if ( str == NULL )
			return NULL;
	}

	memcopy( str, cstr, len );
	str[ len ] = '\0';
	zpl__set_string_length( str, len );

	return str;
}

string string_make_space_for( string str, sw add_len )
{
	sw available = string_available_space( str );

	// NOTE: Return if there is enough space left
	if ( available >= add_len )
	{
		return str;
	}
	else
	{
		sw             new_len, old_size, new_size;
		void *         ptr, *new_ptr;
		allocator      a = ZPL_STRING_HEADER( str )->allocator;
		string_header* header;

		new_len  = string_length( str ) + add_len;
		ptr      = ZPL_STRING_HEADER( str );
		old_size = size_of( string_header ) + string_length( str ) + 1;
		new_size = size_of( string_header ) + new_len + 1;

		new_ptr = resize( a, ptr, old_size, new_size );
		if ( new_ptr == NULL )
			return NULL;

		header            = zpl_cast( string_header* ) new_ptr;
		header->allocator = a;

		str = zpl_cast( string )( header + 1 );
		zpl__set_string_capacity( str, new_len );

		return str;
	}
}

sw string_allocation_size( string const str )
{
	sw cap = string_capacity( str );
	return size_of( string_header ) + cap;
}

b32 string_are_equal( string const lhs, string const rhs )
{
	sw lhs_len, rhs_len, i;
	lhs_len = string_length( lhs );
	rhs_len = string_length( rhs );
	if ( lhs_len != rhs_len )
		return false;

	for ( i = 0; i < lhs_len; i++ )
	{
		if ( lhs[ i ] != rhs[ i ] )
			return false;
	}

	return true;
}

string string_trim( string str, const char* cut_set )
{
	char *start, *end, *start_pos, *end_pos;
	sw    len;

	start_pos = start = str;
	end_pos = end = str + string_length( str ) - 1;

	while ( start_pos <= end && char_first_occurence( cut_set, *start_pos ) )
		start_pos++;
	while ( end_pos > start_pos && char_first_occurence( cut_set, *end_pos ) )
		end_pos--;

	len = zpl_cast( sw )( ( start_pos > end_pos ) ? 0 : ( ( end_pos - start_pos ) + 1 ) );

	if ( str != start_pos )
		memmove( str, start_pos, len );
	str[ len ] = '\0';

	zpl__set_string_length( str, len );

	return str;
}

string string_append_rune( string str, rune r )
{
	if ( r >= 0 )
	{
		u8 buf[ 8 ] = { 0 };
		sw len      = utf8_encode_rune( buf, r );
		return string_append_length( str, buf, len );
	}

	return str;
}

string string_append_fmt( string str, const char* fmt, ... )
{
	sw      res;
	char    buf[ ZPL_PRINTF_MAXLEN ] = { 0 };
	va_list va;
	va_start( va, fmt );
	res = snprintf_va( buf, count_of( buf ) - 1, fmt, va ) - 1;
	va_end( va );
	return string_append_length( str, buf, res );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
