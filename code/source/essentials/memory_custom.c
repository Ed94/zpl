// file: source/essentials/memory_custom.c

#ifndef _IOSC11_SOURCE
#define _IOSC11_SOURCE
#endif

#include <stdlib.h>

#if defined( ZPL_SYSTEM_WINDOWS )
#include <malloc.h>
#endif

// include errno.h for MinGW
#if defined( ZPL_COMPILER_GCC ) || ( defined( ZPL_COMPILER_TINYC ) && defined( ZPL_SYSTEM_WINDOWS ) )
#include <errno.h>
#endif

#if defined( ZPL_COMPILER_MINGW )
#ifdef __MINGW32__
#define _aligned_malloc __mingw_aligned_malloc
#define _aligned_free   __mingw_aligned_free
#endif // MINGW
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

char* alloc_str( zpl_allocator a, char const * str )
{
	return alloc_str_len( a, str, zpl__strlen( str ) );
}

////////////////////////////////////////////////////////////////
//
// Custom Allocation
//
//

//
// Heap Allocator
//

#define ZPL_HEAP_STATS_MAGIC 0xDEADC0DE

typedef struct zpl__heap_stats
{
	u32 magic;
	sw  used_memory;
	sw  alloc_count;
} zpl__heap_stats;

global zpl__heap_stats zpl__heap_stats_info;

void heap_stats_init( void )
{
	zero_item( &zpl__heap_stats_info );
	zpl__heap_stats_info.magic = ZPL_HEAP_STATS_MAGIC;
}
sw heap_stats_used_memory( void )
{
	ZPL_ASSERT_MSG( zpl__heap_stats_info.magic == ZPL_HEAP_STATS_MAGIC, "heap_stats is not initialised yet, call heap_stats_init first!" );
	return zpl__heap_stats_info.used_memory;
}
sw heap_stats_alloc_count( void )
{
	ZPL_ASSERT_MSG( zpl__heap_stats_info.magic == ZPL_HEAP_STATS_MAGIC, "heap_stats is not initialised yet, call heap_stats_init first!" );
	return zpl__heap_stats_info.alloc_count;
}
void heap_stats_check( void )
{
	ZPL_ASSERT_MSG( zpl__heap_stats_info.magic == ZPL_HEAP_STATS_MAGIC, "heap_stats is not initialised yet, call heap_stats_init first!" );
	ZPL_ASSERT( zpl__heap_stats_info.used_memory == 0 );
	ZPL_ASSERT( zpl__heap_stats_info.alloc_count == 0 );
}

typedef struct zpl__heap_alloc_info
{
	sw    size;
	void* physical_start;
} zpl__heap_alloc_info;

