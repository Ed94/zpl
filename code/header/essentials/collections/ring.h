// a_file: header/essentials/collections/ring.h

////////////////////////////////////////////////////////////////
//
// Instantiated Circular buffer
//

/*
Buffer type and function declaration, call: ZPL_RING_DECLARE(PREFIX, FUNC, VALUE)
Buffer function definitions, call: ZPL_RING_DEFINE(PREFIX, FUNC, VALUE)

PREFIX  - a prefix for function prototypes e.g. extern, static, etc.
     FUNC    - the name will prefix function names
     VALUE   - the type of the value to be stored

funcname_init(VALUE * pad, allocator a, sw max_size)
 funcname_free(VALUE * pad)
 funcname_full(VALUE * pad)
 funcname_empty(VALUE * pad)
 funcname_append(VALUE * pad, type data)
 funcname_append_array(VALUE * pad, zpl_array(type) data)
 funcname_get(VALUE * pad)
funcname_get_array(VALUE * pad, uw max_size, allocator a)
*/
ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#define ZPL_RING( PREFIX, FUNC, VALUE )                                                                                                                                            \
	ZPL_RING_DECLARE( PREFIX, FUNC, VALUE );                                                                                                                                       \
	ZPL_RING_DEFINE( FUNC, VALUE );

#define ZPL_RING_DECLARE( prefix, func, type )                                                                                                                                     \
	typedef struct                                                                                                                                                                 \
	{                                                                                                                                                                              \
		allocator backing;                                                                                                                                                         \
		buffer( type ) buf;                                                                                                                                                        \
		uw head, tail;                                                                                                                                                             \
		uw capacity;                                                                                                                                                               \
	} ZPL_JOIN2( func, type );                                                                                                                                                     \
                                                                                                                                                                                   \
	prefix void  ZPL_JOIN2( func, init )( ZPL_JOIN2( func, type ) * pad, allocator a, sw max_size );                                                                               \
	prefix void  ZPL_JOIN2( func, free )( ZPL_JOIN2( func, type ) * pad );                                                                                                         \
	prefix b32   ZPL_JOIN2( func, full )( ZPL_JOIN2( func, type ) * pad );                                                                                                         \
	prefix b32   ZPL_JOIN2( func, empty )( ZPL_JOIN2( func, type ) * pad );                                                                                                        \
	prefix void  ZPL_JOIN2( func, append )( ZPL_JOIN2( func, type ) * pad, type data );                                                                                            \
	prefix void  ZPL_JOIN2( func, append_array )( ZPL_JOIN2( func, type ) * pad, zpl_array( type ) data );                                                                         \
	prefix type* ZPL_JOIN2( func, get )( ZPL_JOIN2( func, type ) * pad );                                                                                                          \
	prefix       zpl_array( type ) ZPL_JOIN2( func, get_array )( ZPL_JOIN2( func, type ) * pad, uw max_size, allocator a );

#define ZPL_RING_DEFINE( func, type )                                                                                                                                              \
	void ZPL_JOIN2( func, init )( ZPL_JOIN2( func, type ) * pad, allocator a, sw max_size )                                                                                        \
	{                                                                                                                                                                              \
		ZPL_JOIN2( func, type ) pad_ = { 0 };                                                                                                                                      \
		*pad                         = pad_;                                                                                                                                       \
                                                                                                                                                                                   \
		pad->backing = a;                                                                                                                                                          \
		buffer_init( pad->buf, a, max_size + 1 );                                                                                                                                  \
		pad->capacity = max_size + 1;                                                                                                                                              \
		pad->head = pad->tail = 0;                                                                                                                                                 \
	}                                                                                                                                                                              \
	void ZPL_JOIN2( func, free )( ZPL_JOIN2( func, type ) * pad )                                                                                                                  \
	{                                                                                                                                                                              \
		buffer_free( pad->buf );                                                                                                                                                   \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	b32 ZPL_JOIN2( func, full )( ZPL_JOIN2( func, type ) * pad )                                                                                                                   \
	{                                                                                                                                                                              \
		return ( ( pad->head + 1 ) % pad->capacity ) == pad->tail;                                                                                                                 \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	b32 ZPL_JOIN2( func, empty )( ZPL_JOIN2( func, type ) * pad )                                                                                                                  \
	{                                                                                                                                                                              \
		return pad->head == pad->tail;                                                                                                                                             \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( func, append )( ZPL_JOIN2( func, type ) * pad, type data )                                                                                                     \
	{                                                                                                                                                                              \
		pad->buf[ pad->head ] = data;                                                                                                                                              \
		pad->head             = ( pad->head + 1 ) % pad->capacity;                                                                                                                 \
                                                                                                                                                                                   \
		if ( pad->head == pad->tail )                                                                                                                                              \
		{                                                                                                                                                                          \
			pad->tail = ( pad->tail + 1 ) % pad->capacity;                                                                                                                         \
		}                                                                                                                                                                          \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( func, append_array )( ZPL_JOIN2( func, type ) * pad, zpl_array( type ) data )                                                                                  \
	{                                                                                                                                                                              \
		uw c = array_count( data );                                                                                                                                                \
		for ( uw i = 0; i < c; ++i )                                                                                                                                               \
		{                                                                                                                                                                          \
			ZPL_JOIN2( func, append )( pad, data[ i ] );                                                                                                                           \
		}                                                                                                                                                                          \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	type* ZPL_JOIN2( func, get )( ZPL_JOIN2( func, type ) * pad )                                                                                                                  \
	{                                                                                                                                                                              \
		if ( ZPL_JOIN2( func, empty )( pad ) )                                                                                                                                     \
		{                                                                                                                                                                          \
			return NULL;                                                                                                                                                           \
		}                                                                                                                                                                          \
                                                                                                                                                                                   \
		type* data = &pad->buf[ pad->tail ];                                                                                                                                       \
		pad->tail  = ( pad->tail + 1 ) % pad->capacity;                                                                                                                            \
                                                                                                                                                                                   \
		return data;                                                                                                                                                               \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	zpl_array( type ) ZPL_JOIN2( func, get_array )( ZPL_JOIN2( func, type ) * pad, uw max_size, allocator a )                                                                      \
	{                                                                                                                                                                              \
		zpl_array( type ) vals = 0;                                                                                                                                                \
		array_init( vals, a );                                                                                                                                                     \
		while ( --max_size && ! ZPL_JOIN2( func, empty )( pad ) )                                                                                                                  \
		{                                                                                                                                                                          \
			array_append( vals, *ZPL_JOIN2( func, get )( pad ) );                                                                                                                  \
		}                                                                                                                                                                          \
		return vals;                                                                                                                                                               \
	}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
