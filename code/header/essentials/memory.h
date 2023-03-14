// a_file: header/essentials/memory.h

/** @a_file mem.c
@brief Memory manipulation and helpers.
@defgroup memman Memory management

 Consists of pointer arithmetic methods, virtual memory management and custom memory allocators.

 @{
 */

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

//! Checks if value is power of 2.
ZPL_DEF_INLINE b32 is_power_of_two( sw x );

//! Aligns address to specified alignment.
ZPL_DEF_INLINE void* align_forward( void* ptr, sw alignment );

//! Aligns value to a specified alignment.
ZPL_DEF_INLINE s64 align_forward_i64( s64 value, sw alignment );

//! Aligns value to a specified alignment.
ZPL_DEF_INLINE u64 align_forward_u64( u64 value, uw alignment );

//! Moves pointer forward by bytes.
ZPL_DEF_INLINE void* pointer_add( void* ptr, sw bytes );

//! Moves pointer backward by bytes.
ZPL_DEF_INLINE void* pointer_sub( void* ptr, sw bytes );

//! Moves pointer forward by bytes.
ZPL_DEF_INLINE void const* pointer_add_const( void const* ptr, sw bytes );

//! Moves pointer backward by bytes.
ZPL_DEF_INLINE void const* pointer_sub_const( void const* ptr, sw bytes );

//! Calculates difference between two addresses.
ZPL_DEF_INLINE sw pointer_diff( void const* begin, void const* end );

#define ptr_add       ZPL_NS pointer_add
#define ptr_sub       ZPL_NS pointer_sub
#define ptr_add_const ZPL_NS pointer_add_const
#define ptr_sub_const ZPL_NS pointer_sub_const
#define ptr_diff      ZPL_NS pointer_diff

//! Clears up memory at location by specified size.

//! @param ptr Memory location to clear up.
//! @param size The size to clear up with.
ZPL_DEF_INLINE void zero_size( void* ptr, sw size );

#ifndef zero_item
//! Clears up an item.
#	define zero_item( t ) zero_size( ( t ), size_of( *( t ) ) )    // NOTE: Pass pointer of struct

//! Clears up an array.
#	define zero_array( a, count ) zero_size( ( a ), size_of( *( a ) ) * count )
#endif

//! Copy memory from source to destination.
ZPL_DEF_INLINE void* memmove( void* dest, void const* source, sw size );

//! Set constant value at memory location with specified size.
ZPL_DEF_INLINE void* memset( void* data, u8 byte_value, sw size );

//! Compare two memory locations with specified size.
ZPL_DEF_INLINE s32 memcompare( void const* s1, void const* s2, sw size );

//! Swap memory contents between 2 locations with size.
ZPL_DEF void memswap( void* i, void* j, sw size );

//! Search for a constant value within the size limit at memory location.
ZPL_DEF void const* memchr( void const* data, u8 byte_value, sw size );

//! Search for a constant value within the size limit at memory location in backwards.
ZPL_DEF void const* memrchr( void const* data, u8 byte_value, sw size );

//! Copy non-overlapping memory from source to destination.
ZPL_DEF void* memcopy( void* dest, void const* source, sw size );

#ifndef memcopy_array

//! Copy non-overlapping array.
#	define memcopy_array( dst, src, count ) memcopy( ( dst ), ( src ), size_of( *( dst ) ) * ( count ) )
#endif

//! Copy an array.
#ifndef memmove_array
#	define memmove_array( dst, src, count ) memmove( ( dst ), ( src ), size_of( *( dst ) ) * ( count ) )
#endif

#ifndef ZPL_BIT_CAST
#	define ZPL_BIT_CAST( dest, source )                                                                                                                                           \
		do                                                                                                                                                                         \
		{                                                                                                                                                                          \
			ZPL_STATIC_ASSERT( size_of( *( dest ) ) <= size_of( source ), "size_of(*(dest)) !<= size_of(source)" );                                                                \
			memcopy( ( dest ), &( source ), size_of( *dest ) );                                                                                                                    \
		} while ( 0 )
