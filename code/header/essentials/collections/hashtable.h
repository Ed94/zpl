// a_file: header/essentials/collections/hashtable.h

/** @a_file hashtable.c
@brief Instantiated hash table
@defgroup hashtable Instantiated hash table


 This is an attempt to implement a templated hash table
 NOTE: The key is always a u64 for simplicity and you will _probably_ _never_ need anything bigger.

 Hash table type and function declaration, call: ZPL_TABLE_DECLARE(PREFIX, NAME, FUNC, VALUE)
 Hash table function definitions, call: ZPL_TABLE_DEFINE(NAME, FUNC, VALUE)

     PREFIX  - a prefix for function prototypes e.g. extern, static, etc.
     NAME    - Name of the Hash Table
     FUNC    - the name will prefix function names
     VALUE   - the type of the value to be stored

    tablename_init(NAME * h, allocator a);
    tablename_destroy(NAME * h);
    tablename_get(NAME * h, u64 key);
    tablename_set(NAME * h, u64 key, VALUE value);
    tablename_grow(NAME * h);
    tablename_map(NAME * h, void (*map_proc)(u64 key, VALUE value))
    tablename_map_mut(NAME * h, void (*map_proc)(u64 key, VALUE * value))
    tablename_rehash(NAME * h, sw new_count);
    tablename_remove(NAME * h, u64 key);

 @{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct hash_table_find_result
{
	sw hash_index;
	sw entry_prev;
	sw entry_index;
} hash_table_find_result;

/**
 * Combined macro for a quick delcaration + definition
 */

#define ZPL_TABLE( PREFIX, NAME, FUNC, VALUE )                                                                                                                                     \
	ZPL_TABLE_DECLARE( PREFIX, NAME, FUNC, VALUE );                                                                                                                                \
	ZPL_TABLE_DEFINE( NAME, FUNC, VALUE );

/**
 * Table delcaration macro that generates the interface
 */

#define ZPL_TABLE_DECLARE( PREFIX, NAME, FUNC, VALUE )                                                                                                                             \
	typedef struct ZPL_JOIN2( NAME, Entry )                                                                                                                                        \
	{                                                                                                                                                                              \
		u64   key;                                                                                                                                                                 \
		sw    next;                                                                                                                                                                \
		VALUE value;                                                                                                                                                               \
	} ZPL_JOIN2( NAME, Entry );                                                                                                                                                    \
                                                                                                                                                                                   \
	typedef struct NAME                                                                                                                                                            \
	{                                                                                                                                                                              \
		zpl_array( sw ) hashes;                                                                                                                                                    \
		zpl_array( ZPL_JOIN2( NAME, Entry ) ) entries;                                                                                                                             \
	} NAME;                                                                                                                                                                        \
                                                                                                                                                                                   \
	PREFIX void   ZPL_JOIN2( FUNC, init )( NAME * h, allocator a );                                                                                                                \
	PREFIX void   ZPL_JOIN2( FUNC, destroy )( NAME * h );                                                                                                                          \
	PREFIX void   ZPL_JOIN2( FUNC, clear )( NAME * h );                                                                                                                            \
	PREFIX VALUE* ZPL_JOIN2( FUNC, get )( NAME * h, u64 key );                                                                                                                     \
	PREFIX sw     ZPL_JOIN2( FUNC, slot )( NAME * h, u64 key );                                                                                                                    \
	PREFIX void   ZPL_JOIN2( FUNC, set )( NAME * h, u64 key, VALUE value );                                                                                                        \
	PREFIX void   ZPL_JOIN2( FUNC, grow )( NAME * h );                                                                                                                             \
	PREFIX void   ZPL_JOIN2( FUNC, rehash )( NAME * h, sw new_count );                                                                                                             \
	PREFIX void   ZPL_JOIN2( FUNC, rehash_fast )( NAME * h );                                                                                                                      \
	PREFIX void   ZPL_JOIN2( FUNC, map )( NAME * h, void ( *map_proc )( u64 key, VALUE value ) );                                                                                  \
	PREFIX void   ZPL_JOIN2( FUNC, map_mut )( NAME * h, void ( *map_proc )( u64 key, VALUE * value ) );                                                                            \
	PREFIX void   ZPL_JOIN2( FUNC, remove )( NAME * h, u64 key );                                                                                                                  \
	PREFIX void   ZPL_JOIN2( FUNC, remove_entry )( NAME * h, sw idx );

/**
 * Table definition interfaces that generates the implementation
 */

