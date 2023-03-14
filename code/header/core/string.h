// a_file: header/core/string.h

/** @a_file string.c
@brief String operations and library
@defgroup string String library

Offers methods for c-string manipulation, but also a string library based on gb_string, which is c-string friendly.

@{
*/

////////////////////////////////////////////////////////////////
//
// Char Functions
//
//


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

ZPL_DEF_INLINE char char_to_lower( char c );
ZPL_DEF_INLINE char char_to_upper( char c );
ZPL_DEF_INLINE b32  char_is_space( char c );
ZPL_DEF_INLINE b32  char_is_digit( char c );
ZPL_DEF_INLINE b32  char_is_hex_digit( char c );
ZPL_DEF_INLINE b32  char_is_alpha( char c );
ZPL_DEF_INLINE b32  char_is_alphanumeric( char c );
ZPL_DEF_INLINE s32  digit_to_int( char c );
ZPL_DEF_INLINE s32  hex_digit_to_int( char c );
ZPL_DEF_INLINE u8   char_to_hex_digit( char c );
ZPL_DEF_INLINE b32  char_is_control( char c );

// NOTE: ASCII only
ZPL_DEF_INLINE void str_to_lower( char* str );
ZPL_DEF_INLINE void str_to_upper( char* str );

ZPL_DEF_INLINE char const* str_trim( char const* str, b32 catch_newline );
ZPL_DEF_INLINE char const* str_skip( char const* str, char c );
ZPL_DEF_INLINE char const* str_skip_any( char const* str, char const* char_list );
ZPL_DEF_INLINE char const* str_skip_literal( char const* str, char c );
ZPL_DEF_INLINE char const* str_control_skip( char const* str, char c );

ZPL_DEF_INLINE sw          strlen( const char* str );
ZPL_DEF_INLINE sw          strnlen( const char* str, sw max_len );
ZPL_DEF_INLINE s32         str_compare( const char* s1, const char* s2 );
ZPL_DEF_INLINE s32         str_compare( const char* s1, const char* s2, sw len );
ZPL_DEF_INLINE char*       strcpy( char* dest, const char* source );
ZPL_DEF_INLINE char*       strcat( char* dest, const char* source );
ZPL_DEF_INLINE char*       strncpy( char* dest, const char* source, sw len );
ZPL_DEF_INLINE sw          strlcpy( char* dest, const char* source, sw len );
ZPL_DEF_INLINE char*       strrev( char* str );    // NOTE: ASCII only
ZPL_DEF_INLINE const char* strtok( char* output, const char* src, const char* delimit );
ZPL_DEF_INLINE const char* strntok( char* output, sw len, const char* src, const char* delimit );

ZPL_DEF_INLINE char*  strdup( allocator a, char* src, sw max_len );
ZPL_DEF_INLINE char** str_split_lines( allocator a_allocator, char* source, b32 strip_whitespace );

#define str_expand( str ) str, ZPL_NS strlen( str )
#define str_advance_while( str, cond )                                                                                                                                             \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		++str;                                                                                                                                                                     \
	} while ( ( cond ) );

ZPL_DEF_INLINE b32 str_has_prefix( const char* str, const char* prefix );
ZPL_DEF_INLINE b32 str_has_suffix( const char* str, const char* suffix );

ZPL_DEF_INLINE const char* char_first_occurence( const char* str, char c );
ZPL_DEF_INLINE const char* char_last_occurence( const char* str, char c );
#define strchr ZPL_NS char_first_occurence

ZPL_DEF_INLINE void str_concat( char* dest, sw dest_len, const char* src_a, sw src_a_len, const char* src_b, sw src_b_len );

ZPL_DEF u64  str_to_u64( const char* str, char** end_ptr, s32 base );    // TODO: Support more than just decimal and hexadecimal
ZPL_DEF s64  str_to_i64( const char* str, char** end_ptr, s32 base );    // TODO: Support more than just decimal and hexadecimal
ZPL_DEF f64  str_to_f64( const char* str, char** end_ptr );
ZPL_DEF void i64_to_str( s64 value, char* string, s32 base );
ZPL_DEF void u64_to_str( u64 value, char* string, s32 base );

ZPL_DEF_INLINE f32 str_to_f32( const char* str, char** end_ptr );

