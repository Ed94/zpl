// a_file: header/essentials/memory_custom.h

////////////////////////////////////////////////////////////////
//
// Custom Allocation
//
//

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef enum alloc_type
{
	ZPL_ALLOCATION_ALLOC,
	ZPL_ALLOCATION_FREE,
	ZPL_ALLOCATION_FREE_ALL,
	ZPL_ALLOCATION_RESIZE,
} alloc_type;

// NOTE: This is useful so you can define an allocator of the same type and parameters
#define ZPL_ALLOCATOR_PROC( name ) void* name( void* allocator_data, alloc_type type, sw size, sw alignment, void* old_memory, sw old_size, u64 flags )
typedef ZPL_ALLOCATOR_PROC( allocator_proc );

typedef struct allocator
{
	allocator_proc* proc;
	void*           data;
} allocator;

typedef enum alloc_flag
{
	ZPL_ALLOCATOR_FLAG_CLEAR_TO_ZERO = ZPL_BIT( 0 ),
} alloc_flag;

#ifndef ZPL_DEFAULT_MEMORY_ALIGNMENT
#	define ZPL_DEFAULT_MEMORY_ALIGNMENT ( 2 * size_of( void* ) )
#endif

#ifndef ZPL_DEFAULT_ALLOCATOR_FLAGS
#	define ZPL_DEFAULT_ALLOCATOR_FLAGS ( ZPL_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
#endif

//! Allocate memory with specified alignment.
ZPL_DEF_INLINE void* alloc_align( allocator a, sw size, sw alignment );

//! Allocate memory with default alignment.
ZPL_DEF_INLINE void* alloc( allocator a, sw size );

//! Free allocated memory.
ZPL_DEF_INLINE void free( allocator a, void* ptr );

//! Free all memory allocated by an allocator.
ZPL_DEF_INLINE void free_all( allocator a );

//! Resize an allocated memory.
ZPL_DEF_INLINE void* resize( allocator a, void* ptr, sw old_size, sw new_size );

//! Resize an allocated memory with specified alignment.
ZPL_DEF_INLINE void* resize_align( allocator a, void* ptr, sw old_size, sw new_size, sw alignment );

//! Allocate memory and copy data into it.
ZPL_DEF_INLINE void* alloc_copy( allocator a, void const* src, sw size );

//! Allocate memory with specified alignment and copy data into it.
ZPL_DEF_INLINE void* alloc_copy_align( allocator a, void const* src, sw size, sw alignment );

//! Allocate memory for null-terminated C-String.
ZPL_DEF char* alloc_str( allocator a, char const* str );

//! Allocate memory for C-String with specified size.
ZPL_DEF_INLINE char* alloc_str_len( allocator a, char const* str, sw len );

#ifndef alloc_item

//! Allocate memory for an item.
#	define alloc_item( allocator_, Type ) ( Type* )alloc( allocator_, size_of( Type ) )

//! Allocate memory for an array of items.
#	define alloc_array( allocator_, Type, count ) ( Type* )alloc( allocator_, size_of( Type ) * ( count ) )
#endif

/* heap memory analysis tools */
/* define ZPL_HEAP_ANALYSIS to enable this feature */
/* call heap_stats_init at the beginning of the entry point */
/* you can call heap_stats_check near the end of the execution to validate any possible leaks */
ZPL_DEF void heap_stats_init( void );
ZPL_DEF sw   heap_stats_used_memory( void );
ZPL_DEF sw   heap_stats_alloc_count( void );
ZPL_DEF void heap_stats_check( void );

//! Allocate/Resize memory using default options.

//! Use this if you don't need a "fancy" resize allocation
ZPL_DEF_INLINE void* default_resize_align( allocator a, void* ptr, sw old_size, sw new_size, sw alignment );

//! The heap allocator backed by operating system's memory manager.
ZPL_DEF_INLINE allocator heap_allocator( void );
ZPL_DEF                  ZPL_ALLOCATOR_PROC( heap_allocator_proc );

#ifndef malloc

//! Helper to allocate memory using heap allocator.
#	define malloc( sz ) ZPL_NS alloc( heap_allocator(), sz )

//! Helper to free memory allocated by heap allocator.
#	define mfree( ptr ) ZPL_NS free( heap_allocator(), ptr )

//! Alias to heap allocator.
#	define heap ZPL_NS heap_allocator
#endif

//
// Arena Allocator
//

typedef struct arena
{
	allocator backing;
	void*     physical_start;
	sw        total_size;
	sw        total_allocated;
	sw        temp_count;
} arena;