ZPL_ALLOCATOR_PROC( heap_allocator_proc )
{
	void* ptr = NULL;
	unused( allocator_data );
	unused( old_size );
	if ( ! alignment )
		alignment = ZPL_DEFAULT_MEMORY_ALIGNMENT;

#ifdef ZPL_HEAP_ANALYSIS
	sw alloc_info_size      = size_of( zpl__heap_alloc_info );
	sw alloc_info_remainder = ( alloc_info_size % alignment );
	sw track_size           = max( alloc_info_size, alignment ) + alloc_info_remainder;
	switch ( type )
	{
		case ZPL_ALLOCATION_FREE :
			{
				if ( ! old_memory )
					break;
				zpl__heap_alloc_info* alloc_info = zpl_cast( zpl__heap_alloc_info* ) old_memory - 1;
				zpl__heap_stats_info.used_memory -= alloc_info->size;
				zpl__heap_stats_info.alloc_count--;
				old_memory = alloc_info->physical_start;
			}
			break;
		case ZPL_ALLOCATION_ALLOC :
			{
				size += track_size;
			}
			break;
		default :
			break;
	}
#endif

	switch ( type )
	{
#if defined( ZPL_COMPILER_MSVC ) || ( defined( ZPL_COMPILER_GCC ) && defined( ZPL_SYSTEM_WINDOWS ) ) || ( defined( ZPL_COMPILER_TINYC ) && defined( ZPL_SYSTEM_WINDOWS ) )
		case ZPL_ALLOCATION_ALLOC :
			ptr = _aligned_malloc( size, alignment );
			if ( flags & ZPL_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
				zero_size( ptr, size );
			break;
		case ZPL_ALLOCATION_FREE :
			_aligned_free( old_memory );
			break;
		case ZPL_ALLOCATION_RESIZE :
			{
				zpl_allocator a = heap_allocator();
				ptr             = default_resize_align( a, old_memory, old_size, size, alignment );
			}
			break;

#elif defined( ZPL_SYSTEM_LINUX ) && ! defined( ZPL_CPU_ARM ) && ! defined( ZPL_COMPILER_TINYC )
		case ZPL_ALLOCATION_ALLOC :
			{
				ptr = aligned_alloc( alignment, ( size + alignment - 1 ) & ~( alignment - 1 ) );

				if ( flags & ZPL_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
				{
					zero_size( ptr, size );
				}
			}
			break;

		case ZPL_ALLOCATION_FREE :
			{
				free( old_memory );
			}
			break;

		case ZPL_ALLOCATION_RESIZE :
			{
				zpl_allocator a = heap_allocator();
				ptr             = default_resize_align( a, old_memory, old_size, size, alignment );
			}
			break;
#else
		case ZPL_ALLOCATION_ALLOC :
			{
				posix_memalign( &ptr, alignment, size );

				if ( flags & ZPL_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
				{
					zero_size( ptr, size );
				}
			}
			break;

		case ZPL_ALLOCATION_FREE :
			{
				free( old_memory );
			}
			break;

		case ZPL_ALLOCATION_RESIZE :
			{
				zpl_allocator a = heap_allocator();
				ptr             = default_resize_align( a, old_memory, old_size, size, alignment );
			}
			break;
#endif

		case ZPL_ALLOCATION_FREE_ALL :
			break;
	}

#ifdef ZPL_HEAP_ANALYSIS
	if ( type == ZPL_ALLOCATION_ALLOC )
	{
		zpl__heap_alloc_info* alloc_info = zpl_cast( zpl__heap_alloc_info* )( zpl_cast( char* ) ptr + alloc_info_remainder );
		zero_item( alloc_info );
		alloc_info->size           = size - track_size;
		alloc_info->physical_start = ptr;
		ptr                        = zpl_cast( void* )( alloc_info + 1 );
		zpl__heap_stats_info.used_memory += alloc_info->size;
		zpl__heap_stats_info.alloc_count++;
	}
#endif

	return ptr;
}

//
// Arena Allocator
//

ZPL_ALLOCATOR_PROC( arena_allocator_proc )
{
	zpl_arena* arena = zpl_cast( zpl_arena* ) allocator_data;
	void*      ptr   = NULL;

	unused( old_size );

	switch ( type )
	{
		case ZPL_ALLOCATION_ALLOC :
			{
				void* end        = pointer_add( arena->physical_start, arena->total_allocated );
				sw    total_size = align_forward_i64( size, alignment );

				// NOTE: Out of memory
				if ( arena->total_allocated + total_size > zpl_cast( sw ) arena->total_size )
				{
					// zpl__printf_err("%s", "Arena out of memory\n");
					return NULL;
				}

				ptr = align_forward( end, alignment );
				arena->total_allocated += total_size;
				if ( flags & ZPL_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
					zero_size( ptr, size );
			}
			break;

		case ZPL_ALLOCATION_FREE :
			// NOTE: Free all at once
			// Use Temp_Arena_Memory if you want to free a block
			break;

		case ZPL_ALLOCATION_FREE_ALL :
			arena->total_allocated = 0;
			break;

		case ZPL_ALLOCATION_RESIZE :
			{
				// TODO: Check if ptr is on top of stack and just extend
				zpl_allocator a = arena_allocator( arena );
				ptr             = default_resize_align( a, old_memory, old_size, size, alignment );
			}
			break;
	}
	return ptr;
}

//
// Pool Allocator
//

void pool_init_align( zpl_pool* pool, zpl_allocator backing, sw num_blocks, sw block_size, sw block_align )
{
	sw    actual_block_size, pool_size, block_index;
	void *data, *curr;
	uptr* end;

	zero_item( pool );

	pool->backing     = backing;
	pool->block_size  = block_size;
	pool->block_align = block_align;
	pool->num_blocks  = num_blocks;

	actual_block_size = block_size + block_align;
	pool_size         = num_blocks * actual_block_size;

	data = alloc_align( backing, pool_size, block_align );

	// NOTE: Init intrusive freelist
	curr = data;
	for ( block_index = 0; block_index < num_blocks - 1; block_index++ )
	{
		uptr* next = zpl_cast( uptr* ) curr;
		*next      = zpl_cast( uptr ) curr + actual_block_size;
		curr       = pointer_add( curr, actual_block_size );
	}

	end  = zpl_cast( uptr* ) curr;
	*end = zpl_cast( uptr ) NULL;

	pool->physical_start = data;
	pool->free_list      = data;
}

ZPL_ALLOCATOR_PROC( pool_allocator_proc )
{
	zpl_pool* pool = zpl_cast( zpl_pool* ) allocator_data;
	void*     ptr  = NULL;

	unused( old_size );

	switch ( type )
	{
		case ZPL_ALLOCATION_ALLOC :
			{
				uptr next_free;
				ZPL_ASSERT( size == pool->block_size );
				ZPL_ASSERT( alignment == pool->block_align );
				ZPL_ASSERT( pool->free_list != NULL );

				next_free       = *zpl_cast( uptr* ) pool->free_list;
				ptr             = pool->free_list;
				pool->free_list = zpl_cast( void* ) next_free;
				pool->total_size += pool->block_size;
				if ( flags & ZPL_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
					zero_size( ptr, size );
			}
			break;

		case ZPL_ALLOCATION_FREE :
			{
				uptr* next;
				if ( old_memory == NULL )
					return NULL;

				next            = zpl_cast( uptr* ) old_memory;
				*next           = zpl_cast( uptr ) pool->free_list;
				pool->free_list = old_memory;
				pool->total_size -= pool->block_size;
			}
			break;

		case ZPL_ALLOCATION_FREE_ALL :
			{
				sw    actual_block_size, block_index;
				void* curr;
				uptr* end;

				actual_block_size = pool->block_size + pool->block_align;
				pool->total_size  = 0;

				// NOTE: Init intrusive freelist
				curr = pool->physical_start;
				for ( block_index = 0; block_index < pool->num_blocks - 1; block_index++ )
				{
					uptr* next = zpl_cast( uptr* ) curr;
					*next      = zpl_cast( uptr ) curr + actual_block_size;
					curr       = pointer_add( curr, actual_block_size );
				}

				end             = zpl_cast( uptr* ) curr;
				*end            = zpl_cast( uptr ) NULL;
				pool->free_list = pool->physical_start;
			}
			break;

		case ZPL_ALLOCATION_RESIZE :
			// NOTE: Cannot resize
			ZPL_PANIC( "You cannot resize something allocated by with a pool." );
			break;
	}

	return ptr;
}

//
// Scratch Memory Allocator
//

void scratch_memory_init( scratch_memory* s, void* start, sw size )
{
	s->physical_start = start;
	s->total_size     = size;
	s->alloc_point    = start;
	s->free_point     = start;
}

b32 scratch_memory_is_in_use( scratch_memory* s, void* ptr )
{
	if ( s->free_point == s->alloc_point )
		return false;
	if ( s->alloc_point > s->free_point )
		return ptr >= s->free_point && ptr < s->alloc_point;
	return ptr >= s->free_point || ptr < s->alloc_point;
}

zpl_allocator scratch_allocator( scratch_memory* s )
{
	zpl_allocator a;
	a.proc = scratch_allocator_proc;
	a.data = s;
	return a;
}

ZPL_ALLOCATOR_PROC( scratch_allocator_proc )
{
	scratch_memory* s   = zpl_cast( scratch_memory* ) allocator_data;
	void*           ptr = NULL;
	ZPL_ASSERT_NOT_NULL( s );

	switch ( type )
	{
		case ZPL_ALLOCATION_ALLOC :
			{
				void*                 pt     = s->alloc_point;
				allocation_header_ev* header = zpl_cast( allocation_header_ev* ) pt;
				void*                 data   = align_forward( header + 1, alignment );
				void*                 end    = pointer_add( s->physical_start, s->total_size );

				ZPL_ASSERT( alignment % 4 == 0 );
				size = ( ( size + 3 ) / 4 ) * 4;
				pt   = pointer_add( pt, size );

				// NOTE: Wrap around
				if ( pt > end )
				{
					header->size = pointer_diff( header, end ) | ZPL_ISIZE_HIGH_BIT;
					pt           = s->physical_start;
					header       = zpl_cast( allocation_header_ev* ) pt;
					data         = align_forward( header + 1, alignment );
					pt           = pointer_add( pt, size );
				}

				if ( ! scratch_memory_is_in_use( s, pt ) )
				{
					allocation_header_fill( header, pt, pointer_diff( header, pt ) );
					s->alloc_point = zpl_cast( u8* ) pt;
					ptr            = data;
				}

				if ( flags & ZPL_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
					zero_size( ptr, size );
			}
			break;

		case ZPL_ALLOCATION_FREE :
			{
				if ( old_memory )
				{
					void* end = pointer_add( s->physical_start, s->total_size );
					if ( old_memory < s->physical_start || old_memory >= end )
					{
						ZPL_ASSERT( false );
					}
					else
					{
						// NOTE: Mark as free
						allocation_header_ev* h = allocation_header( old_memory );
						ZPL_ASSERT( ( h->size & ZPL_ISIZE_HIGH_BIT ) == 0 );
						h->size = h->size | ZPL_ISIZE_HIGH_BIT;

						while ( s->free_point != s->alloc_point )
						{
							allocation_header_ev* header = zpl_cast( allocation_header_ev* ) s->free_point;
							if ( ( header->size & ZPL_ISIZE_HIGH_BIT ) == 0 )
								break;

							s->free_point = pointer_add( s->free_point, h->size & ( ~ZPL_ISIZE_HIGH_BIT ) );
							if ( s->free_point == end )
								s->free_point = s->physical_start;
						}
					}
				}
			}
			break;

		case ZPL_ALLOCATION_FREE_ALL :
			s->alloc_point = s->physical_start;
			s->free_point  = s->physical_start;
			break;

		case ZPL_ALLOCATION_RESIZE :
			ptr = default_resize_align( scratch_allocator( s ), old_memory, old_size, size, alignment );
			break;
	}

	return ptr;
}

//
// Stack Memory Allocator
//
ZPL_ALLOCATOR_PROC( stack_allocator_proc )
{
	stack_memory* s   = zpl_cast( stack_memory* ) allocator_data;
	void*         ptr = NULL;
	ZPL_ASSERT_NOT_NULL( s );
	unused( old_size );
	unused( flags );

	switch ( type )
	{
		case ZPL_ALLOCATION_ALLOC :
			{
				size += ZPL_STACK_ALLOC_OFFSET;
				u64 alloc_offset = s->allocated;

				void* curr = zpl_cast( u64* ) align_forward( zpl_cast( u64* ) pointer_add( s->physical_start, s->allocated ), alignment );

				if ( zpl_cast( u64* ) pointer_add( curr, size ) > zpl_cast( u64* ) pointer_add( s->physical_start, s->total_size ) )
				{
					if ( s->backing.proc )
					{
						void* old_start   = s->physical_start;
						s->physical_start = resize_align( s->backing, s->physical_start, s->total_size, s->total_size + size, alignment );
						curr              = zpl_cast( u64* ) align_forward( zpl_cast( u64* ) pointer_add( s->physical_start, s->allocated ), alignment );
						s->total_size     = pointer_diff( old_start, s->physical_start );
					}
					else
					{
						ZPL_PANIC( "Can not resize stack's memory! Allocator not defined!" );
					}
				}

				s->allocated = pointer_diff( s->physical_start, curr ) + size;

				*(u64*)curr = alloc_offset;
				curr        = pointer_add( curr, ZPL_STACK_ALLOC_OFFSET );

				ptr = curr;
			}
			break;

		case ZPL_ALLOCATION_FREE :
			{
				if ( old_memory )
				{
					void* curr = old_memory;
					curr       = pointer_sub( curr, ZPL_STACK_ALLOC_OFFSET );

					u64 alloc_offset = *(u64*)curr;
					s->allocated     = (uw)alloc_offset;
				}
			}
			break;

		case ZPL_ALLOCATION_FREE_ALL :
			{
				s->allocated = 0;
			}
			break;

		case ZPL_ALLOCATION_RESIZE :
			{
				ZPL_PANIC( "You cannot resize something allocated by a stack." );
			}
			break;
	}
	return ptr;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
