// file: header/essentials/collections/buffer.h

////////////////////////////////////////////////////////////////
//
// Fixed Capacity Buffer (POD Types)
//
//
// zpl_buffer(Type) works like zpl_string or zpl_array where the actual type is just a pointer to the first
// element.
//
// Available Procedures for zpl_buffer(Type)
// zpl_buffer_init
// zpl_buffer_free
// zpl_buffer_append
// zpl_buffer_appendv
// zpl_buffer_pop
// zpl_buffer_clear


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct buffer_header
{
	AllocatorInfo backing;
	sw            count;
	sw            capacity;
} buffer_header;

#define buffer( Type ) Type*

#define buffer_make( Type, Name, allocator, cap ) \
	Type* Name;                                   \
	ZPL_NS( buffer_init )( Name, allocator, cap )

#define ZPL_BUFFER_HEADER( x ) ( zpl_cast( ZPL_NS( buffer_header )* )( x ) - 1 )
#define buffer_count( x )      ( ZPL_BUFFER_HEADER( x )->count )
#define buffer_capacity( x )   ( ZPL_BUFFER_HEADER( x )->capacity )
#define buffer_end( x )        ( x + ( buffer_count( x ) - 1 ) )

#define buffer_init( x, allocator, cap )                                                                                                                           \
	do                                                                                                                                                             \
	{                                                                                                                                                              \
		void** nx                    = zpl_cast( void** ) & ( x );                                                                                                 \
		ZPL_NS( buffer_header )* _bh = zpl_cast( ZPL_NS( buffer_header )* ) alloc( ( allocator ), sizeof( ZPL_NS( buffer_header ) ) + ( cap )*size_of( *( x ) ) ); \
		_bh->backing                 = allocator;                                                                                                                  \
		_bh->count                   = 0;                                                                                                                          \
		_bh->capacity                = cap;                                                                                                                        \
		*nx                          = zpl_cast( void* )( _bh + 1 );                                                                                               \
	} while ( 0 )

#define buffer_free( x ) ( ZPL_NS( free )( ZPL_BUFFER_HEADER( x )->backing, ZPL_BUFFER_HEADER( x ) ) )

#define buffer_append( x, item )                 \
	do                                           \
	{                                            \
		( x )[ buffer_count( x )++ ] = ( item ); \
	} while ( 0 )

#define buffer_appendv( x, items, item_count )                                                           \
	do                                                                                                   \
	{                                                                                                    \
		ZPL_ASSERT( size_of( *( items ) ) == size_of( *( x ) ) );                                        \
		ZPL_ASSERT( buffer_count( x ) + item_count <= buffer_capacity( x ) );                            \
		ZPL_NS( memcopy )( &( x )[ buffer_count( x ) ], ( items ), size_of( *( x ) ) * ( item_count ) ); \
		buffer_count( x ) += ( item_count );                                                             \
	} while ( 0 )

#define buffer_copy_init( y, x )                                                                   \
	do                                                                                             \
	{                                                                                              \
		ZPL_NS( buffer_init_reserve )( y, ZPL_NS( buffer_allocator )( x ), buffer_capacity( x ) ); \
		ZPL_NS( memcopy )( y, x, buffer_capacity( x ) * size_of( *x ) );                           \
		buffer_count( y ) = buffer_count( x );                                                     \
	} while ( 0 )

#define buffer_pop( x )                      \
	do                                       \
	{                                        \
		ZPL_ASSERT( buffer_count( x ) > 0 ); \
		buffer_count( x )--;                 \
	} while ( 0 )
#define buffer_clear( x )      \
	do                         \
	{                          \
		buffer_count( x ) = 0; \
	} while ( 0 )

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