//! Initialize memory a_arena from existing memory region.
ZPL_DEF_INLINE void arena_init_from_memory( arena* a_arena, void* start, sw size );

//! Initialize memory a_arena using existing memory allocator.
ZPL_DEF_INLINE void arena_init_from_allocator( arena* a_arena, allocator backing, sw size );

//! Initialize memory a_arena within an existing parent memory a_arena.
ZPL_DEF_INLINE void arena_init_sub( arena* a_arena, arena* parent_arena, sw size );

//! Release the memory used by memory a_arena.
ZPL_DEF_INLINE void arena_free( arena* a_arena );


//! Retrieve memory a_arena's aligned allocation address.
ZPL_DEF_INLINE sw arena_alignment_of( arena* a_arena, sw alignment );

//! Retrieve memory a_arena's remaining size.
ZPL_DEF_INLINE sw arena_size_remaining( arena* a_arena, sw alignment );

//! Check whether memory a_arena has any temporary snapshots.
ZPL_DEF_INLINE void arena_check( arena* a_arena );

//! Allocation Types: a_allocator, free_all, resize
ZPL_DEF_INLINE allocator arena_allocator( arena* a_arena );
ZPL_DEF                  ZPL_ALLOCATOR_PROC( arena_allocator_proc );

typedef struct arena_snapshot
{
	arena* a_arena;
	sw     original_count;
} arena_snapshot;

//! Capture a snapshot of used memory in a memory a_arena.
ZPL_DEF_INLINE arena_snapshot arena_snapshot_begin( arena* a_arena );

//! Reset memory a_arena's usage by a captured snapshot.
ZPL_DEF_INLINE void arena_snapshot_end( arena_snapshot tmp_mem );

//
// Pool Allocator
//


typedef struct pool
{
	allocator backing;
	void*     physical_start;
	void*     free_list;
	sw        block_size;
	sw        block_align;
	sw        total_size;
	sw        num_blocks;
} pool;

//! Initialize a_pool allocator.
ZPL_DEF_INLINE void pool_init( pool* a_pool, allocator backing, sw num_blocks, sw block_size );

//! Initialize a_pool allocator with specific block alignment.
ZPL_DEF void pool_init_align( pool* a_pool, allocator backing, sw num_blocks, sw block_size, sw block_align );

//! Release the resources used by a_pool allocator.
ZPL_DEF_INLINE void pool_free( pool* a_pool );

//! Allocation Types: a_allocator, free
ZPL_DEF_INLINE allocator pool_allocator( pool* a_pool );
ZPL_DEF                  ZPL_ALLOCATOR_PROC( pool_allocator_proc );

//
// Scratch Memory Allocator - Ring Buffer Based Arena
//

typedef struct allocation_header_ev
{
	sw size;
} allocation_header_ev;

ZPL_DEF_INLINE allocation_header_ev* allocation_header( void* data );
ZPL_DEF_INLINE void                  allocation_header_fill( allocation_header_ev* header, void* data, sw size );

#if defined( ZPL_ARCH_32_BIT )
#	define ZPL_ISIZE_HIGH_BIT 0x80000000
#elif defined( ZPL_ARCH_64_BIT )
#	define ZPL_ISIZE_HIGH_BIT 0x8000000000000000ll
#else
#	error
#endif

typedef struct scratch_memory
{
	void* physical_start;
	sw    total_size;
	void* alloc_point;
	void* free_point;
} scratch_memory;

//! Initialize ring buffer a_arena.
ZPL_DEF void scratch_memory_init( scratch_memory* s, void* start, sw size );

//! Check whether ring buffer a_arena is in use.
ZPL_DEF b32 scratch_memory_is_in_use( scratch_memory* s, void* ptr );

//! Allocation Types: a_allocator, free, free_all, resize
ZPL_DEF allocator scratch_allocator( scratch_memory* s );
ZPL_DEF           ZPL_ALLOCATOR_PROC( scratch_allocator_proc );

//
// Stack Memory Allocator
//


typedef struct stack_memory
{
	allocator backing;

	void* physical_start;
	uw    total_size;
	uw    allocated;
} stack_memory;

//! Initialize stack allocator from existing memory.
ZPL_DEF_INLINE void stack_memory_init_from_memory( stack_memory* s, void* start, sw size );

//! Initialize stack allocator using existing memory allocator.
ZPL_DEF_INLINE void stack_memory_init( stack_memory* s, allocator backing, sw size );

//! Check whether stack allocator is in use.
ZPL_DEF_INLINE b32 stack_memory_is_in_use( stack_memory* s, void* ptr );

//! Release the resources used by stack allocator.
ZPL_DEF_INLINE void stack_memory_free( stack_memory* s );

