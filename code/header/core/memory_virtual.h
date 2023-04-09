// file: header/core/memory_virtual.h


////////////////////////////////////////////////////////////////
//
// Virtual Memory
//
//

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct virtual_memory
{
	void* data;
	sw    size;
} virtual_memory;

//! Initialize virtual memory from existing data.
ZPL_DEF virtual_memory vm( void* data, sw size );

//! Allocate virtual memory at address with size.

//! @param addr The starting address of the region to reserve. If NULL, it lets operating system to decide where to allocate it.
//! @param size The size to serve.
ZPL_DEF virtual_memory vm_alloc( void* addr, sw size );

//! Release the virtual memory.
ZPL_DEF b32 vm_free( virtual_memory vm );

//! Trim virtual memory.
ZPL_DEF virtual_memory vm_trim( virtual_memory vm, sw lead_size, sw size );

//! Purge virtual memory.
ZPL_DEF b32 vm_purge( virtual_memory vm );

//! Retrieve VM's page size and alignment.
ZPL_DEF sw virtual_memory_page_size( sw* alignment_out );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
