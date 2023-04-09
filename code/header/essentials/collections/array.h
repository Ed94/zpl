// file: header/essentials/collections/array.h

////////////////////////////////////////////////////////////////
//
// Dynamic Array (POD Types)
//
// zpl_array(Type) works like zpl_string or zpl_buffer where the actual type is just a pointer to the first
// element.
//
// Available Procedures for zpl_array(Type)
// zpl_array_init
// zpl_array_free
// zpl_array_set_capacity
// zpl_array_grow
// zpl_array_append
// zpl_array_appendv
// zpl_array_pop
// zpl_array_clear
// zpl_array_back
// zpl_array_front
// zpl_array_resize
// zpl_array_reserve
//

#if 0    // Example
void foo(void) {
    sw i;
    int test_values[] = {4, 2, 1, 7};
    AllocatorInfo a = heap_allocator();
    Array(int) items;

    array_init(items, a);

    array_append(items, 1);
    array_append(items, 4);
    array_append(items, 9);
    array_append(items, 16);

    items[1] = 3; // Manually set value
    // NOTE: No array bounds checking

    for (i = 0; i < items.count; i++)
        printf("%d\n", items[i]);
    // 1
    // 3
    // 9
    // 16

    array_clear(items);

    array_appendv(items, test_values, count_of(test_values));
    for (i = 0; i < items.count; i++)
        printf("%d\n", items[i]);
    // 4
    // 2
    // 1
    // 7

    array_free(items);
}
#endif


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct ArrayHeader
{
	sw            elem_size;
	sw            count;
	sw            capacity;
	AllocatorInfo allocator;
} ArrayHeader;

#define Array( Type ) Type*

#define array_make( Type, Name, allocator ) \
	Type* Name;                             \
	ZPL_NS( array_init )( Name, allocator )

#ifndef ZPL_ARRAY_GROW_FORMULA
#	define ZPL_ARRAY_GROW_FORMULA( x ) ( 2 * ( x ) + 8 )
#endif

ZPL_STATIC_ASSERT( ZPL_ARRAY_GROW_FORMULA( 0 ) > 0, "ZPL_ARRAY_GROW_FORMULA(0) <= 0" );

#define ZPL_ARRAY_HEADER( x ) ( zpl_cast( ZPL_NS( ArrayHeader )* )( x ) - 1 )
#define array_allocator( x )  ( ZPL_ARRAY_HEADER( x )->allocator )
#define array_elem_size( x )  ( ZPL_ARRAY_HEADER( x )->elem_size )
#define array_count( x )      ( ZPL_ARRAY_HEADER( x )->count )
#define array_capacity( x )   ( ZPL_ARRAY_HEADER( x )->capacity )
#define array_end( x )        ( x + ( array_count( x ) - 1 ) )

ZPL_IMPL_INLINE b8 _array_init_reserve( void** _array_, AllocatorInfo allocator_, sw elem_size, sw cap )
{
	ArrayHeader* _ah = zpl_cast( ArrayHeader* ) alloc( allocator_, size_of( ArrayHeader ) + elem_size * cap );
	if ( ! _ah )
		return false;
	_ah->allocator = allocator_;
	_ah->elem_size = elem_size;
	_ah->count     = 0;
	_ah->capacity  = cap;
	*_array_       = zpl_cast( void* )( _ah + 1 );
	return true;
}

#define array_init_reserve( x, allocator_, cap ) ZPL_NS( _array_init_reserve )( zpl_cast( void** ) & ( x ), allocator_, size_of( *( x ) ), ( cap ) )

// NOTE: Give it an initial default capacity
#define array_init( x, allocator ) array_init_reserve( x, allocator, ZPL_ARRAY_GROW_FORMULA( 0 ) )

#define array_free( x )                                         \
	do                                                          \
	{                                                           \
		if ( x )                                                \
		{                                                       \
			ZPL_NS( ArrayHeader )* _ah = ZPL_ARRAY_HEADER( x ); \
			ZPL_NS( free )( _ah->allocator, _ah );              \
		}                                                       \
	} while ( 0 )