#endif

#ifndef kilobytes
#	define kilobytes( x ) ( ( x ) * ( ZPL_NS s64 )( 1024 ) )
#	define megabytes( x ) ( kilobytes( x ) * ( ZPL_NS s64 )( 1024 ) )
#	define gigabytes( x ) ( megabytes( x ) * ( ZPL_NS s64 )( 1024 ) )
#	define terabytes( x ) ( gigabytes( x ) * ( ZPL_NS s64 )( 1024 ) )
#endif


/* inlines */

#define ZPL__ONES          ( zpl_cast( uw ) - 1 / ZPL_U8_MAX )
#define ZPL__HIGHS         ( ZPL__ONES * ( ZPL_U8_MAX / 2 + 1 ) )
#define ZPL__HAS_ZERO( x ) ( ( ( x )-ZPL__ONES ) & ~( x )&ZPL__HIGHS )

ZPL_IMPL_INLINE void* align_forward( void* ptr, sw alignment )
{
	uptr p;

	ZPL_ASSERT( is_power_of_two( alignment ) );

	p = zpl_cast( uptr ) ptr;
	return zpl_cast( void* )( ( p + ( alignment - 1 ) ) & ~( alignment - 1 ) );
}

ZPL_IMPL_INLINE s64 align_forward_i64( s64 value, sw alignment )
{
	return value + ( alignment - value % alignment ) % alignment;
}

ZPL_IMPL_INLINE u64 align_forward_u64( u64 value, uw alignment )
{
	return value + ( alignment - value % alignment ) % alignment;
}

ZPL_IMPL_INLINE void* pointer_add( void* ptr, sw bytes )
{
	return zpl_cast( void* )( zpl_cast( u8* ) ptr + bytes );
}

ZPL_IMPL_INLINE void* pointer_sub( void* ptr, sw bytes )
{
	return zpl_cast( void* )( zpl_cast( u8* ) ptr - bytes );
}

ZPL_IMPL_INLINE void const* pointer_add_const( void const* ptr, sw bytes )
{
	return zpl_cast( void const* )( zpl_cast( u8 const* ) ptr + bytes );
}

ZPL_IMPL_INLINE void const* pointer_sub_const( void const* ptr, sw bytes )
{
	return zpl_cast( void const* )( zpl_cast( u8 const* ) ptr - bytes );
}

ZPL_IMPL_INLINE sw pointer_diff( void const* begin, void const* end )
{
	return zpl_cast( sw )( zpl_cast( u8 const* ) end - zpl_cast( u8 const* ) begin );
}

ZPL_IMPL_INLINE void zero_size( void* ptr, sw size )
{
	memset( ptr, 0, size );
}

#if defined( _MSC_VER ) && ! defined( __clang__ )
#	pragma intrinsic( __movsb )
#endif

ZPL_IMPL_INLINE void* memmove( void* dest, void const* source, sw n )
{
	if ( dest == NULL )
	{
		return NULL;
	}

	u8*       d = zpl_cast( u8* ) dest;
	u8 const* s = zpl_cast( u8 const* ) source;

	if ( d == s )
		return d;
	if ( s + n <= d || d + n <= s )    // NOTE: Non-overlapping
		return memcopy( d, s, n );

	if ( d < s )
	{
		if ( zpl_cast( uptr ) s % size_of( sw ) == zpl_cast( uptr ) d % size_of( sw ) )
		{
			while ( zpl_cast( uptr ) d % size_of( sw ) )
			{
				if ( ! n-- )
					return dest;
				*d++ = *s++;
			}
			while ( n >= size_of( sw ) )
			{
				*zpl_cast( sw* ) d = *zpl_cast( sw* ) s;
				n                  -= size_of( sw );
				d                  += size_of( sw );
				s                  += size_of( sw );
			}
		}
		for ( ; n; n-- )
			*d++ = *s++;
	}
	else
	{
		if ( ( zpl_cast( uptr ) s % size_of( sw ) ) == ( zpl_cast( uptr ) d % size_of( sw ) ) )
		{
			while ( zpl_cast( uptr )( d + n ) % size_of( sw ) )
			{
				if ( ! n-- )
					return dest;
				d[ n ] = s[ n ];
			}
			while ( n >= size_of( sw ) )
			{
				n                         -= size_of( sw );
				*zpl_cast( sw* )( d + n ) = *zpl_cast( sw* )( s + n );
			}
		}
		while ( n )
			n--, d[ n ] = s[ n ];
	}

	return dest;
}

