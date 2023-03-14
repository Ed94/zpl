// file: source/dll.c

#if defined( ZPL_SYSTEM_UNIX ) || defined( ZPL_SYSTEM_MACOS )
#include <dlfcn.h>
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

////////////////////////////////////////////////////////////////
//
// DLL Handling
//
//

#if defined( ZPL_SYSTEM_WINDOWS )
dll_handle dll_load( char const * filepath )
{
	return zpl_cast( dll_handle ) LoadLibraryA( filepath );
}

void dll_unload( dll_handle dll )
{
	FreeLibrary( zpl_cast( HMODULE ) dll );
}

dll_proc dll_proc_address( dll_handle dll, char const * proc_name )
{
	return zpl_cast( dll_proc ) GetProcAddress( zpl_cast( HMODULE ) dll, proc_name );
}

#else // POSIX

dll_handle dll_load( char const * filepath )
{
	return zpl_cast( dll_handle ) dlopen( filepath, RTLD_LAZY | RTLD_GLOBAL );
}

void dll_unload( dll_handle dll )
{
	dlclose( dll );
}

dll_proc dll_proc_address( dll_handle dll, char const * proc_name )
{
	return zpl_cast( dll_proc ) dlsym( dll, proc_name );
}

#endif

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