////////////////////////////////////////////////////////////////
//
// UTF-8 Handling
//
//

// NOTE: Does not check if utf-8 string is valid
ZPL_IMPL_INLINE sw utf8_strlen( u8 const* str );
ZPL_IMPL_INLINE sw utf8_strnlen( u8 const* str, sw max_len );

// NOTE: Windows doesn't handle 8 bit filenames well
ZPL_DEF u16* utf8_to_ucs2( u16* buffer, sw len, u8 const* str );
ZPL_DEF u8*  ucs2_to_utf8( u8* buffer, sw len, u16 const* str );
ZPL_DEF u16* utf8_to_ucs2_buf( u8 const* str );     // NOTE: Uses locally persisting buffer
ZPL_DEF u8*  ucs2_to_utf8_buf( u16 const* str );    // NOTE: Uses locally persisting buffer

// NOTE: Returns size of codepoint in bytes
ZPL_DEF sw utf8_decode( u8 const* str, sw str_len, rune* codepoint );
ZPL_DEF sw utf8_codepoint_size( u8 const* str, sw str_len );
ZPL_DEF sw utf8_encode_rune( u8 buf[ 4 ], rune r );

/* inlines */

ZPL_IMPL_INLINE char char_to_lower( char c )
{
	if ( c >= 'A' && c <= 'Z' )
		return 'a' + ( c - 'A' );
	return c;
}

ZPL_IMPL_INLINE char char_to_upper( char c )
{
	if ( c >= 'a' && c <= 'z' )
		return 'A' + ( c - 'a' );
	return c;
}

ZPL_IMPL_INLINE b32 char_is_space( char c )
{
	if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v' )
		return true;
	return false;
}

ZPL_IMPL_INLINE b32 char_is_digit( char c )
{
	if ( c >= '0' && c <= '9' )
		return true;
	return false;
}

ZPL_IMPL_INLINE b32 char_is_hex_digit( char c )
{
	if ( char_is_digit( c ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) )
		return true;
	return false;
}

ZPL_IMPL_INLINE b32 char_is_alpha( char c )
{
	if ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) )
		return true;
	return false;
}

ZPL_IMPL_INLINE b32 char_is_alphanumeric( char c )
{
	return char_is_alpha( c ) || char_is_digit( c );
}

ZPL_IMPL_INLINE s32 digit_to_int( char c )
{
	return char_is_digit( c ) ? c - '0' : c - 'W';
}

ZPL_IMPL_INLINE s32 hex_digit_to_int( char c )
{
	if ( char_is_digit( c ) )
		return digit_to_int( c );
	else if ( is_between( c, 'a', 'f' ) )
		return c - 'a' + 10;
	else if ( is_between( c, 'A', 'F' ) )
		return c - 'A' + 10;
	return -1;
}

ZPL_IMPL_INLINE u8 char_to_hex_digit( char c )
{
	if ( c >= '0' && c <= '9' )
		return ( u8 )( c - '0' );
	if ( c >= 'a' && c <= 'f' )
		return ( u8 )( c - 'a' );
	if ( c >= 'A' && c <= 'F' )
		return ( u8 )( c - 'A' );
	return 0;
}

ZPL_IMPL_INLINE void str_to_lower( char* str )
{
	if ( ! str )
		return;
	while ( *str )
	{
		*str = char_to_lower( *str );
		str++;
	}
}

ZPL_IMPL_INLINE void str_to_upper( char* str )
{
	if ( ! str )
		return;
	while ( *str )
	{
		*str = char_to_upper( *str );
		str++;
	}
}

ZPL_IMPL_INLINE sw strlen( const char* str )
{
	if ( str == NULL )
	{
		return 0;
	}
	const char* p = str;
	while ( *str )
		str++;
	return str - p;
}

ZPL_IMPL_INLINE sw strnlen( const char* str, sw max_len )
{
	const char* end = zpl_cast( const char* ) memchr( str, 0, max_len );
	if ( end )
		return end - str;
	return max_len;
}

ZPL_IMPL_INLINE sw utf8_strlen( u8 const* str )
{
	sw count = 0;
	for ( ; *str; count++ )
	{
		u8 c   = *str;
		sw inc = 0;
		if ( c < 0x80 )
			inc = 1;
		else if ( ( c & 0xe0 ) == 0xc0 )
			inc = 2;
		else if ( ( c & 0xf0 ) == 0xe0 )
			inc = 3;
		else if ( ( c & 0xf8 ) == 0xf0 )
			inc = 4;
		else
			return -1;

		str += inc;
	}
	return count;
}

