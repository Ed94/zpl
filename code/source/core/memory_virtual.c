// a_file: source/core/memory_virtual.c

////////////////////////////////////////////////////////////////
//
// Virtual Memory
//
//

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

virtual_memory vm( void* data, sw size )
{
	virtual_memory vm;
	vm.data = data;
	vm.size = size;
	return vm;
}

#if defined( ZPL_SYSTEM_WINDOWS )
virtual_memory vm_alloc( void* addr, sw size )
{
	virtual_memory vm;
	ZPL_ASSERT( size > 0 );
	vm.data = VirtualAlloc( addr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
	vm.size = size;
	return vm;
}

b32 vm_free( virtual_memory vm )
{
	MEMORY_BASIC_INFORMATION info;
	while ( vm.size > 0 )
	{
		if ( VirtualQuery( vm.data, &info, size_of( info ) ) == 0 )
			return false;
		if ( info.BaseAddress != vm.data || info.AllocationBase != vm.data || info.State != MEM_COMMIT || info.RegionSize > zpl_cast( uw ) vm.size )
		{
			return false;
		}
		if ( VirtualFree( vm.data, 0, MEM_RELEASE ) == 0 )
			return false;
		vm.data = pointer_add( vm.data, info.RegionSize );
		vm.size -= info.RegionSize;
	}
	return true;
}

virtual_memory vm_trim( virtual_memory vm, sw lead_size, sw size )
{
	virtual_memory new_vm = { 0 };
	void*          ptr;
	ZPL_ASSERT( vm.size >= lead_size + size );

	ptr = pointer_add( vm.data, lead_size );

	vm_free( vm );
	new_vm = vm_alloc( ptr, size );
	if ( new_vm.data == ptr )
		return new_vm;
	if ( new_vm.data )
		vm_free( new_vm );
	return new_vm;
}

b32 vm_purge( virtual_memory vm )
{
	VirtualAlloc( vm.data, vm.size, MEM_RESET, PAGE_READWRITE );
	// NOTE: Can this really fail?
	return true;
}

sw virtual_memory_page_size( sw* alignment_out )
{
	SYSTEM_INFO info;
	GetSystemInfo( &info );
	if ( alignment_out )
		*alignment_out = info.dwAllocationGranularity;
	return info.dwPageSize;
}

#else
#	include <sys/mman.h>

#	ifndef MAP_ANONYMOUS
#		define MAP_ANONYMOUS MAP_ANON
#	endif

virtual_memory vm_alloc( void* addr, sw size )
{
	virtual_memory vm;
	ZPL_ASSERT( size > 0 );
	vm.data = mmap( addr, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0 );
	vm.size = size;
	return vm;
}

b32 vm_free( virtual_memory vm )
{
	munmap( vm.data, vm.size );
	return true;
}

virtual_memory vm_trim( virtual_memory vm, sw lead_size, sw size )
{
	void* ptr;
	sw    trail_size;
	ZPL_ASSERT( vm.size >= lead_size + size );

	ptr        = pointer_add( vm.data, lead_size );
	trail_size = vm.size - lead_size - size;

	if ( lead_size != 0 )
		vm_free( vm( vm.data, lead_size ) );
	if ( trail_size != 0 )
		vm_free( vm( ptr, trail_size ) );
	return vm( ptr, size );
}

b32 vm_purge( virtual_memory vm )
{
	int err = madvise( vm.data, vm.size, MADV_DONTNEED );
	return err != 0;
}

sw virtual_memory_page_size( sw* alignment_out )
{
	// TODO: Is this always true?
	sw result = zpl_cast( sw ) sysconf( _SC_PAGE_SIZE );
	if ( alignment_out )
		*alignment_out = result;
	return result;
}

#endif

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
