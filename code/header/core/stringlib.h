// a_file: header/core/stringlib.h


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef char* string;

typedef struct string_header
{
	allocator allocator;
	sw        length;
	sw        capacity;
} string_header;

#define ZPL_STRING_HEADER( str ) ( zpl_cast( ZPL_NS string_header* )( str ) - 1 )

ZPL_DEF string string_make_reserve( allocator a, sw capacity );
ZPL_DEF string string_make_length( allocator a, void const* str, sw num_bytes );
ZPL_DEF string string_sprintf( allocator a, char* buf, sw num_bytes, const char* fmt, ... );
ZPL_DEF string string_sprintf_buf( allocator a, const char* fmt, ... );    // NOTE: Uses locally persistent buffer
ZPL_DEF string string_append_length( string str, void const* other, sw num_bytes );
ZPL_DEF string string_appendc( string str, const char* other );
ZPL_DEF string string_join( allocator a, const char** parts, sw count, const char* glue );
ZPL_DEF string string_set( string str, const char* cstr );
ZPL_DEF string string_make_space_for( string str, sw add_len );
ZPL_DEF sw     string_allocation_size( string const str );
ZPL_DEF b32    string_are_equal( string const lhs, string const rhs );
ZPL_DEF string string_trim( string str, const char* cut_set );
ZPL_DEF string string_append_rune( string str, rune r );
ZPL_DEF string string_append_fmt( string str, const char* fmt, ... );

ZPL_DEF_INLINE string string_make( allocator a, const char* str );
ZPL_DEF_INLINE void   string_free( string str );
ZPL_DEF_INLINE void   string_clear( string str );
ZPL_DEF_INLINE string string_duplicate( allocator a, string const str );
ZPL_DEF_INLINE sw     string_length( string const str );
ZPL_DEF_INLINE sw     string_capacity( string const str );
ZPL_DEF_INLINE sw     string_available_space( string const str );
ZPL_DEF_INLINE string string_append( string str, string const other );
ZPL_DEF_INLINE string string_trim_space( string str );    // Whitespace ` \t\r\n\v\f`
ZPL_DEF_INLINE void   zpl__set_string_length( string str, sw len );
ZPL_DEF_INLINE void   zpl__set_string_capacity( string str, sw cap );

ZPL_IMPL_INLINE void zpl__set_string_length( string str, sw len )
{
	ZPL_STRING_HEADER( str )->length = len;
}

ZPL_IMPL_INLINE void zpl__set_string_capacity( string str, sw cap )
{
	ZPL_STRING_HEADER( str )->capacity = cap;
}

ZPL_IMPL_INLINE string string_make( allocator a, const char* str )
{
	sw len = str ? strlen( str ) : 0;
	return string_make_length( a, str, len );
}

ZPL_IMPL_INLINE void string_free( string str )
{
	if ( str )
	{
		string_header* header = ZPL_STRING_HEADER( str );
		free( header->allocator, header );
	}
}

ZPL_IMPL_INLINE string string_duplicate( allocator a, string const str )
{
	return string_make_length( a, str, string_length( str ) );
}

ZPL_IMPL_INLINE sw string_length( string const str )
{
	return ZPL_STRING_HEADER( str )->length;
}

ZPL_IMPL_INLINE sw string_capacity( string const str )
{
	return ZPL_STRING_HEADER( str )->capacity;
}

ZPL_IMPL_INLINE sw string_available_space( string const str )
{
	string_header* h = ZPL_STRING_HEADER( str );
	if ( h->capacity > h->length )
		return h->capacity - h->length;
	return 0;
}

ZPL_IMPL_INLINE void string_clear( string str )
{
	zpl__set_string_length( str, 0 );
	str[ 0 ] = '\0';
}

ZPL_IMPL_INLINE string string_append( string str, string const other )
{
	return string_append_length( str, other, string_length( other ) );
}

ZPL_IMPL_INLINE string string_trim_space( string str )
{
	return string_trim( str, " \t\r\n\v\f" );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
