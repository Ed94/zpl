// file: source/core/sort.c

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#define ZPL__COMPARE_PROC( Type )                                                                                                                                                  \
	global sw Type##__cmp_offset;                                                                                                                                                  \
	ZPL_COMPARE_PROC( Type##__cmp )                                                                                                                                                \
	{                                                                                                                                                                              \
		Type const p = *zpl_cast( Type const * ) pointer_add_const( a, Type##__cmp_offset );                                                                                       \
		Type const q = *zpl_cast( Type const * ) pointer_add_const( b, Type##__cmp_offset );                                                                                       \
		return p < q ? -1 : p > q;                                                                                                                                                 \
	}                                                                                                                                                                              \
	ZPL_COMPARE_PROC_PTR( Type##_cmp( sw offset ) )                                                                                                                                \
	{                                                                                                                                                                              \
		Type##__cmp_offset = offset;                                                                                                                                               \
		return &Type##__cmp;                                                                                                                                                       \
	}

ZPL__COMPARE_PROC( u8 );
ZPL__COMPARE_PROC( s16 );
ZPL__COMPARE_PROC( s32 );
ZPL__COMPARE_PROC( s64 );
ZPL__COMPARE_PROC( sw );
ZPL__COMPARE_PROC( f32 );
ZPL__COMPARE_PROC( f64 );

// NOTE: str_cmp is special as it requires a funny type and funny comparison
global sw zpl__str_cmp_offset;
ZPL_COMPARE_PROC( zpl__str_cmp )
{
	char const * p = *zpl_cast( char const ** ) pointer_add_const( a, zpl__str_cmp_offset );
	char const * q = *zpl_cast( char const ** ) pointer_add_const( b, zpl__str_cmp_offset );
	return str_compare( p, q );
}
ZPL_COMPARE_PROC_PTR( str_cmp( sw offset ) )
{
	zpl__str_cmp_offset = offset;
	return &zpl__str_cmp;
}

#undef ZPL__COMPARE_PROC

// TODO: Make user definable?
#define ZPL__SORT_STACK_SIZE           64
#define zpl__SORT_INSERT_SORT_TRESHOLD 8

#define ZPL__SORT_PUSH( _base, _limit )                                                                                                                                            \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		stack_ptr[ 0 ] = ( _base );                                                                                                                                                \
		stack_ptr[ 1 ] = ( _limit );                                                                                                                                               \
		stack_ptr += 2;                                                                                                                                                            \
	} while ( 0 )

#define ZPL__SORT_POP( _base, _limit )                                                                                                                                             \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		stack_ptr -= 2;                                                                                                                                                            \
		( _base )  = stack_ptr[ 0 ];                                                                                                                                               \
		( _limit ) = stack_ptr[ 1 ];                                                                                                                                               \
	} while ( 0 )

void sort( void* base_, sw count, sw size, compare_proc cmp )
{
	u8 *i, *j;
	u8* base      = zpl_cast( u8* ) base_;
	u8* limit     = base + count * size;
	sw  threshold = zpl__SORT_INSERT_SORT_TRESHOLD * size;

	// NOTE: Prepare the stack
	u8*  stack[ ZPL__SORT_STACK_SIZE ] = { 0 };
	u8** stack_ptr                     = stack;

	for ( ;; )
	{
		if ( ( limit - base ) > threshold )
		{
			// NOTE: Quick sort
			i = base + size;
			j = limit - size;

			zpl_memswap( ( ( limit - base ) / size / 2 ) * size + base, base, size );
			if ( cmp( i, j ) > 0 )
				zpl_memswap( i, j, size );
			if ( cmp( base, j ) > 0 )
				zpl_memswap( base, j, size );
			if ( cmp( i, base ) > 0 )
				zpl_memswap( i, base, size );

			for ( ;; )
			{
				do
					i += size;
				while ( cmp( i, base ) < 0 );
				do
					j -= size;
				while ( cmp( j, base ) > 0 );
				if ( i > j )
					break;
				zpl_memswap( i, j, size );
			}

			zpl_memswap( base, j, size );

			if ( j - base > limit - i )
			{
				ZPL__SORT_PUSH( base, j );
				base = i;
			}
			else
			{
				ZPL__SORT_PUSH( i, limit );
				limit = j;
			}
		}
		else
		{
			// NOTE: Insertion sort
			for ( j = base, i = j + size; i < limit; j = i, i += size )
			{
				for ( ; cmp( j, j + size ) > 0; j -= size )
				{
					zpl_memswap( j, j + size, size );
					if ( j == base )
						break;
				}
			}

			if ( stack_ptr == stack )
				break; // NOTE: Sorting is done!
			ZPL__SORT_POP( base, limit );
		}
	}
}

#undef ZPL__SORT_PUSH
#undef ZPL__SORT_POP

#define ZPL_RADIX_SORT_PROC_GEN( Type )                                                                                                                                            \
	ZPL_RADIX_SORT_PROC( Type )                                                                                                                                                    \
	{                                                                                                                                                                              \
		##Type* source = items;                                                                                                                                                    \
		##Type* dest   = temp;                                                                                                                                                     \
		sw      byte_index, i, byte_max = 8 * size_of(##Type );                                                                                                                    \
		for ( byte_index = 0; byte_index < byte_max; byte_index += 8 )                                                                                                             \
		{                                                                                                                                                                          \
			sw offsets[ 256 ] = { 0 };                                                                                                                                             \
			sw total          = 0;                                                                                                                                                 \
			/* NOTE: First pass - count how many of each key */                                                                                                                    \
			for ( i = 0; i < count; i++ )                                                                                                                                          \
			{                                                                                                                                                                      \
				##Type radix_value = source[ i ];                                                                                                                                  \
				##Type radix_piece = ( radix_value >> byte_index ) & 0xff;                                                                                                         \
				offsets[ radix_piece ]++;                                                                                                                                          \
			}                                                                                                                                                                      \
			/* NOTE: Change counts to offsets */                                                                                                                                   \
			for ( i = 0; i < count_of( offsets ); i++ )                                                                                                                            \
			{                                                                                                                                                                      \
				sw skcount   = offsets[ i ];                                                                                                                                       \
				offsets[ i ] = total;                                                                                                                                              \
				total += skcount;                                                                                                                                                  \
			}                                                                                                                                                                      \
			/* NOTE: Second pass - place elements into the right location */                                                                                                       \
			for ( i = 0; i < count; i++ )                                                                                                                                          \
			{                                                                                                                                                                      \
				##Type radix_value               = source[ i ];                                                                                                                    \
				##Type radix_piece               = ( radix_value >> byte_index ) & 0xff;                                                                                           \
				dest[ offsets[ radix_piece ]++ ] = source[ i ];                                                                                                                    \
			}                                                                                                                                                                      \
			swap(##Type*, source, dest );                                                                                                                                          \
		}                                                                                                                                                                          \
	}

ZPL_RADIX_SORT_PROC_GEN( u8 );
ZPL_RADIX_SORT_PROC_GEN( u16 );
ZPL_RADIX_SORT_PROC_GEN( u32 );
ZPL_RADIX_SORT_PROC_GEN( u64 );

void shuffle( void* base, sw count, sw size )
{
	u8*    a;
	sw     i, j;
	random random;
	random_init( &random );

	a = zpl_cast( u8* ) base + ( count - 1 ) * size;
	for ( i = count; i > 1; i-- )
	{
		j = random_gen_isize( &random ) % i;
		zpl_memswap( a, zpl_cast( u8* ) base + j * size, size );
		a -= size;
	}
}

void reverse( void* base, sw count, sw size )
{
	sw i, j = count - 1;
	for ( i = 0; i < j; i++, j++ )
		zpl_memswap( zpl_cast( u8* ) base + i * size, zpl_cast( u8* ) base + j * size, size );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