ZPL_IMPL_INLINE sw utf8_strnlen( u8 const* str, sw max_len )
{
	sw count = 0;
	for ( ; *str && max_len > 0; count++ )
	{
		u8 c   = *str;
		sw inc = 0;
		if ( c < 0x80 )
			inc = 1;
		else if ( ( c & 0xe0 ) == 0xc0 )
			inc = 2;
		else if ( ( c & 0xf0 ) == 0xe0 )
			inc = 3;
		else if ( ( c & 0xf8 ) == 0xf0 )
			inc = 4;
		else
			return -1;

		str     += inc;
		max_len -= inc;
	}
	return count;
}

ZPL_IMPL_INLINE s32 str_compare( const char* s1, const char* s2 )
{
	while ( *s1 && ( *s1 == *s2 ) )
	{
		s1++, s2++;
	}
	return *( u8* )s1 - *( u8* )s2;
}

ZPL_IMPL_INLINE char* strcpy( char* dest, const char* source )
{
	ZPL_ASSERT_NOT_NULL( dest );
	if ( source )
	{
		char* str = dest;
		while ( *source )
			*str++ = *source++;
	}
	return dest;
}

ZPL_IMPL_INLINE char* strcat( char* dest, const char* source )
{
	ZPL_ASSERT_NOT_NULL( dest );
	if ( source )
	{
		char* str = dest;
		while ( *str )
			++str;
		while ( *source )
			*str++ = *source++;
	}
	return dest;
}

ZPL_IMPL_INLINE char* strncpy( char* dest, const char* source, sw len )
{
	ZPL_ASSERT_NOT_NULL( dest );
	if ( source )
	{
		char* str = dest;
		while ( len > 0 && *source )
		{
			*str++ = *source++;
			len--;
		}
		while ( len > 0 )
		{
			*str++ = '\0';
			len--;
		}
	}
	return dest;
}

ZPL_IMPL_INLINE sw strlcpy( char* dest, const char* source, sw len )
{
	sw result = 0;
	ZPL_ASSERT_NOT_NULL( dest );
	if ( source )
	{
		const char* source_start = source;
		char*       str          = dest;
		while ( len > 0 && *source )
		{
			*str++ = *source++;
			len--;
		}
		while ( len > 0 )
		{
			*str++ = '\0';
			len--;
		}

		result = source - source_start;
	}
	return result;
}

ZPL_IMPL_INLINE char* strrev( char* str )
{
	sw    len = strlen( str );
	char* a   = str + 0;
	char* b   = str + len - 1;
	len       /= 2;
	while ( len-- )
	{
		swap( char, *a, *b );
		a++, b--;
	}
	return str;
}

ZPL_IMPL_INLINE s32 str_compare( const char* s1, const char* s2, sw len )
{
	for ( ; len > 0; s1++, s2++, len-- )
	{
		if ( *s1 != *s2 )
			return ( ( s1 < s2 ) ? -1 : +1 );
		else if ( *s1 == '\0' )
			return 0;
	}
	return 0;
}

ZPL_IMPL_INLINE const char* strtok( char* output, const char* src, const char* delimit )
{
	while ( *src && char_first_occurence( delimit, *src ) == NULL )
		*output++ = *src++;

	*output = 0;
	return *src ? src + 1 : src;
}

ZPL_IMPL_INLINE const char* strntok( char* output, sw len, const char* src, const char* delimit )
{
	ZPL_ASSERT( len > 0 );
	*( output + len - 1 ) = 0;
	while ( *src && char_first_occurence( delimit, *src ) == NULL && len > 0 )
	{
		*output++ = *src++;
		len--;
	}

	if ( len > 0 )
		*output = 0;
	return *src ? src + 1 : src;
}

ZPL_IMPL_INLINE b32 char_is_control( char c )
{
	return ! ! strchr( "\"\\/bfnrt", c );
}

ZPL_IMPL_INLINE b32 zpl__is_special_char( char c )
{
	return ! ! strchr( "<>:/", c );
}

