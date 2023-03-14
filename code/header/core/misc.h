// a_file: header/core/misc.h

/** @a_file misc.c
@brief Various other stuff
@defgroup misc Various other stuff

 Methods that don't belong anywhere but are still very useful in many occasions.

 @{
 */

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

ZPL_DEF void yield( void );

//! Returns allocated buffer
ZPL_DEF const char* get_env( const char* name );
ZPL_DEF const char* get_env_buf( const char* name );
ZPL_DEF string      get_env_str( const char* name );
ZPL_DEF void        set_env( const char* name, const char* value );
ZPL_DEF void        unset_env( const char* name );

ZPL_DEF u32    system_command( const char* command, uw buffer_len, char* buffer );
ZPL_DEF string system_command_str( const char* command, allocator backing );

ZPL_DEF_INLINE u16 endian_swap16( u16 i );
ZPL_DEF_INLINE u32 endian_swap32( u32 i );
ZPL_DEF_INLINE u64 endian_swap64( u64 i );

ZPL_DEF_INLINE sw count_set_bits( u64 mask );

//! @}
//$$

ZPL_IMPL_INLINE u16 endian_swap16( u16 i )
{
	return ( i >> 8 ) | ( i << 8 );
}

ZPL_IMPL_INLINE u32 endian_swap32( u32 i )
{
	return ( i >> 24 ) | ( i << 24 ) | ( ( i & 0x00ff0000u ) >> 8 ) | ( ( i & 0x0000ff00u ) << 8 );
}

ZPL_IMPL_INLINE u64 endian_swap64( u64 i )
{
	return ( i >> 56 ) | ( i << 56 ) | ( ( i & 0x00ff000000000000ull ) >> 40 ) | ( ( i & 0x000000000000ff00ull ) << 40 ) | ( ( i & 0x0000ff0000000000ull ) >> 24 )
	    | ( ( i & 0x0000000000ff0000ull ) << 24 ) | ( ( i & 0x000000ff00000000ull ) >> 8 ) | ( ( i & 0x00000000ff000000ull ) << 8 );
}

ZPL_IMPL_INLINE s32 next_pow2( s32 x )
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

ZPL_IMPL_INLINE void bit_set( u32* x, u32 bit )
{
	*x = *x | ( 1 << bit );
}

ZPL_IMPL_INLINE b8 bit_get( u32 x, u32 bit )
{
	return ( x & ( 1 << bit ) );
}

ZPL_IMPL_INLINE void bit_reset( u32* x, u32 bit )
{
	*x = *x & ~( 1 << bit );
}

ZPL_IMPL_INLINE sw count_set_bits( u64 mask )
{
	sw count = 0;
	while ( mask )
	{
		count += ( mask & 1 );
		mask  >>= 1;
	}
	return count;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
