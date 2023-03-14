// file: header/essentials/collections/array.h

////////////////////////////////////////////////////////////////
//
// Dynamic Array (POD Types)
//
// zpl_array(Type) works like string or buffer where the actual type is just a pointer to the first
// element.
//
// Available Procedures for zpl_array(Type)
// array_init
// array_free
// array_set_capacity
// array_grow
// array_append
// array_appendv
// array_pop
// array_clear
// array_back
// array_front
// array_resize
// array_reserve
//

#if 0 // Example
void foo(void) {
    sw i;
    int test_values[] = {4, 2, 1, 7};
    zpl_allocator a = heap_allocator();
    zpl_array(int) items;

    array_init(items, a);

    array_append(items, 1);
    array_append(items, 4);
    array_append(items, 9);
    array_append(items, 16);

    items[1] = 3; // Manually set value
    // NOTE: No array bounds checking

    for (i = 0; i < items.count; i++)
        zpl_printf("%d\n", items[i]);
    // 1
    // 3
    // 9
    // 16

    array_clear(items);

    array_appendv(items, test_values, count_of(test_values));
    for (i = 0; i < items.count; i++)
        zpl_printf("%d\n", items[i]);
    // 4
    // 2
    // 1
    // 7

    array_free(items);
}
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct array_header
{
	sw            elem_size;
	sw            count;
	sw            capacity;
	zpl_allocator allocator;
} array_header;

#define zpl_array( Type ) Type*

#define array_make( Type, Name, allocator )                                                                                                                                        \
	Type* Name;                                                                                                                                                                    \
	array_init( Name, allocator )

#ifndef ZPL_ARRAY_GROW_FORMULA
#define ZPL_ARRAY_GROW_FORMULA( x ) ( 2 * ( x ) + 8 )
#endif

ZPL_STATIC_ASSERT( ZPL_ARRAY_GROW_FORMULA( 0 ) > 0, "ZPL_ARRAY_GROW_FORMULA(0) <= 0" );

#define ZPL_ARRAY_HEADER( x ) ( zpl_cast( array_header* )( x ) - 1 )
#define array_allocator( x )  ( ZPL_ARRAY_HEADER( x )->allocator )
#define array_elem_size( x )  ( ZPL_ARRAY_HEADER( x )->elem_size )
#define array_count( x )      ( ZPL_ARRAY_HEADER( x )->count )
#define array_capacity( x )   ( ZPL_ARRAY_HEADER( x )->capacity )
#define array_end( x )        ( x + ( array_count( x ) - 1 ) )

ZPL_IMPL_INLINE b8 zpl__array_init_reserve( void** zpl__array_, zpl_allocator allocator_, sw elem_size, sw cap )
{
	array_header* zpl__ah = zpl_cast( array_header* ) alloc( allocator_, size_of( array_header ) + elem_size * cap );
	if ( ! zpl__ah )
		return false;
	zpl__ah->allocator = allocator_;
	zpl__ah->elem_size = elem_size;
	zpl__ah->count     = 0;
	zpl__ah->capacity  = cap;
	*zpl__array_       = zpl_cast( void* )( zpl__ah + 1 );
	return true;
}

#define array_init_reserve( x, allocator_, cap ) zpl__array_init_reserve( zpl_cast( void** ) & ( x ), allocator_, size_of( *( x ) ), ( cap ) )

// NOTE: Give it an initial default capacity
#define array_init( x, allocator ) array_init_reserve( x, allocator, ZPL_ARRAY_GROW_FORMULA( 0 ) )

#define array_free( x )                                                                                                                                                            \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		if ( x )                                                                                                                                                                   \
		{                                                                                                                                                                          \
			array_header* zpl__ah = ZPL_ARRAY_HEADER( x );                                                                                                                         \
			free( zpl__ah->allocator, zpl__ah );                                                                                                                                   \
		}                                                                                                                                                                          \
	} while ( 0 )

