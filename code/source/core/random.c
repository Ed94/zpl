// file: source/core/random.c


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#if defined( ZPL_MODULE_THREADING )
global atomic32 _random_shared_counter = { 0 };
#else
global s32 _random_shared_counter = 0;
#endif

internal u32 _get_noise_from_time( void )
{
	u32 accum = 0;
	f64 start, remaining, end, curr = 0;
	u64 interval = 100000ll;

	start     = time_rel();
	remaining = ( interval - zpl_cast( u64 )( interval * start ) % interval ) / zpl_cast( f64 ) interval;
	end       = start + remaining;

	do
	{
		curr   = time_rel();
		accum += zpl_cast( u32 ) curr;
	} while ( curr >= end );
	return accum;
}

// NOTE: Partly from http://preshing.com/20121224/how-to-generate-a-sequence-of-unique-random-integers/
// But the generation is even more random-er-est

internal ZPL_ALWAYS_INLINE u32 _permute_qpr( u32 x )
{
	local_persist u32 const prime = 4294967291;    // 2^32 - 5
	if ( x >= prime )
	{
		return x;
	}
	else
	{
		u32 residue = zpl_cast( u32 )( zpl_cast( u64 ) x * x ) % prime;
		if ( x <= prime / 2 )
			return residue;
		else
			return prime - residue;
	}
}

internal ZPL_ALWAYS_INLINE u32 _permute_with_offset( u32 x, u32 offset )
{
	return ( _permute_qpr( x ) + offset ) ^ 0x5bf03635;
}

void random_init( random* r )
{
	u64 time, tick;
	sw  i, j;
	u32 x    = 0;
	r->value = 0;

	r->offsets[ 0 ] = _get_noise_from_time();
#ifdef ZPL_MODULE_THREADING
	r->offsets[ 1 ] = atomic32_fetch_add( &_random_shared_counter, 1 );
	r->offsets[ 2 ] = thread_current_id();
	r->offsets[ 3 ] = thread_current_id() * 3 + 1;
#else
	r->offsets[ 1 ] = _random_shared_counter++;
	r->offsets[ 2 ] = 0;
	r->offsets[ 3 ] = 1;
#endif
	time            = time_tz_ms();
	r->offsets[ 4 ] = zpl_cast( u32 )( time >> 32 );
	r->offsets[ 5 ] = zpl_cast( u32 ) time;
	r->offsets[ 6 ] = _get_noise_from_time();
	tick            = rdtsc();
	r->offsets[ 7 ] = zpl_cast( u32 )( tick ^ ( tick >> 32 ) );

	for ( j = 0; j < 4; j++ )
	{
		for ( i = 0; i < count_of( r->offsets ); i++ )
		{
			r->offsets[ i ] = x = _permute_with_offset( x, r->offsets[ i ] );
		}
	}
}

u32 random_gen_u32( random* r )
{
	u32 x     = r->value;
	u32 carry = 1;
	sw  i;
	for ( i = 0; i < count_of( r->offsets ); i++ )
	{
		x = _permute_with_offset( x, r->offsets[ i ] );
		if ( carry > 0 )
		{
			carry = ++r->offsets[ i ] ? 0 : 1;
		}
	}

	r->value = x;
	return x;
}

u32 random_gen_u32_unique( random* r )
{
	u32 x = r->value;
	sw  i;
	r->value++;
	for ( i = 0; i < count_of( r->offsets ); i++ )
	{
		x = _permute_with_offset( x, r->offsets[ i ] );
	}

	return x;
}

u64 random_gen_u64( random* r )
{
	return ( ( zpl_cast( u64 ) random_gen_u32( r ) ) << 32 ) | random_gen_u32( r );
}

sw random_gen_isize( random* r )
{
#if defined( ZPL_ARCH_32_BIT )
	u32 u = random_gen_u32( r );
#else
	u64 u           = random_gen_u64( r );
#endif
	sw i;
	memcopy( &i, &u, size_of( u ) );
	return i;
}

s64 random_range_i64( random* r, s64 lower_inc, s64 higher_inc )
{
	u64 u     = random_gen_u64( r );
	s64 diff  = higher_inc - lower_inc + 1;
	u        %= diff;
	s64 i;
	memcopy( &i, &u, size_of( u ) );
	i += lower_inc;
	return i;
}

sw random_range_isize( random* r, sw lower_inc, sw higher_inc )
{
#if defined( ZPL_ARCH_32_BIT )
	u32 u = random_gen_u32( r );
#else
	u64 u           = random_gen_u64( r );
#endif
	sw diff  = higher_inc - lower_inc + 1;
	u       %= diff;
	sw i;
	memcopy( &i, &u, size_of( u ) );
	i += lower_inc;
	return i;
}

ZPL_ALWAYS_INLINE f64 _random_copy_sign64( f64 x, f64 y )
{
	s64 ix = 0, iy = 0;
	memcopy( &ix, &x, size_of( s64 ) );
	memcopy( &iy, &y, size_of( s64 ) );

	ix &= 0x7fffffffffffffff;
	ix |= iy & 0x8000000000000000;

	f64 r = 0.0;
	memcopy( &r, &ix, size_of( f64 ) );
	return r;
}

f64 random_range_f64( random* r, f64 lower_inc, f64 higher_inc )
{
	f64 f    = zpl_cast( f64 ) random_gen_u64( r ) / zpl_cast( f64 ) ZPL_U64_MAX;
	f64 diff = higher_inc - lower_inc;

	f *= diff;
	f += lower_inc;
	return f;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
