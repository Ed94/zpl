// a_file: header/essentials/debug.h

/* Debugging stuff */

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#ifndef ZPL_DEBUG_TRAP
#	if defined( _MSC_VER )
#		if _MSC_VER < 1300
#			define ZPL_DEBUG_TRAP() __asm int 3 /* Trap to debugger! */
#		else
#			define ZPL_DEBUG_TRAP() __debugbreak()
#		endif
#	elif defined( ZPL_COMPILER_TINYC )
#		define ZPL_DEBUG_TRAP() exit( 1 )
#	else
#		define ZPL_DEBUG_TRAP() __builtin_trap()
#	endif
#endif

#ifndef ZPL_ASSERT_MSG
#	define ZPL_ASSERT_MSG( cond, msg, ... )                                                                                                                                       \
		do                                                                                                                                                                         \
		{                                                                                                                                                                          \
			if ( ! ( cond ) )                                                                                                                                                      \
			{                                                                                                                                                                      \
				ZPL_NS assert_handler( #cond, __FILE__, zpl_cast( s64 ) __LINE__, msg, ##__VA_ARGS__ );                                                                            \
				ZPL_DEBUG_TRAP();                                                                                                                                                  \
			}                                                                                                                                                                      \
		} while ( 0 )
#endif

#ifndef ZPL_ASSERT
#	define ZPL_ASSERT( cond ) ZPL_ASSERT_MSG( cond, NULL )
#endif

#ifndef ZPL_ASSERT_NOT_NULL
#	define ZPL_ASSERT_NOT_NULL( ptr ) ZPL_ASSERT_MSG( ( ptr ) != NULL, #ptr " must not be NULL" )
#endif

// NOTE: Things that shouldn't happen with a message!
#ifndef ZPL_PANIC
#	define ZPL_PANIC( msg, ... ) ZPL_ASSERT_MSG( 0, msg, ##__VA_ARGS__ )
#endif

#ifndef ZPL_NOT_IMPLEMENTED
#	define ZPL_NOT_IMPLEMENTED ZPL_PANIC( "not implemented" )
#endif

/* Functions */

ZPL_DEF void assert_handler( char const* condition, char const* a_file, s32 line, char const* msg, ... );
ZPL_DEF s32  assert_crash( char const* condition );
ZPL_DEF void exit( u32 code );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