ZPL_IMPL_INLINE b8 zpl__array_set_capacity( void** array, sw capacity )
{
	array_header* h = ZPL_ARRAY_HEADER( *array );
	if ( capacity == h->capacity )
		return true;
	if ( capacity < h->count )
		h->count = capacity;
	sw            size = size_of( array_header ) + h->elem_size * capacity;
	array_header* nh   = zpl_cast( array_header* ) alloc( h->allocator, size );
	if ( ! nh )
		return false;
	zpl_memmove( nh, h, size_of( array_header ) + h->elem_size * h->count );
	nh->allocator = h->allocator;
	nh->elem_size = h->elem_size;
	nh->count     = h->count;
	nh->capacity  = capacity;
	free( h->allocator, h );
	*array = nh + 1;
	return true;
}

#define array_set_capacity( x, capacity ) zpl__array_set_capacity( zpl_cast( void** ) & ( x ), ( capacity ) )

ZPL_IMPL_INLINE b8 zpl__array_grow( void** x, sw min_capacity )
{
	sw new_capacity = ZPL_ARRAY_GROW_FORMULA( array_capacity( *x ) );
	if ( new_capacity < min_capacity )
		new_capacity = min_capacity;
	return zpl__array_set_capacity( x, new_capacity );
}

#define array_grow( x, min_capacity ) zpl__array_grow( zpl_cast( void** ) & ( x ), ( min_capacity ) )

ZPL_IMPL_INLINE b8 zpl__array_append_helper( void** x )
{
	if ( array_capacity( *x ) < array_count( *x ) + 1 )
	{
		if ( ! zpl__array_grow( x, 0 ) )
			return false;
	}
	return true;
}

#define array_append( x, item ) ( zpl__array_append_helper( zpl_cast( void** ) & ( x ) ) && ( ( ( x )[ array_count( x )++ ] = ( item ) ), true ) )

ZPL_IMPL_INLINE b8 zpl__array_append_at_helper( void** x, sw ind )
{
	if ( ind >= array_count( *x ) )
		ind = array_count( *x ) - 1;
	if ( ind < 0 )
		ind = 0;
	if ( array_capacity( *x ) < array_count( *x ) + 1 )
	{
		if ( ! zpl__array_grow( x, 0 ) )
			return false;
	}
	s8* s = ( zpl_cast( s8* ) * x ) + ind * array_elem_size( *x );
	zpl_memmove( s + array_elem_size( *x ), s, array_elem_size( *x ) * ( array_count( *x ) - ind ) );
	return true;
}

#define array_append_at( x, item, ind ) ( zpl__array_append_at_helper( zpl_cast( void** ) & ( x ), ( ind ) ) && ( ( ( x )[ ind ] = ( item ) ), array_count( x )++, true ) )

ZPL_IMPL_INLINE b8 zpl__array_appendv( void** x, void* items, sw item_size, sw item_count )
{
	ZPL_ASSERT( item_size == array_elem_size( *x ) );
	if ( array_capacity( *x ) < array_count( *x ) + item_count )
	{
		if ( ! zpl__array_grow( x, array_count( *x ) + item_count ) )
			return false;
	}
	zpl_memcopy( ( zpl_cast( s8* ) * x ) + array_count( *x ) * array_elem_size( *x ), items, array_elem_size( *x ) * item_count );
	array_count( *x ) += item_count;
	return true;
}

#define array_appendv( x, items, item_count ) zpl__array_appendv( zpl_cast( void** ) & ( x ), ( items ), size_of( ( items )[ 0 ] ), ( item_count ) )