ZPL_IMPL_INLINE b8 _array_set_capacity( void** array, sw capacity )
{
	ArrayHeader* h = ZPL_ARRAY_HEADER( *array );
	if ( capacity == h->capacity )
		return true;
	if ( capacity < h->count )
		h->count = capacity;
	sw           size = size_of( ArrayHeader ) + h->elem_size * capacity;
	ArrayHeader* nh   = zpl_cast( ArrayHeader* ) alloc( h->allocator, size );
	if ( ! nh )
		return false;
	memmove( nh, h, size_of( ArrayHeader ) + h->elem_size * h->count );
	nh->allocator = h->allocator;
	nh->elem_size = h->elem_size;
	nh->count     = h->count;
	nh->capacity  = capacity;
	free( h->allocator, h );
	*array = nh + 1;
	return true;
}

#define array_set_capacity( x, capacity ) ZPL_NS( _array_set_capacity )( zpl_cast( void** ) & ( x ), ( capacity ) )

ZPL_IMPL_INLINE b8 _array_grow( void** x, sw min_capacity )
{
	sw new_capacity = ZPL_ARRAY_GROW_FORMULA( array_capacity( *x ) );
	if ( new_capacity < min_capacity )
		new_capacity = min_capacity;
	return _array_set_capacity( x, new_capacity );
}

#define array_grow( x, min_capacity ) ZPL_NS( _array_grow )( zpl_cast( void** ) & ( x ), ( min_capacity ) )

ZPL_IMPL_INLINE b8 _array_append_helper( void** x )
{
	if ( array_capacity( *x ) < array_count( *x ) + 1 )
	{
		if ( ! _array_grow( x, 0 ) )
			return false;
	}
	return true;
}

#define array_append( x, item ) ( ZPL_NS( _array_append_helper )( zpl_cast( void** ) & ( x ) ) && ( ( ( x )[ array_count( x )++ ] = ( item ) ), true ) )

ZPL_IMPL_INLINE b8 _array_append_at_helper( void** x, sw ind )
{
	if ( ind >= array_count( *x ) )
		ind = array_count( *x ) - 1;
	if ( ind < 0 )
		ind = 0;
	if ( array_capacity( *x ) < array_count( *x ) + 1 )
	{
		if ( ! _array_grow( x, 0 ) )
			return false;
	}
	s8* s = ( zpl_cast( s8* ) * x ) + ind * array_elem_size( *x );
	memmove( s + array_elem_size( *x ), s, array_elem_size( *x ) * ( array_count( *x ) - ind ) );
	return true;
}

#define array_append_at( x, item, ind ) ( ZPL_NS( _array_append_at_helper )( zpl_cast( void** ) & ( x ), ( ind ) ) && ( ( ( x )[ ind ] = ( item ) ), array_count( x )++, true ) )

ZPL_IMPL_INLINE b8 _array_appendv( void** x, void* items, sw item_size, sw item_count )
{
	ZPL_ASSERT( item_size == array_elem_size( *x ) );
	if ( array_capacity( *x ) < array_count( *x ) + item_count )
	{
		if ( ! _array_grow( x, array_count( *x ) + item_count ) )
			return false;
	}
	memcopy( ( zpl_cast( s8* ) * x ) + array_count( *x ) * array_elem_size( *x ), items, array_elem_size( *x ) * item_count );
	array_count( *x ) += item_count;
	return true;
}

#define array_appendv( x, items, item_count ) ZPL_NS( _array_appendv )( zpl_cast( void** ) & ( x ), ( items ), size_of( ( items )[ 0 ] ), ( item_count ) )

