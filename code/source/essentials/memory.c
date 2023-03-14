// file: source/essentials/memory.c

#include <string.h>

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

void zpl_memswap( void* i, void* j, sw size )
{
	if ( i == j )
		return;

	if ( size == 4 )
	{
		swap( u32, *zpl_cast( u32* ) i, *zpl_cast( u32* ) j );
	}
	else if ( size == 8 )
	{
		swap( u64, *zpl_cast( u64* ) i, *zpl_cast( u64* ) j );
	}
	else if ( size < 8 )
	{
		u8* a = zpl_cast( u8* ) i;
		u8* b = zpl_cast( u8* ) j;
		if ( a != b )
		{
			while ( size-- )
			{
				swap( u8, *a++, *b++ );
			}
		}
	}
	else
	{
		char buffer[ 256 ];

		while ( size > size_of( buffer ) )
		{
			zpl_memswap( i, j, size_of( buffer ) );
			i = pointer_add( i, size_of( buffer ) );
			j = pointer_add( j, size_of( buffer ) );
			size -= size_of( buffer );
		}

		zpl_memcopy( buffer, i, size );
		zpl_memcopy( i, j, size );
		zpl_memcopy( j, buffer, size );
	}
}

void const * zpl_memchr( void const * data, u8 c, sw n )
{
	u8 const * s = zpl_cast( u8 const * ) data;
	while ( ( zpl_cast( uptr ) s & ( sizeof( uw ) - 1 ) ) && n && *s != c )
	{
		s++;
		n--;
	}
	if ( n && *s != c )
	{
		sw const * w;
		sw         k = ZPL__ONES * c;
		w            = zpl_cast( sw const * ) s;
		while ( n >= size_of( sw ) && ! ZPL__HAS_ZERO( *w ^ k ) )
		{
			w++;
			n -= size_of( sw );
		}
		s = zpl_cast( u8 const * ) w;
		while ( n && *s != c )
		{
			s++;
			n--;
		}
	}

	return n ? zpl_cast( void const * ) s : NULL;
}

void const * memrchr( void const * data, u8 c, sw n )
{
	u8 const * s = zpl_cast( u8 const * ) data;
	while ( n-- )
	{
		if ( s[ n ] == c )
			return zpl_cast( void const * )( s + n );
	}
	return NULL;
}

void* zpl_memcopy( void* dest, void const * source, sw n )
{
	if ( dest == NULL )
	{
		return NULL;
	}

	return memcpy( dest, source, n );

	// TODO: Re-work the whole method
#if 0
#if defined( _MSC_VER )
    __movsb(zpl_cast(u8 *) dest, zpl_cast(u8 *) source, n);
#elif defined( ZPL_CPU_X86 ) && ! defined( ZPL_SYSTEM_EMSCRIPTEN )
    u8 *__dest8 = zpl_cast(u8 *) dest;
    u8 *__source8 = zpl_cast(u8 *) source;
    __asm__ __volatile__("rep movsb" : "+D"(__dest8), "+S"(__source8), "+c"(n) : : "memory");
#elif defined( ZPL_CPU_ARM )
    return memcpy(dest, source, n);
#else
    u8 *d = zpl_cast(u8 *) dest;
    u8 const *s = zpl_cast(u8 const *) source;
    u32 w, x;

    for (; zpl_cast(uptr) s % 4 && n; n--) *d++ = *s++;

    if (zpl_cast(uptr) d % 4 == 0) {
        for (; n >= 16; s += 16, d += 16, n -= 16) {
            *zpl_cast(u32 *)(d + 0) = *zpl_cast(u32 *)(s + 0);
            *zpl_cast(u32 *)(d + 4) = *zpl_cast(u32 *)(s + 4);
            *zpl_cast(u32 *)(d + 8) = *zpl_cast(u32 *)(s + 8);
            *zpl_cast(u32 *)(d + 12) = *zpl_cast(u32 *)(s + 12);
        }
        if (n & 8) {
            *zpl_cast(u32 *)(d + 0) = *zpl_cast(u32 *)(s + 0);
            *zpl_cast(u32 *)(d + 4) = *zpl_cast(u32 *)(s + 4);
            d += 8;
            s += 8;
        }
        if (n & 4) {
            *zpl_cast(u32 *)(d + 0) = *zpl_cast(u32 *)(s + 0);
            d += 4;
            s += 4;
        }
        if (n & 2) {
            *d++ = *s++;
            *d++ = *s++;
        }
        if (n & 1) { *d = *s; }
        return dest;
    }

    if (n >= 32) {
#if __BYTE_ORDER == __BIG_ENDIAN
#define LS <<
#define RS >>
#else
#define LS >>
#define RS <<
#endif
        switch (zpl_cast(uptr) d % 4) {
            case 1: {
                w = *zpl_cast(u32 *) s;
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                n -= 3;
                while (n > 16) {
                    x = *zpl_cast(u32 *)(s + 1);
                    *zpl_cast(u32 *)(d + 0) = (w LS 24) | (x RS 8);
                    w = *zpl_cast(u32 *)(s + 5);
                    *zpl_cast(u32 *)(d + 4) = (x LS 24) | (w RS 8);
                    x = *zpl_cast(u32 *)(s + 9);
                    *zpl_cast(u32 *)(d + 8) = (w LS 24) | (x RS 8);
                    w = *zpl_cast(u32 *)(s + 13);
                    *zpl_cast(u32 *)(d + 12) = (x LS 24) | (w RS 8);

                    s += 16;
                    d += 16;
                    n -= 16;
                }
            } break;
            case 2: {
                w = *zpl_cast(u32 *) s;
                *d++ = *s++;
                *d++ = *s++;
                n -= 2;
                while (n > 17) {
                    x = *zpl_cast(u32 *)(s + 2);
                    *zpl_cast(u32 *)(d + 0) = (w LS 16) | (x RS 16);
                    w = *zpl_cast(u32 *)(s + 6);
                    *zpl_cast(u32 *)(d + 4) = (x LS 16) | (w RS 16);
                    x = *zpl_cast(u32 *)(s + 10);
                    *zpl_cast(u32 *)(d + 8) = (w LS 16) | (x RS 16);
                    w = *zpl_cast(u32 *)(s + 14);
                    *zpl_cast(u32 *)(d + 12) = (x LS 16) | (w RS 16);

                    s += 16;
                    d += 16;
                    n -= 16;
                }
            } break;
            case 3: {
                w = *zpl_cast(u32 *) s;
                *d++ = *s++;
                n -= 1;
                while (n > 18) {
                    x = *zpl_cast(u32 *)(s + 3);
                    *zpl_cast(u32 *)(d + 0) = (w LS 8) | (x RS 24);
                    w = *zpl_cast(u32 *)(s + 7);
                    *zpl_cast(u32 *)(d + 4) = (x LS 8) | (w RS 24);
                    x = *zpl_cast(u32 *)(s + 11);
                    *zpl_cast(u32 *)(d + 8) = (w LS 8) | (x RS 24);
                    w = *zpl_cast(u32 *)(s + 15);
                    *zpl_cast(u32 *)(d + 12) = (x LS 8) | (w RS 24);

                    s += 16;
                    d += 16;
                    n -= 16;
                }
            } break;
            default: break; // NOTE: Do nowt!
        }
#undef LS
#undef RS
        if (n & 16) {
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
        }
        if (n & 8) {
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
        }
        if (n & 4) {
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
        }
        if (n & 2) {
            *d++ = *s++;
            *d++ = *s++;
        }
        if (n & 1) { *d = *s; }
    }

#endif
#endif

	return dest;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