ZPL_IMPL_INLINE b8 zpl__array_appendv_at( void** x, void* items, sw item_size, sw item_count, sw ind )
{
	if ( ind >= array_count( *x ) )
		return zpl__array_appendv( x, items, item_size, item_count );
	ZPL_ASSERT( item_size == array_elem_size( *x ) );
	if ( array_capacity( *x ) < array_count( *x ) + item_count )
	{
		if ( ! zpl__array_grow( x, array_count( *x ) + item_count ) )
			return false;
	}
	zpl_memmove( ( zpl_cast( s8* ) * x ) + ( ind + item_count ) * array_elem_size( *x ), ( zpl_cast( s8* ) * x ) + ind * array_elem_size( *x ),
	             array_elem_size( *x ) * ( array_count( *x ) - ind ) );
	zpl_memcopy( ( zpl_cast( s8* ) * x ) + ind * array_elem_size( *x ), items, array_elem_size( *x ) * item_count );
	array_count( *x ) += item_count;
	return true;
}

#define array_appendv_at( x, items, item_count, ind ) zpl__array_appendv_at( zpl_cast( void** ) & ( x ), ( items ), size_of( ( items )[ 0 ] ), ( item_count ), ( ind ) )

#define array_fill( x, begin, end, value )                                                                                                                                         \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		ZPL_ASSERT( ( begin ) >= 0 && ( end ) < array_count( x ) );                                                                                                                \
		ZPL_ASSERT( size_of( value ) == size_of( ( x )[ 0 ] ) );                                                                                                                   \
		for ( sw i = ( begin ); i < ( end ); i++ )                                                                                                                                 \
		{                                                                                                                                                                          \
			x[ i ] = value;                                                                                                                                                        \
		}                                                                                                                                                                          \
	} while ( 0 )

#define array_remove_at( x, index )                                                                                                                                                \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		array_header* zpl__ah = ZPL_ARRAY_HEADER( x );                                                                                                                             \
		ZPL_ASSERT( index < zpl__ah->count );                                                                                                                                      \
		zpl_memmove( x + index, x + index + 1, size_of( x[ 0 ] ) * ( zpl__ah->count - index - 1 ) );                                                                               \
		--zpl__ah->count;                                                                                                                                                          \
	} while ( 0 )

ZPL_IMPL_INLINE b8 zpl__array_copy_init( void** y, void** x )
{
	if ( ! zpl__array_init_reserve( y, array_allocator( *x ), array_elem_size( *x ), array_capacity( *x ) ) )
		return false;
	zpl_memcopy( *y, *x, array_capacity( *x ) * array_elem_size( *x ) );
	array_count( *y ) = array_count( *x );
	return true;
}

#define array_copy_init( y, x ) zpl__array_copy_init( zpl_cast( void** ) & ( y ), zpl_cast( void** ) & ( x ) )

#define array_pop( x )                                                                                                                                                             \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		ZPL_ASSERT( ZPL_ARRAY_HEADER( x )->count > 0 );                                                                                                                            \
		ZPL_ARRAY_HEADER( x )->count--;                                                                                                                                            \
	} while ( 0 )
#define array_back( x )  x[ ZPL_ARRAY_HEADER( x )->count - 1 ]
#define array_front( x ) x[ 0 ]
#define array_clear( x )                                                                                                                                                           \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		ZPL_ARRAY_HEADER( x )->count = 0;                                                                                                                                          \
	} while ( 0 )

ZPL_IMPL_INLINE b8 zpl__array_resize( void** x, sw new_count )
{
	if ( ZPL_ARRAY_HEADER( *x )->capacity < new_count )
	{
		if ( ! zpl__array_grow( x, new_count ) )
			return false;
	}
	ZPL_ARRAY_HEADER( *x )->count = new_count;
	return true;
}

#define array_resize( x, new_count ) zpl__array_resize( zpl_cast( void** ) & ( x ), ( new_count ) )

ZPL_IMPL_INLINE b8 zpl__array_reserve( void** x, sw new_capacity )
{
	if ( ZPL_ARRAY_HEADER( *x )->capacity < new_capacity )
		return zpl__array_set_capacity( x, new_capacity );
	return true;
}

#define array_reserve( x, new_capacity ) zpl__array_reserve( zpl_cast( void** ) & ( x ), ( new_capacity ) )

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