ZPL_IMPL_INLINE b8 _array_appendv_at( void** x, void* items, sw item_size, sw item_count, sw ind )
{
	if ( ind >= array_count( *x ) )
		return _array_appendv( x, items, item_size, item_count );
	ZPL_ASSERT( item_size == array_elem_size( *x ) );
	if ( array_capacity( *x ) < array_count( *x ) + item_count )
	{
		if ( ! _array_grow( x, array_count( *x ) + item_count ) )
			return false;
	}
	memmove(
	    ( zpl_cast( s8* ) * x ) + ( ind + item_count ) * array_elem_size( *x ),
	    ( zpl_cast( s8* ) * x ) + ind * array_elem_size( *x ),
	    array_elem_size( *x ) * ( array_count( *x ) - ind )
	);
	memcopy( ( zpl_cast( s8* ) * x ) + ind * array_elem_size( *x ), items, array_elem_size( *x ) * item_count );
	array_count( *x ) += item_count;
	return true;
}

#define array_appendv_at( x, items, item_count, ind ) ZPL_NS( _array_appendv_at )( zpl_cast( void** ) & ( x ), ( items ), size_of( ( items )[ 0 ] ), ( item_count ), ( ind ) )

#define array_fill( x, begin, end, value )                          \
	do                                                              \
	{                                                               \
		ZPL_ASSERT( ( begin ) >= 0 && ( end ) < array_count( x ) ); \
		ZPL_ASSERT( size_of( value ) == size_of( ( x )[ 0 ] ) );    \
		for ( ZPL_NS( sw ) i = ( begin ); i < ( end ); i++ )        \
		{                                                           \
			x[ i ] = value;                                         \
		}                                                           \
	} while ( 0 )

#define array_remove_at( x, index )                                                                    \
	do                                                                                                 \
	{                                                                                                  \
		ZPL_NS( ArrayHeader )* _ah = ZPL_ARRAY_HEADER( x );                                            \
		ZPL_ASSERT( index < _ah->count );                                                              \
		ZPL_NS( memmove )( x + index, x + index + 1, size_of( x[ 0 ] ) * ( _ah->count - index - 1 ) ); \
		--_ah->count;                                                                                  \
	} while ( 0 )

ZPL_IMPL_INLINE b8 _array_copy_init( void** y, void** x )
{
	if ( ! _array_init_reserve( y, array_allocator( *x ), array_elem_size( *x ), array_capacity( *x ) ) )
		return false;
	memcopy( *y, *x, array_capacity( *x ) * array_elem_size( *x ) );
	array_count( *y ) = array_count( *x );
	return true;
}

#define array_copy_init( y, x ) ZPL_NS( _array_copy_init )( zpl_cast( void** ) & ( y ), zpl_cast( void** ) & ( x ) )

#define array_pop( x )                                  \
	do                                                  \
	{                                                   \
		ZPL_ASSERT( ZPL_ARRAY_HEADER( x )->count > 0 ); \
		ZPL_ARRAY_HEADER( x )->count--;                 \
	} while ( 0 )
#define array_back( x )  x[ ZPL_ARRAY_HEADER( x )->count - 1 ]
#define array_front( x ) x[ 0 ]
#define array_clear( x )                  \
	do                                    \
	{                                     \
		ZPL_ARRAY_HEADER( x )->count = 0; \
	} while ( 0 )

ZPL_IMPL_INLINE b8 _array_resize( void** x, sw new_count )
{
	if ( ZPL_ARRAY_HEADER( *x )->capacity < new_count )
	{
		if ( ! _array_grow( x, new_count ) )
			return false;
	}
	ZPL_ARRAY_HEADER( *x )->count = new_count;
	return true;
}

#define array_resize( x, new_count ) ZPL_NS( _array_resize )( zpl_cast( void** ) & ( x ), ( new_count ) )

ZPL_IMPL_INLINE b8 _array_reserve( void** x, sw new_capacity )
{
	if ( ZPL_ARRAY_HEADER( *x )->capacity < new_capacity )
		return _array_set_capacity( x, new_capacity );
	return true;
}

#define array_reserve( x, new_capacity ) ZPL_NS( _array_reserve )( zpl_cast( void** ) & ( x ), ( new_capacity ) )

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
