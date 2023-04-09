// file: header/core/stringlib.h


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef char* String;

typedef struct StringHeader
{
	AllocatorInfo allocator;
	sw            length;
	sw            capacity;
} StringHeader;

#define ZPL_STRING_HEADER( str ) ( zpl_cast( ZPL_NS( StringHeader )* )( str ) - 1 )

ZPL_DEF String string_make_reserve( AllocatorInfo a, sw capacity );
ZPL_DEF String string_make_length( AllocatorInfo a, void const* str, sw num_bytes );
ZPL_DEF String string_sprintf( AllocatorInfo a, char* buf, sw num_bytes, const char* fmt, ... );
ZPL_DEF String string_sprintf_buf( AllocatorInfo a, const char* fmt, ... );    // NOTE: Uses locally persistent buffer
ZPL_DEF String string_append_length( String str, void const* other, sw num_bytes );
ZPL_DEF String string_appendc( String str, const char* other );
ZPL_DEF String string_join( AllocatorInfo a, const char** parts, sw count, const char* glue );
ZPL_DEF String string_set( String str, const char* cstr );
ZPL_DEF String string_make_space_for( String str, sw add_len );
ZPL_DEF sw     string_allocation_size( String const str );
ZPL_DEF b32    string_are_equal( String const lhs, String const rhs );
ZPL_DEF String string_trim( String str, const char* cut_set );
ZPL_DEF String string_append_rune( String str, rune r );
ZPL_DEF String string_append_fmt( String str, const char* fmt, ... );

ZPL_DEF_INLINE String string_make( AllocatorInfo a, const char* str );
ZPL_DEF_INLINE void   string_free( String str );
ZPL_DEF_INLINE void   string_clear( String str );
ZPL_DEF_INLINE String string_duplicate( AllocatorInfo a, String const str );
ZPL_DEF_INLINE sw     string_length( String const str );
ZPL_DEF_INLINE sw     string_capacity( String const str );
ZPL_DEF_INLINE sw     string_available_space( String const str );
ZPL_DEF_INLINE String string_append( String str, String const other );
ZPL_DEF_INLINE String string_trim_space( String str );    // Whitespace ` \t\r\n\v\f`
ZPL_DEF_INLINE void   _set_string_length( String str, sw len );
ZPL_DEF_INLINE void   _set_string_capacity( String str, sw cap );

ZPL_IMPL_INLINE void _set_string_length( String str, sw len )
{
	ZPL_STRING_HEADER( str )->length = len;
}

ZPL_IMPL_INLINE void _set_string_capacity( String str, sw cap )
{
	ZPL_STRING_HEADER( str )->capacity = cap;
}

ZPL_IMPL_INLINE String string_make( AllocatorInfo a, const char* str )
{
	sw len = str ? strlen( str ) : 0;
	return string_make_length( a, str, len );
}

ZPL_IMPL_INLINE void string_free( String str )
{
	if ( str )
	{
		StringHeader* header = ZPL_STRING_HEADER( str );
		free( header->allocator, header );
	}
}

ZPL_IMPL_INLINE String string_duplicate( AllocatorInfo a, String const str )
{
	return string_make_length( a, str, string_length( str ) );
}

ZPL_IMPL_INLINE sw string_length( String const str )
{
	return ZPL_STRING_HEADER( str )->length;
}

ZPL_IMPL_INLINE sw string_capacity( String const str )
{
	return ZPL_STRING_HEADER( str )->capacity;
}

ZPL_IMPL_INLINE sw string_available_space( String const str )
{
	StringHeader* h = ZPL_STRING_HEADER( str );
	if ( h->capacity > h->length )
		return h->capacity - h->length;
	return 0;
}

ZPL_IMPL_INLINE void string_clear( String str )
{
	_set_string_length( str, 0 );
	str[ 0 ] = '\0';
}

ZPL_IMPL_INLINE String string_append( String str, String const other )
{
	return string_append_length( str, other, string_length( other ) );
}

ZPL_IMPL_INLINE String string_trim_space( String str )
{
	return string_trim( str, " \t\r\n\v\f" );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
