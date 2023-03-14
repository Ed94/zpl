// file: header/dll.h

/** @file dll.c
@brief DLL Handling
@defgroup dll DLL handling

@{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef void* dll_handle;
typedef void ( *dll_proc )( void );

ZPL_DEF dll_handle dll_load( char const * filepath );
ZPL_DEF void       dll_unload( dll_handle dll );
ZPL_DEF dll_proc   dll_proc_address( dll_handle dll, char const * proc_name );

//! @}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
