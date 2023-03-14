// file: header/core/sort.h

/** @file sort.c
@brief Sorting and searching methods.
@defgroup sort Sorting and searching

Methods for sorting arrays using either Quick/Merge-sort combo or Radix sort. It also contains simple implementation of binary search, as well as an easy to use API to define your
own comparators.

@{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#define ZPL_COMPARE_PROC( name ) int name( void const * a, void const * b )
typedef ZPL_COMPARE_PROC( compare_proc );

#define ZPL_COMPARE_PROC_PTR( def ) ZPL_COMPARE_PROC( ( *def ) )

// Procedure pointers
// NOTE: The offset parameter specifies the offset in the structure
// e.g. i32_cmp(offset_of(Thing, value))
// Use 0 if it's just the type instead.

ZPL_DEF ZPL_COMPARE_PROC_PTR( i16_cmp( sw offset ) );
ZPL_DEF ZPL_COMPARE_PROC_PTR( u8_cmp( sw offset ) );
ZPL_DEF ZPL_COMPARE_PROC_PTR( i32_cmp( sw offset ) );
ZPL_DEF ZPL_COMPARE_PROC_PTR( i64_cmp( sw offset ) );
ZPL_DEF ZPL_COMPARE_PROC_PTR( isize_cmp( sw offset ) );
ZPL_DEF ZPL_COMPARE_PROC_PTR( str_cmp( sw offset ) );
ZPL_DEF ZPL_COMPARE_PROC_PTR( f32_cmp( sw offset ) );
ZPL_DEF ZPL_COMPARE_PROC_PTR( f64_cmp( sw offset ) );

// TODO: Better sorting algorithms

//! Sorts an array.

//! Uses quick sort for large arrays but insertion sort for small ones.
#define sort_array( array, count, compare_proc ) sort( array, count, size_of( *( array ) ), compare_proc )

//! Perform sorting operation on a memory location with a specified item count and size.
ZPL_DEF void sort( void* base, sw count, sw size, compare_proc compare_proc );

// NOTE: the count of temp == count of items
#define radix_sort( Type )          radix_sort_##Type
#define ZPL_RADIX_SORT_PROC( Type ) void radix_sort( Type )(##Type * items, ##Type * temp, sw count )

ZPL_DEF ZPL_RADIX_SORT_PROC( u8 );
ZPL_DEF ZPL_RADIX_SORT_PROC( u16 );
ZPL_DEF ZPL_RADIX_SORT_PROC( u32 );
ZPL_DEF ZPL_RADIX_SORT_PROC( u64 );

//! Performs binary search on an array.

//! Returns index or -1 if not found
#define binary_search_array( array, count, key, compare_proc ) binary_search( array, count, size_of( *( array ) ), key, compare_proc )

//! Performs binary search on a memory location with specified item count and size.
ZPL_DEF_INLINE sw binary_search( void const * base, sw count, sw size, void const * key, compare_proc compare_proc );

#define shuffle_array( array, count ) shuffle( array, count, size_of( *( array ) ) )

//! Shuffles a memory.
ZPL_DEF void shuffle( void* base, sw count, sw size );

#define reverse_array( array, count ) reverse( array, count, size_of( *( array ) ) )

//! Reverses memory's contents
ZPL_DEF void reverse( void* base, sw count, sw size );

//! @}

ZPL_IMPL_INLINE sw binary_search( void const * base, sw count, sw size, void const * key, compare_proc compare_proc )
{
	sw start = 0;
	sw end   = count;

	while ( start < end )
	{
		sw mid    = start + ( end - start ) / 2;
		sw result = compare_proc( key, zpl_cast( u8* ) base + mid * size );
		if ( result < 0 )
			end = mid;
		else if ( result > 0 )
			start = mid + 1;
		else
			return mid;
	}

	return -1;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