#define ZPL_TABLE_DEFINE( NAME, FUNC, VALUE )                                                                                                                                      \
	void ZPL_JOIN2( FUNC, init )( NAME * h, allocator a )                                                                                                                          \
	{                                                                                                                                                                              \
		array_init( h->hashes, a );                                                                                                                                                \
		array_init( h->entries, a );                                                                                                                                               \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, destroy )( NAME * h )                                                                                                                                    \
	{                                                                                                                                                                              \
		if ( h->entries )                                                                                                                                                          \
			array_free( h->entries );                                                                                                                                              \
		if ( h->hashes )                                                                                                                                                           \
			array_free( h->hashes );                                                                                                                                               \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, clear )( NAME * h )                                                                                                                                      \
	{                                                                                                                                                                              \
		for ( int i = 0; i < array_count( h->hashes ); i++ )                                                                                                                       \
			h->hashes[ i ] = -1;                                                                                                                                                   \
		array_clear( h->entries );                                                                                                                                                 \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	sw ZPL_JOIN2( FUNC, slot )( NAME * h, u64 key )                                                                                                                                \
	{                                                                                                                                                                              \
		for ( sw i = 0; i < array_count( h->entries ); i++ )                                                                                                                       \
		{                                                                                                                                                                          \
			if ( h->entries[ i ].key == key )                                                                                                                                      \
			{                                                                                                                                                                      \
				return i;                                                                                                                                                          \
			}                                                                                                                                                                      \
		}                                                                                                                                                                          \
		return -1;                                                                                                                                                                 \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	internal sw ZPL_JOIN2( FUNC, _add_entry )( NAME * h, u64 key )                                                                                                                 \
	{                                                                                                                                                                              \
		sw index;                                                                                                                                                                  \
		ZPL_JOIN2( NAME, Entry ) e = { 0 };                                                                                                                                        \
		e.key                      = key;                                                                                                                                          \
		e.next                     = -1;                                                                                                                                           \
		index                      = array_count( h->entries );                                                                                                                    \
		array_append( h->entries, e );                                                                                                                                             \
		return index;                                                                                                                                                              \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	internal hash_table_find_result ZPL_JOIN2( FUNC, _find )( NAME * h, u64 key )                                                                                                  \
	{                                                                                                                                                                              \
		hash_table_find_result r = { -1, -1, -1 };                                                                                                                                 \
		if ( array_count( h->hashes ) > 0 )                                                                                                                                        \
		{                                                                                                                                                                          \
			r.hash_index  = key % array_count( h->hashes );                                                                                                                        \
			r.entry_index = h->hashes[ r.hash_index ];                                                                                                                             \
			while ( r.entry_index >= 0 )                                                                                                                                           \
			{                                                                                                                                                                      \
				if ( h->entries[ r.entry_index ].key == key )                                                                                                                      \
					return r;                                                                                                                                                      \
				r.entry_prev  = r.entry_index;                                                                                                                                     \
				r.entry_index = h->entries[ r.entry_index ].next;                                                                                                                  \
			}                                                                                                                                                                      \
		}                                                                                                                                                                          \
		return r;                                                                                                                                                                  \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	internal b32 ZPL_JOIN2( FUNC, _full )( NAME * h )                                                                                                                              \
	{                                                                                                                                                                              \
		return 0.75f * array_count( h->hashes ) < array_count( h->entries );                                                                                                       \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, grow )( NAME * h )                                                                                                                                       \
	{                                                                                                                                                                              \
		sw new_count = ZPL_ARRAY_GROW_FORMULA( array_count( h->entries ) );                                                                                                        \
		ZPL_JOIN2( FUNC, rehash )( h, new_count );                                                                                                                                 \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, rehash )( NAME * h, sw new_count )                                                                                                                       \
	{                                                                                                                                                                              \
		sw   i, j;                                                                                                                                                                 \
		NAME nh = { 0 };                                                                                                                                                           \
		ZPL_JOIN2( FUNC, init )( &nh, array_allocator( h->hashes ) );                                                                                                              \
		array_resize( nh.hashes, new_count );                                                                                                                                      \
		array_reserve( nh.entries, array_count( h->entries ) );                                                                                                                    \
		for ( i = 0; i < new_count; i++ )                                                                                                                                          \
			nh.hashes[ i ] = -1;                                                                                                                                                   \
		for ( i = 0; i < array_count( h->entries ); i++ )                                                                                                                          \
		{                                                                                                                                                                          \
			ZPL_JOIN2( NAME, Entry ) * e;                                                                                                                                          \
			hash_table_find_result fr;                                                                                                                                             \
			if ( array_count( nh.hashes ) == 0 )                                                                                                                                   \
				ZPL_JOIN2( FUNC, grow )( &nh );                                                                                                                                    \
			e  = &h->entries[ i ];                                                                                                                                                 \
			fr = ZPL_JOIN2( FUNC, _find )( &nh, e->key );                                                                                                                          \
			j  = ZPL_JOIN2( FUNC, _add_entry )( &nh, e->key );                                                                                                                     \
			if ( fr.entry_prev < 0 )                                                                                                                                               \
				nh.hashes[ fr.hash_index ] = j;                                                                                                                                    \
			else                                                                                                                                                                   \
				nh.entries[ fr.entry_prev ].next = j;                                                                                                                              \
			nh.entries[ j ].next  = fr.entry_index;                                                                                                                                \
			nh.entries[ j ].value = e->value;                                                                                                                                      \
		}                                                                                                                                                                          \
		ZPL_JOIN2( FUNC, destroy )( h );                                                                                                                                           \
		h->hashes  = nh.hashes;                                                                                                                                                    \
		h->entries = nh.entries;                                                                                                                                                   \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, rehash_fast )( NAME * h )                                                                                                                                \
	{                                                                                                                                                                              \
		sw i;                                                                                                                                                                      \
		for ( i = 0; i < array_count( h->entries ); i++ )                                                                                                                          \
			h->entries[ i ].next = -1;                                                                                                                                             \
		for ( i = 0; i < array_count( h->hashes ); i++ )                                                                                                                           \
			h->hashes[ i ] = -1;                                                                                                                                                   \
		for ( i = 0; i < array_count( h->entries ); i++ )                                                                                                                          \
		{                                                                                                                                                                          \
			ZPL_JOIN2( NAME, Entry ) * e;                                                                                                                                          \
			hash_table_find_result fr;                                                                                                                                             \
			e  = &h->entries[ i ];                                                                                                                                                 \
			fr = ZPL_JOIN2( FUNC, _find )( h, e->key );                                                                                                                            \
			if ( fr.entry_prev < 0 )                                                                                                                                               \
				h->hashes[ fr.hash_index ] = i;                                                                                                                                    \
			else                                                                                                                                                                   \
				h->entries[ fr.entry_prev ].next = i;                                                                                                                              \
		}                                                                                                                                                                          \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	VALUE* ZPL_JOIN2( FUNC, get )( NAME * h, u64 key )                                                                                                                             \
	{                                                                                                                                                                              \
		sw index = ZPL_JOIN2( FUNC, _find )( h, key ).entry_index;                                                                                                                 \
		if ( index >= 0 )                                                                                                                                                          \
			return &h->entries[ index ].value;                                                                                                                                     \
		return NULL;                                                                                                                                                               \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, remove )( NAME * h, u64 key )                                                                                                                            \
	{                                                                                                                                                                              \
		hash_table_find_result fr = ZPL_JOIN2( FUNC, _find )( h, key );                                                                                                            \
		if ( fr.entry_index >= 0 )                                                                                                                                                 \
		{                                                                                                                                                                          \
			array_remove_at( h->entries, fr.entry_index );                                                                                                                         \
			ZPL_JOIN2( FUNC, rehash_fast )( h );                                                                                                                                   \
		}                                                                                                                                                                          \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, remove_entry )( NAME * h, sw idx )                                                                                                                       \
	{                                                                                                                                                                              \
		array_remove_at( h->entries, idx );                                                                                                                                        \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, map )( NAME * h, void ( *map_proc )( u64 key, VALUE value ) )                                                                                            \
	{                                                                                                                                                                              \
		ZPL_ASSERT_NOT_NULL( h );                                                                                                                                                  \
		ZPL_ASSERT_NOT_NULL( map_proc );                                                                                                                                           \
		for ( sw i = 0; i < array_count( h->entries ); ++i )                                                                                                                       \
		{                                                                                                                                                                          \
			map_proc( h->entries[ i ].key, h->entries[ i ].value );                                                                                                                \
		}                                                                                                                                                                          \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, map_mut )( NAME * h, void ( *map_proc )( u64 key, VALUE * value ) )                                                                                      \
	{                                                                                                                                                                              \
		ZPL_ASSERT_NOT_NULL( h );                                                                                                                                                  \
		ZPL_ASSERT_NOT_NULL( map_proc );                                                                                                                                           \
		for ( sw i = 0; i < array_count( h->entries ); ++i )                                                                                                                       \
		{                                                                                                                                                                          \
			map_proc( h->entries[ i ].key, &h->entries[ i ].value );                                                                                                               \
		}                                                                                                                                                                          \
	}                                                                                                                                                                              \
                                                                                                                                                                                   \
	void ZPL_JOIN2( FUNC, set )( NAME * h, u64 key, VALUE value )                                                                                                                  \
	{                                                                                                                                                                              \
		sw                     index;                                                                                                                                              \
		hash_table_find_result fr;                                                                                                                                                 \
		if ( array_count( h->hashes ) == 0 )                                                                                                                                       \
			ZPL_JOIN2( FUNC, grow )( h );                                                                                                                                          \
		fr = ZPL_JOIN2( FUNC, _find )( h, key );                                                                                                                                   \
		if ( fr.entry_index >= 0 )                                                                                                                                                 \
		{                                                                                                                                                                          \
			index = fr.entry_index;                                                                                                                                                \
		}                                                                                                                                                                          \
		else                                                                                                                                                                       \
		{                                                                                                                                                                          \
			index = ZPL_JOIN2( FUNC, _add_entry )( h, key );                                                                                                                       \
			if ( fr.entry_prev >= 0 )                                                                                                                                              \
			{                                                                                                                                                                      \
				h->entries[ fr.entry_prev ].next = index;                                                                                                                          \
			}                                                                                                                                                                      \
			else                                                                                                                                                                   \
			{                                                                                                                                                                      \
				h->hashes[ fr.hash_index ] = index;                                                                                                                                \
			}                                                                                                                                                                      \
		}                                                                                                                                                                          \
		h->entries[ index ].value = value;                                                                                                                                         \
		if ( ZPL_JOIN2( FUNC, _full )( h ) )                                                                                                                                       \
			ZPL_JOIN2( FUNC, grow )( h );                                                                                                                                          \
	}

//! @}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
