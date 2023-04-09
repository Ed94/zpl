// file: header/essentials/types.h



/* Basic types */

#if defined( ZPL_COMPILER_MSVC )
ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS
#	if _MSC_VER < 1300
typedef unsigned char  u8;
typedef signed char    s8;
typedef unsigned short u16;
typedef signed short   s16;
typedef unsigned int   u32;
typedef signed int     s32;
#    else
typedef unsigned __int8  u8;
typedef signed __int8    s8;
typedef unsigned __int16 u16;
typedef signed __int16   s16;
typedef unsigned __int32 u32;
typedef signed __int32   s32;
#    endif
typedef unsigned __int64 u64;
typedef signed __int64   s64;
ZPL_END_C_DECLS
ZPL_END_NAMESPACE
#else
#	include <stdint.h>

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS
typedef uint8_t  u8;
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;
ZPL_END_C_DECLS
ZPL_END_NAMESPACE
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS
ZPL_STATIC_ASSERT( sizeof( u8 ) == sizeof( s8 ), "sizeof(u8) != sizeof(s8)" );
ZPL_STATIC_ASSERT( sizeof( u16 ) == sizeof( s16 ), "sizeof(u16) != sizeof(s16)" );
ZPL_STATIC_ASSERT( sizeof( u32 ) == sizeof( s32 ), "sizeof(u32) != sizeof(s32)" );
ZPL_STATIC_ASSERT( sizeof( u64 ) == sizeof( s64 ), "sizeof(u64) != sizeof(s64)" );

ZPL_STATIC_ASSERT( sizeof( u8 ) == 1, "sizeof(u8) != 1" );
ZPL_STATIC_ASSERT( sizeof( u16 ) == 2, "sizeof(u16) != 2" );
ZPL_STATIC_ASSERT( sizeof( u32 ) == 4, "sizeof(u32) != 4" );
ZPL_STATIC_ASSERT( sizeof( u64 ) == 8, "sizeof(u64) != 8" );

typedef size_t    uw;
typedef ptrdiff_t sw;

ZPL_STATIC_ASSERT( sizeof( uw ) == sizeof( sw ), "sizeof(uw) != sizeof(sw)" );

// NOTE: (u)zpl_intptr is only here for semantic reasons really as this library will only support 32/64 bit OSes.
#if defined( _WIN64 )
typedef signed __int64   sptr;
typedef unsigned __int64 uptr;
#elif defined( _WIN32 )
// NOTE; To mark types changing their size, e.g. zpl_intptr
#	ifndef _W64
#		if ! defined( __midl ) && ( defined( _X86_ ) || defined( _M_IX86 ) ) && _MSC_VER >= 1300
#			define _W64 __w64
#		else
#			define _W64
#		endif
#	endif
typedef _W64 signed int   sptr;
typedef _W64 unsigned int uptr;
#else
typedef uintptr_t uptr;
typedef intptr_t  sptr;
#endif

ZPL_STATIC_ASSERT( sizeof( uptr ) == sizeof( sptr ), "sizeof(uptr) != sizeof(sptr)" );

typedef float  f32;
typedef double f64;

ZPL_STATIC_ASSERT( sizeof( f32 ) == 4, "sizeof(f32) != 4" );
ZPL_STATIC_ASSERT( sizeof( f64 ) == 8, "sizeof(f64) != 8" );

typedef s32 rune;    // NOTE: Unicode codepoint
typedef s32 char32;
#define ZPL_RUNE_INVALID zpl_cast( ZPL_NS( rune ) )( 0xfffd )
#define ZPL_RUNE_MAX     zpl_cast( ZPL_NS( rune ) )( 0x0010ffff )
#define ZPL_RUNE_BOM     zpl_cast( ZPL_NS( rune ) )( 0xfeff )
#define ZPL_RUNE_EOF     zpl_cast( ZPL_NS( rune ) )( -1 )

typedef s8  b8;
typedef s16 b16;
typedef s32 b32;
ZPL_END_C_DECLS
ZPL_END_NAMESPACE

#if ! defined( __cplusplus )
#	if ( defined( _MSC_VER ) && _MSC_VER < 1800 ) || ( ! defined( _MSC_VER ) && ! defined( __STDC_VERSION__ ) )
#		ifndef true
#			define true( 0 == 0 )
#		endif
#		ifndef false
#			define false( 0 != 0 )
#		endif
ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS
typedef b8 bool;
ZPL_END_C_DECLS
ZPL_END_NAMESPACE
#	else
#		include <stdbool.h>
#	endif
#endif

#ifndef ZPL_U8_MIN
#	define ZPL_U8_MIN 0u
#	define ZPL_U8_MAX 0xffu
#	define ZPL_I8_MIN ( -0x7f - 1 )
#	define ZPL_I8_MAX 0x7f

#	define ZPL_U16_MIN 0u
#	define ZPL_U16_MAX 0xffffu
#	define ZPL_I16_MIN ( -0x7fff - 1 )
#	define ZPL_I16_MAX 0x7fff

#	define ZPL_U32_MIN 0u
#	define ZPL_U32_MAX 0xffffffffu
#	define ZPL_I32_MIN ( -0x7fffffff - 1 )
#	define ZPL_I32_MAX 0x7fffffff

#	define ZPL_U64_MIN 0ull
#	define ZPL_U64_MAX 0xffffffffffffffffull
#	define ZPL_I64_MIN ( -0x7fffffffffffffffll - 1 )
#	define ZPL_I64_MAX 0x7fffffffffffffffll

#	if defined( ZPL_ARCH_32_BIT )
#		define ZPL_USIZE_MIN ZPL_U32_MIN
#		define ZPL_USIZE_MAX ZPL_U32_MAX
#		define ZPL_ISIZE_MIN ZPL_S32_MIN
#		define ZPL_ISIZE_MAX ZPL_S32_MAX
#	elif defined( ZPL_ARCH_64_BIT )
#		define ZPL_USIZE_MIN ZPL_U64_MIN
#		define ZPL_USIZE_MAX ZPL_U64_MAX
#		define ZPL_ISIZE_MIN ZPL_I64_MIN
#		define ZPL_ISIZE_MAX ZPL_I64_MAX
#	else
#		error Unknown architecture size. This library only supports 32 bit and 64 bit architectures.
#	endif

#	define ZPL_F32_MIN 1.17549435e-38f
#	define ZPL_F32_MAX 3.40282347e+38f

#	define ZPL_F64_MIN 2.2250738585072014e-308
#	define ZPL_F64_MAX 1.7976931348623157e+308
#endif

#ifdef ZPL_DEFINE_NULL_MACRO
#	ifndef NULL
#		define NULL ZPL_NULL
#	endif
#endif
