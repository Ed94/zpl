// file: header/essentials/helpers.h

/* Various macro based helpers */

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#ifndef zpl_cast
#define zpl_cast( Type ) ( Type )
#endif

#ifndef size_of
#define size_of( x ) ( sw )( sizeof( x ) )
#endif

#ifndef count_of
#define count_of( x ) ( ( size_of( x ) / size_of( 0 [ x ] ) ) / ( (sw)( ! ( size_of( x ) % size_of( 0 [ x ] ) ) ) ) )
#endif

#ifndef offset_of
#if defined( _MSC_VER ) || defined( ZPL_COMPILER_TINYC )
#define offset_of( Type, element ) ( ( sw ) & ( ( (Type*)0 )->element ) )
#else
#define offset_of( Type, element ) __builtin_offsetof( Type, element )
#endif
#endif

#if defined( __cplusplus )
#ifndef align_of
#if __cplusplus >= 201103L
#define align_of( Type ) (sw)alignof( Type )
#else
extern "C++"
{
	template < typename T >
	struct alignment_trick
	{
		char c;
		T    member;
	};
}
#define align_of( Type ) offset_of( alignment_trick< Type >, member )
#endif
#endif
#else
#ifndef align_of
#define align_of( Type )                                                                                                                                                           \
	offset_of(                                                                                                                                                                     \
	    struct {                                                                                                                                                                   \
		    char c;                                                                                                                                                                \
		    Type member;                                                                                                                                                           \
	    },                                                                                                                                                                         \
	    member )
#endif
#endif

#ifndef swap
#define swap( Type, a, b )                                                                                                                                                         \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		Type tmp = ( a );                                                                                                                                                          \
		( a )    = ( b );                                                                                                                                                          \
		( b )    = tmp;                                                                                                                                                            \
	} while ( 0 )
#endif

#ifndef global
#define global static // Global variables
#endif

#ifndef internal
#define internal static // Internal linkage
#endif

#ifndef local_persist
#define local_persist static // Local Persisting variables
#endif

#ifndef unused
#if defined( _MSC_VER )
#define unused( x ) ( __pragma( warning( suppress : 4100 ) )( x ) )
#elif defined( __GCC__ )
#define unused( x ) __attribute__( ( __unused__ ) ) ( x )
#else
#define unused( x ) ( (void)( size_of( x ) ) )
#endif
#endif

#ifndef ZPL_JOIN_MACROS
#define ZPL_JOIN_MACROS

#define ZPL_JOIN2               ZPL_CONCAT
#define ZPL_JOIN3( a, b, c )    ZPL_JOIN2( ZPL_JOIN2( a, b ), c )
#define ZPL_JOIN4( a, b, c, d ) ZPL_JOIN2( ZPL_JOIN2( ZPL_JOIN2( a, b ), c ), d )
#endif

#ifndef ZPL_BIT
#define ZPL_BIT( x ) ( 1 << ( x ) )
#endif

#ifndef min
#define min( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#endif

#ifndef max
#define max( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#endif

#ifndef min3
#define min3( a, b, c ) min( min( a, b ), c )
#endif

#ifndef max3
#define max3( a, b, c ) max( max( a, b ), c )
#endif

#ifndef clamp
#define clamp( x, lower, upper ) min( max( ( x ), ( lower ) ), ( upper ) )
#endif

#ifndef clamp01
#define clamp01( x ) clamp( ( x ), 0, 1 )
#endif

#ifndef is_between
#define is_between( x, lower, upper ) ( ( ( lower ) <= ( x ) ) && ( ( x ) <= ( upper ) ) )
#endif

#ifndef is_between_limit
#define is_between_limit( x, lower, upper ) ( ( ( lower ) <= ( x ) ) && ( ( x ) < ( upper ) ) )
#endif

#ifndef step
#define step( x, y ) ( ( ( x ) / ( y ) ) * ( y ) )
#endif

#ifndef abs
#define abs( x ) ( ( x ) < 0 ? -( x ) : ( x ) )
#endif

#ifndef ZPL_MASK_SET
#define ZPL_MASK_SET( var, set, mask )                                                                                                                                             \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		if ( set )                                                                                                                                                                 \
			( var ) |= ( mask );                                                                                                                                                   \
		else                                                                                                                                                                       \
			( var ) &= ~( mask );                                                                                                                                                  \
	} while ( 0 )
#endif

// Multiline string literals in C99!
#ifndef ZPL_MULTILINE
#define ZPL_MULTILINE( ... ) #__VA_ARGS__
#endif

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