//! Allocation Types: a_allocator, free, free_all
ZPL_DEF_INLINE allocator stack_allocator( stack_memory* s );
ZPL_DEF                  ZPL_ALLOCATOR_PROC( stack_allocator_proc );

/* inlines */

ZPL_IMPL_INLINE void* alloc_align( allocator a, sw size, sw alignment )
{
	return a.proc( a.data, ZPL_ALLOCATION_ALLOC, size, alignment, NULL, 0, ZPL_DEFAULT_ALLOCATOR_FLAGS );
}

ZPL_IMPL_INLINE void* alloc( allocator a, sw size )
{
	return alloc_align( a, size, ZPL_DEFAULT_MEMORY_ALIGNMENT );
}

ZPL_IMPL_INLINE void free( allocator a, void* ptr )
{
	if ( ptr != NULL )
		a.proc( a.data, ZPL_ALLOCATION_FREE, 0, 0, ptr, 0, ZPL_DEFAULT_ALLOCATOR_FLAGS );
}

ZPL_IMPL_INLINE void free_all( allocator a )
{
	a.proc( a.data, ZPL_ALLOCATION_FREE_ALL, 0, 0, NULL, 0, ZPL_DEFAULT_ALLOCATOR_FLAGS );
}

ZPL_IMPL_INLINE void* resize( allocator a, void* ptr, sw old_size, sw new_size )
{
	return resize_align( a, ptr, old_size, new_size, ZPL_DEFAULT_MEMORY_ALIGNMENT );
}

ZPL_IMPL_INLINE void* resize_align( allocator a, void* ptr, sw old_size, sw new_size, sw alignment )
{
	return a.proc( a.data, ZPL_ALLOCATION_RESIZE, new_size, alignment, ptr, old_size, ZPL_DEFAULT_ALLOCATOR_FLAGS );
}

ZPL_IMPL_INLINE void* alloc_copy( allocator a, void const* src, sw size )
{
	return memcopy( alloc( a, size ), src, size );
}

ZPL_IMPL_INLINE void* alloc_copy_align( allocator a, void const* src, sw size, sw alignment )
{
	return memcopy( alloc_align( a, size, alignment ), src, size );
}

ZPL_IMPL_INLINE char* alloc_str_len( allocator a, char const* str, sw len )
{
	char* result;
	result = zpl_cast( char* ) alloc( a, len + 1 );
	memmove( result, str, len );
	result[ len ] = '\0';
	return result;
}

ZPL_IMPL_INLINE void* default_resize_align( allocator a, void* old_memory, sw old_size, sw new_size, sw alignment )
{
	if ( ! old_memory )
		return alloc_align( a, new_size, alignment );

	if ( new_size == 0 )
	{
		free( a, old_memory );
		return NULL;
	}

	if ( new_size < old_size )
		new_size = old_size;

	if ( old_size == new_size )
	{
		return old_memory;
	}
	else
	{
		void* new_memory = alloc_align( a, new_size, alignment );
		if ( ! new_memory )
			return NULL;
		memmove( new_memory, old_memory, min( new_size, old_size ) );
		free( a, old_memory );
		return new_memory;
	}
}

//
// Heap Allocator
//

ZPL_IMPL_INLINE allocator heap_allocator( void )
{
	allocator a;
	a.proc = heap_allocator_proc;
	a.data = NULL;
	return a;
}

//
// Arena Allocator
//

ZPL_IMPL_INLINE void arena_init_from_memory( arena* a_arena, void* start, sw size )
{
	a_arena->backing.proc    = NULL;
	a_arena->backing.data    = NULL;
	a_arena->physical_start  = start;
	a_arena->total_size      = size;
	a_arena->total_allocated = 0;
	a_arena->temp_count      = 0;
}

ZPL_IMPL_INLINE void arena_init_from_allocator( arena* a_arena, allocator backing, sw size )
{
	a_arena->backing         = backing;
	a_arena->physical_start  = alloc( backing, size );    // NOTE: Uses default alignment
	a_arena->total_size      = size;
	a_arena->total_allocated = 0;
	a_arena->temp_count      = 0;
}

ZPL_IMPL_INLINE void arena_init_sub( arena* a_arena, arena* parent_arena, sw size )
{
	arena_init_from_allocator( a_arena, arena_allocator( parent_arena ), size );
}

ZPL_IMPL_INLINE void arena_free( arena* a_arena )
{
	if ( a_arena->backing.proc )
	{
		free( a_arena->backing, a_arena->physical_start );
		a_arena->physical_start = NULL;
	}
}