ZPL_IMPL_INLINE void* memset( void* dest, u8 c, sw n )
{
	if ( dest == NULL )
	{
		return NULL;
	}

	u8* s = zpl_cast( u8* ) dest;
	sw  k;
	u32 c32 = ( ( u32 )-1 ) / 255 * c;

	if ( n == 0 )
		return dest;
	s[ 0 ] = s[ n - 1 ] = c;
	if ( n < 3 )
		return dest;
	s[ 1 ] = s[ n - 2 ] = c;
	s[ 2 ] = s[ n - 3 ] = c;
	if ( n < 7 )
		return dest;
	s[ 3 ] = s[ n - 4 ] = c;
	if ( n < 9 )
		return dest;

	k = -zpl_cast( sptr ) s & 3;
	s += k;
	n -= k;
	n &= -4;

	*zpl_cast( u32* )( s + 0 )     = c32;
	*zpl_cast( u32* )( s + n - 4 ) = c32;
	if ( n < 9 )
		return dest;
	*zpl_cast( u32* )( s + 4 )      = c32;
	*zpl_cast( u32* )( s + 8 )      = c32;
	*zpl_cast( u32* )( s + n - 12 ) = c32;
	*zpl_cast( u32* )( s + n - 8 )  = c32;
	if ( n < 25 )
		return dest;
	*zpl_cast( u32* )( s + 12 )     = c32;
	*zpl_cast( u32* )( s + 16 )     = c32;
	*zpl_cast( u32* )( s + 20 )     = c32;
	*zpl_cast( u32* )( s + 24 )     = c32;
	*zpl_cast( u32* )( s + n - 28 ) = c32;
	*zpl_cast( u32* )( s + n - 24 ) = c32;
	*zpl_cast( u32* )( s + n - 20 ) = c32;
	*zpl_cast( u32* )( s + n - 16 ) = c32;

	k = 24 + ( zpl_cast( uptr ) s & 4 );
	s += k;
	n -= k;

	{
		u64 c64 = ( zpl_cast( u64 ) c32 << 32 ) | c32;
		while ( n > 31 )
		{
			*zpl_cast( u64* )( s + 0 )  = c64;
			*zpl_cast( u64* )( s + 8 )  = c64;
			*zpl_cast( u64* )( s + 16 ) = c64;
			*zpl_cast( u64* )( s + 24 ) = c64;

			n -= 32;
			s += 32;
		}
	}

	return dest;
}

ZPL_IMPL_INLINE s32 memcompare( void const* s1, void const* s2, sw size )
{
	u8 const* s1p8 = zpl_cast( u8 const* ) s1;
	u8 const* s2p8 = zpl_cast( u8 const* ) s2;

	if ( s1 == NULL || s2 == NULL )
	{
		return 0;
	}

	while ( size-- )
	{
		sw d;
		if ( ( d = ( *s1p8++ - *s2p8++ ) ) != 0 )
			return zpl_cast( s32 ) d;
	}
	return 0;
}

ZPL_IMPL_INLINE b32 is_power_of_two( sw x )
{
	if ( x <= 0 )
		return false;
	return ! ( x & ( x - 1 ) );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