ZPL_IMPL_INLINE b32 zpl__is_assign_char( char c )
{
	return ! ! strchr( ":=|", c );
}

ZPL_IMPL_INLINE b32 zpl__is_delim_char( char c )
{
	return ! ! strchr( ",|\n", c );
}

ZPL_IMPL_INLINE char const* str_control_skip( char const* str, char c )
{
	while ( ( *str && *str != c ) || ( *( str - 1 ) == '\\' && *str == c && char_is_control( c ) ) )
	{
		++str;
	}

	return str;
}

ZPL_IMPL_INLINE b32 str_has_prefix( const char* str, const char* prefix )
{
	while ( *prefix )
	{
		if ( *str++ != *prefix++ )
			return false;
	}
	return true;
}

ZPL_IMPL_INLINE b32 str_has_suffix( const char* str, const char* suffix )
{
	sw i = strlen( str );
	sw j = strlen( suffix );
	if ( j <= i )
		return str_compare( str + i - j, suffix ) == 0;
	return false;
}

ZPL_IMPL_INLINE const char* char_first_occurence( const char* s, char c )
{
	char ch = c;
	for ( ; *s != ch; s++ )
	{
		if ( *s == '\0' )
			return NULL;
	}
	return s;
}

ZPL_IMPL_INLINE const char* char_last_occurence( const char* s, char c )
{
	char* result = ( char* )NULL;
	do
	{
		if ( *s == c )
			result = ( char* )s;
	} while ( *s++ );

	return result;
}

ZPL_IMPL_INLINE char const* str_trim( char const* str, b32 catch_newline )
{
	while ( *str && char_is_space( *str ) && ( ! catch_newline || ( catch_newline && *str != '\n' ) ) )
	{
		++str;
	}
	return str;
}

ZPL_IMPL_INLINE char const* str_skip( char const* str, char c )
{
	while ( *str && *str != c )
	{
		++str;
	}
	return str;
}

ZPL_IMPL_INLINE char const* str_skip_any( char const* str, char const* char_list )
{
	char const* closest_ptr     = zpl_cast( char const* ) ptr_add( ( void* )str, strlen( str ) );
	sw          char_list_count = strlen( char_list );
	for ( sw i = 0; i < char_list_count; i++ )
	{
		char const* p = str_skip( str, char_list[ i ] );
		closest_ptr   = min( closest_ptr, p );
	}
	return closest_ptr;
}

ZPL_IMPL_INLINE char const* str_skip_literal( char const* str, char c )
{
	while ( ( *str && *str != c ) || ( *str == c && *( str - 1 ) == '\\' ) )
	{
		++str;
	}
	return str;
}

ZPL_IMPL_INLINE void str_concat( char* dest, sw dest_len, const char* src_a, sw src_a_len, const char* src_b, sw src_b_len )
{
	ZPL_ASSERT( dest_len >= src_a_len + src_b_len + 1 );
	if ( dest )
	{
		memcopy( dest, src_a, src_a_len );
		memcopy( dest + src_a_len, src_b, src_b_len );
		dest[ src_a_len + src_b_len ] = '\0';
	}
}

ZPL_IMPL_INLINE f32 str_to_f32( const char* str, char** end_ptr )
{
	f64 f = str_to_f64( str, end_ptr );
	f32 r = zpl_cast( f32 ) f;
	return r;
}

ZPL_IMPL_INLINE char* strdup( allocator a, char* src, sw max_len )
{
	ZPL_ASSERT_NOT_NULL( src );
	sw    len  = strlen( src );
	char* dest = zpl_cast( char* ) alloc( a, max_len );
	memset( dest + len, 0, max_len - len );
	strncpy( dest, src, max_len );

	return dest;
}

ZPL_IMPL_INLINE char** str_split_lines( allocator a_allocator, char* source, b32 strip_whitespace )
{
	char **lines = NULL, *p = source, *pd = p;
	array_init( lines, a_allocator );

	while ( *p )
	{
		if ( *pd == '\n' )
		{
			*pd = 0;
			if ( *( pd - 1 ) == '\r' )
				*( pd - 1 ) = 0;
			if ( strip_whitespace && ( pd - p ) == 0 )
			{
				p = pd + 1;
				continue;
			}
			array_append( lines, p );
			p = pd + 1;
		}
		++pd;
	}
	return lines;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