ZPL_IMPL_INLINE sw arena_alignment_of( arena* a_arena, sw alignment )
{
	sw alignment_offset, result_pointer, mask;
	ZPL_ASSERT( is_power_of_two( alignment ) );

	alignment_offset = 0;
	result_pointer   = zpl_cast( sw ) a_arena->physical_start + a_arena->total_allocated;
	mask             = alignment - 1;
	if ( result_pointer & mask )
		alignment_offset = alignment - ( result_pointer & mask );

	return alignment_offset;
}

ZPL_IMPL_INLINE sw arena_size_remaining( arena* a_arena, sw alignment )
{
	sw result = a_arena->total_size - ( a_arena->total_allocated + arena_alignment_of( a_arena, alignment ) );
	return result;
}

ZPL_IMPL_INLINE void arena_check( arena* a_arena )
{
	ZPL_ASSERT( a_arena->temp_count == 0 );
}

ZPL_IMPL_INLINE allocator arena_allocator( arena* a_arena )
{
	allocator allocator;
	allocator.proc = arena_allocator_proc;
	allocator.data = a_arena;
	return allocator;
}

ZPL_IMPL_INLINE arena_snapshot arena_snapshot_begin( arena* a_arena )
{
	arena_snapshot tmp;
	tmp.a_arena        = a_arena;
	tmp.original_count = a_arena->total_allocated;
	a_arena->temp_count++;
	return tmp;
}

ZPL_IMPL_INLINE void arena_snapshot_end( arena_snapshot tmp )
{
	ZPL_ASSERT( tmp.a_arena->total_allocated >= tmp.original_count );
	ZPL_ASSERT( tmp.a_arena->temp_count > 0 );
	tmp.a_arena->total_allocated = tmp.original_count;
	tmp.a_arena->temp_count--;
}

//
// Pool Allocator
//

ZPL_IMPL_INLINE void pool_init( pool* a_pool, allocator backing, sw num_blocks, sw block_size )
{
	pool_init_align( a_pool, backing, num_blocks, block_size, ZPL_DEFAULT_MEMORY_ALIGNMENT );
}

ZPL_IMPL_INLINE void pool_free( pool* a_pool )
{
	if ( a_pool->backing.proc )
	{
		free( a_pool->backing, a_pool->physical_start );
	}
}

ZPL_IMPL_INLINE allocator pool_allocator( pool* a_pool )
{
	allocator allocator;
	allocator.proc = pool_allocator_proc;
	allocator.data = a_pool;
	return allocator;
}

ZPL_IMPL_INLINE allocation_header_ev* allocation_header( void* data )
{
	sw* p = zpl_cast( sw* ) data;
	while ( p[ -1 ] == zpl_cast( sw )( -1 ) )
		p--;
	return zpl_cast( allocation_header_ev* ) p - 1;
}

ZPL_IMPL_INLINE void allocation_header_fill( allocation_header_ev* header, void* data, sw size )
{
	sw* ptr;
	header->size = size;
	ptr          = zpl_cast( sw* )( header + 1 );
	while ( zpl_cast( void* ) ptr < data )
		*ptr++ = zpl_cast( sw )( -1 );
}

//
// Stack Memory Allocator
//

#define ZPL_STACK_ALLOC_OFFSET sizeof( ZPL_NS u64 )
ZPL_STATIC_ASSERT( ZPL_STACK_ALLOC_OFFSET == 8, "ZPL_STACK_ALLOC_OFFSET != 8" );

ZPL_IMPL_INLINE void stack_memory_init_from_memory( stack_memory* s, void* start, sw size )
{
	s->physical_start = start;
	s->total_size     = size;
	s->allocated      = 0;
}

ZPL_IMPL_INLINE void stack_memory_init( stack_memory* s, allocator backing, sw size )
{
	s->backing        = backing;
	s->physical_start = alloc( backing, size );
	s->total_size     = size;
	s->allocated      = 0;
}

ZPL_IMPL_INLINE b32 stack_memory_is_in_use( stack_memory* s, void* ptr )
{
	if ( s->allocated == 0 )
		return false;

	if ( ptr > s->physical_start && ptr < pointer_add( s->physical_start, s->total_size ) )
	{
		return true;
	}

	return false;
}

ZPL_IMPL_INLINE void stack_memory_free( stack_memory* s )
{
	if ( s->backing.proc )
	{
		free( s->backing, s->physical_start );
		s->physical_start = NULL;
	}
}

ZPL_IMPL_INLINE allocator stack_allocator( stack_memory* s )
{
	allocator a;
	a.proc = stack_allocator_proc;
	a.data = s;
	return a;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
