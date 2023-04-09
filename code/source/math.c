// file: source/math.c


#if defined( ZPL_COMPILER_TINYC ) && defined( ZPL_NO_MATH_H )
#	undef ZPL_NO_MATH_H
#endif

#if ! defined( ZPL_NO_MATH_H )
#	include <math.h>
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

////////////////////////////////////////////////////////////////
//
// Math
//

f32 to_radians( f32 degrees )
{
	return degrees * ZPL_TAU / 360.0f;
}

f32 to_degrees( f32 radians )
{
	return radians * 360.0f / ZPL_TAU;
}

f32 angle_diff( f32 radians_a, f32 radians_b )
{
	f32 delta  = mod( radians_b - radians_a, ZPL_TAU );
	delta      = mod( delta + 1.5f * ZPL_TAU, ZPL_TAU );
	delta     -= 0.5f * ZPL_TAU;
	return delta;
}

f32 copy_sign( f32 x, f32 y )
{
	s32 ix, iy;
	f32 r;
	memcopy( &ix, &x, size_of( x ) );
	memcopy( &iy, &y, size_of( y ) );

	ix &= 0x7fffffff;
	ix |= iy & 0x80000000;
	memcopy( &r, &ix, size_of( ix ) );
	return r;
}

f32 remainder( f32 x, f32 y )
{
	return x - ( round( x / y ) * y );
}

f32 mod( f32 x, f32 y )
{
	f32 result;
	y      = abs( y );
	result = remainder( abs( x ), y );
	if ( sign( result ) > 0.0f )
		result += y;
	return copy_sign( result, x );
}

f64 copy_sign64( f64 x, f64 y )
{
	s64 ix, iy;
	f64 r;
	memcopy( &ix, &x, size_of( x ) );
	memcopy( &iy, &y, size_of( y ) );

	ix &= 0x7fffffffffffffff;
	ix |= iy & 0x8000000000000000;
	memcopy( &r, &ix, size_of( ix ) );
	return r;
}

f64 floor64( f64 x )
{
	return zpl_cast( f64 )( ( x >= 0.0 ) ? zpl_cast( s64 ) x : zpl_cast( s64 )( x - 0.9999999999999999 ) );
}

f64 ceil64( f64 x )
{
	return zpl_cast( f64 )( ( x < 0 ) ? zpl_cast( s64 ) x : ( zpl_cast( s64 ) x ) + 1 );
}

f64 round64( f64 x )
{
	return zpl_cast( f64 )( ( x >= 0.0 ) ? floor64( x + 0.5 ) : ceil64( x - 0.5 ) );
}

f64 remainder64( f64 x, f64 y )
{
	return x - ( round64( x / y ) * y );
}

f64 abs64( f64 x )
{
	return x < 0 ? -x : x;
}

f64 sign64( f64 x )
{
	return x < 0 ? -1.0 : +1.0;
}

f64 mod64( f64 x, f64 y )
{
	f64 result;
	y      = abs64( y );
	result = remainder64( abs64( x ), y );
	if ( sign64( result ) )
		result += y;
	return copy_sign64( result, x );
}

f32 quake_rsqrt( f32 a )
{
	union
	{
		int i;
		f32 f;
	} t;

	f32       x2;
	f32 const three_halfs = 1.5f;

	x2  = a * 0.5f;
	t.f = a;
	t.i = 0x5f375a86 - ( t.i >> 1 );                  /* What the fuck? */
	t.f = t.f * ( three_halfs - ( x2 * t.f * t.f ) ); /* 1st iteration */
	t.f = t.f * ( three_halfs - ( x2 * t.f * t.f ) ); /* 2nd iteration, this can be removed */

	return t.f;
}

#if defined( ZPL_NO_MATH_H )
#	if defined( _MSC_VER )

f32 rsqrt( f32 a )
{
	return _mm_cvtss_f32( _mm_rsqrt_ss( _mm_set_ss( a ) ) );
}

f32 sqrt( f32 a )
{
	return _mm_cvtss_f32( _mm_sqrt_ss( _mm_set_ss( a ) ) );
};

f32 sin( f32 a )
{
	static f32 const a0 = +1.91059300966915117e-31f;
	static f32 const a1 = +1.00086760103908896f;
	static f32 const a2 = -1.21276126894734565e-2f;
	static f32 const a3 = -1.38078780785773762e-1f;
	static f32 const a4 = -2.67353392911981221e-2f;
	static f32 const a5 = +2.08026600266304389e-2f;
	static f32 const a6 = -3.03996055049204407e-3f;
	static f32 const a7 = +1.38235642404333740e-4f;
	return a0 + a * ( a1 + a * ( a2 + a * ( a3 + a * ( a4 + a * ( a5 + a * ( a6 + a * a7 ) ) ) ) ) );
}

f32 cos( f32 a )
{
	static f32 const a0 = +1.00238601909309722f;
	static f32 const a1 = -3.81919947353040024e-2f;
	static f32 const a2 = -3.94382342128062756e-1f;
	static f32 const a3 = -1.18134036025221444e-1f;
	static f32 const a4 = +1.07123798512170878e-1f;
	static f32 const a5 = -1.86637164165180873e-2f;
	static f32 const a6 = +9.90140908664079833e-4f;
	static f32 const a7 = -5.23022132118824778e-14f;
	return a0 + a * ( a1 + a * ( a2 + a * ( a3 + a * ( a4 + a * ( a5 + a * ( a6 + a * a7 ) ) ) ) ) );
}

f32 tan( f32 radians )
{
	f32 rr  = radians * radians;
	f32 a   = 9.5168091e-03f;
	a      *= rr;
	a      += 2.900525e-03f;
	a      *= rr;
	a      += 2.45650893e-02f;
	a      *= rr;
	a      += 5.33740603e-02f;
	a      *= rr;
	a      += 1.333923995e-01f;
	a      *= rr;
	a      += 3.333314036e-01f;
	a      *= rr;
	a      += 1.0f;
	a      *= radians;
	return a;
}

f32 arcsin( f32 a )
{
	return arctan2( a, sqrt( ( 1.0f + a ) * ( 1.0f - a ) ) );
}

f32 arccos( f32 a )
{
	return arctan2( sqrt( ( 1.0f + a ) * ( 1.0f - a ) ), a );
}

f32 arctan( f32 a )
{
	f32 u  = a * a;
	f32 u2 = u * u;
	f32 u3 = u2 * u;
	f32 u4 = u3 * u;
	f32 f  = 1.0f + 0.33288950512027f * u - 0.08467922817644f * u2 + 0.03252232640125f * u3 - 0.00749305860992f * u4;
	return a / f;
}

f32 arctan2( f32 y, f32 x )
{
	if ( abs( x ) > abs( y ) )
	{
		f32 a = arctan( y / x );
		if ( x > 0.0f )
			return a;
		else
			return y > 0.0f ? a + ZPL_TAU_OVER_2 : a - ZPL_TAU_OVER_2;
	}
	else
	{
		f32 a = arctan( x / y );
		if ( x > 0.0f )
			return y > 0.0f ? ZPL_TAU_OVER_4 - a : -ZPL_TAU_OVER_4 - a;
		else
			return y > 0.0f ? ZPL_TAU_OVER_4 + a : -ZPL_TAU_OVER_4 + a;
	}
}

f32 exp( f32 a )
{
	union
	{
		f32 f;
		int i;
	} u, v;

	u.i = ( int )( 6051102 * a + 1056478197 );
	v.i = ( int )( 1056478197 - 6051102 * a );
	return u.f / v.f;
}

f32 log( f32 a )
{
	union
	{
		f32 f;
		int i;
	} u = { a };

	return ( u.i - 1064866805 ) * 8.262958405176314e-8f; /* 1 / 12102203.0; */
}

f32 pow( f32 a, f32 b )
{
	int flipped = 0, e;
	f32 f, r = 1.0f;
	if ( b < 0 )
	{
		flipped = 1;
		b       = -b;
	}

	e = ( int )b;
	f = exp( b - e );

	while ( e )
	{
		if ( e & 1 )
			r *= a;
		a  *= a;
		e >>= 1;
	}

	r *= f;
	return flipped ? 1.0f / r : r;
}

#	else

f32 rsqrt( f32 a )
{
	return 1.0f / __builtin_sqrt( a );
}

f32 sqrt( f32 a )
{
	return __builtin_sqrt( a );
}

f32 sin( f32 radians )
{
	return __builtin_sinf( radians );
}

f32 cos( f32 radians )
{
	return __builtin_cosf( radians );
}

f32 tan( f32 radians )
{
	return __builtin_tanf( radians );
}

f32 arcsin( f32 a )
{
	return __builtin_asinf( a );
}

f32 arccos( f32 a )
{
	return __builtin_acosf( a );
}

f32 arctan( f32 a )
{
	return __builtin_atanf( a );
}

f32 arctan2( f32 y, f32 x )
{
	return __builtin_atan2f( y, x );
}

f32 exp( f32 x )
{
	return __builtin_expf( x );
}

f32 log( f32 x )
{
	return __builtin_logf( x );
}

// TODO: Should this be zpl_exp(y * zpl_log(x)) ???
f32 pow( f32 x, f32 y )
{
	return __builtin_powf( x, y );
}

#	endif
#else
f32 rsqrt( f32 a )
{
	return 1.0f / sqrtf( a );
}

f32 sqrt( f32 a )
{
	return sqrtf( a );
};

f32 sin( f32 radians )
{
	return sinf( radians );
};

f32 cos( f32 radians )
{
	return cosf( radians );
};

f32 tan( f32 radians )
{
	return tanf( radians );
};

f32 arcsin( f32 a )
{
	return asinf( a );
};

f32 arccos( f32 a )
{
	return acosf( a );
};

f32 arctan( f32 a )
{
	return atanf( a );
};

f32 arctan2( f32 y, f32 x )
{
	return atan2f( y, x );
};

f32 exp( f32 x )
{
	return expf( x );
}

f32 log( f32 x )
{
	return logf( x );
}

f32 pow( f32 x, f32 y )
{
	return powf( x, y );
}
#endif

f32 exp2( f32 x )
{
	return exp( ZPL_LOG_TWO * x );
}

f32 log2( f32 x )
{
	return log( x ) / ZPL_LOG_TWO;
}

f32 fast_exp( f32 x )
{
	/* NOTE: Only works in the range -1 <= x <= +1 */
	f32 e = 1.0f + x * ( 1.0f + x * 0.5f * ( 1.0f + x * 0.3333333333f * ( 1.0f + x * 0.25f * ( 1.0f + x * 0.2f ) ) ) );
	return e;
}

f32 fast_exp2( f32 x )
{
	return fast_exp( ZPL_LOG_TWO * x );
}

f32 round( f32 x )
{
	return ( float )( ( x >= 0.0f ) ? floor( x + 0.5f ) : ceil( x - 0.5f ) );
}

f32 floor( f32 x )
{
	return ( float )( ( x >= 0.0f ) ? ( int )x : ( int )( x - 0.9999999999999999f ) );
}

f32 ceil( f32 x )
{
	return ( float )( ( x < 0.0f ) ? ( int )x : ( ( int )x ) + 1 );
}

f32 half_to_float( half value )
{
	union
	{
		unsigned int i;
		f32          f;
	} result;

	int s = ( value >> 15 ) & 0x001;
	int e = ( value >> 10 ) & 0x01f;
	int m = value & 0x3ff;

	if ( e == 0 )
	{
		if ( m == 0 )
		{
			/* Plus or minus zero */
			result.i = ( unsigned int )( s << 31 );
			return result.f;
		}
		else
		{
			/* Denormalized number */
			while ( ! ( m & 0x00000400 ) )
			{
				m <<= 1;
				e  -= 1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if ( e == 31 )
	{
		if ( m == 0 )
		{
			/* Positive or negative infinity */
			result.i = ( unsigned int )( ( s << 31 ) | 0x7f800000 );
			return result.f;
		}
		else
		{
			/* Nan */
			result.i = ( unsigned int )( ( s << 31 ) | 0x7f800000 | ( m << 13 ) );
			return result.f;
		}
	}

	e = e + ( 127 - 15 );
	m = m << 13;

	result.i = ( unsigned int )( ( s << 31 ) | ( e << 23 ) | m );
	return result.f;
}

half float_to_half( f32 value )
{
	union
	{
		unsigned int i;
		f32          f;
	} v;

	int i, s, e, m;

	v.f = value;
	i   = ( int )v.i;

	s = ( i >> 16 ) & 0x00008000;
	e = ( ( i >> 23 ) & 0x000000ff ) - ( 127 - 15 );
	m = i & 0x007fffff;

	if ( e <= 0 )
	{
		if ( e < -10 )
			return ( half )s;
		m = ( m | 0x00800000 ) >> ( 1 - e );

		if ( m & 0x00001000 )
			m += 0x00002000;

		return ( half )( s | ( m >> 13 ) );
	}
	else if ( e == 0xff - ( 127 - 15 ) )
	{
		if ( m == 0 )
		{
			return ( half )( s | 0x7c00 ); /* NOTE: infinity */
		}
		else
		{
			/* NOTE: NAN */
			m >>= 13;
			return ( half )( s | 0x7c00 | m | ( m == 0 ) );
		}
	}
	else
	{
		if ( m & 0x00001000 )
		{
			m += 0x00002000;
			if ( m & 0x00800000 )
			{
				m  = 0;
				e += 1;
			}
		}

		if ( e > 30 )
		{
			f32 volatile f = 1e12f;
			int j;
			for ( j = 0; j < 10; j++ )
				f *= f; /* NOTE: Cause overflow */

			return ( half )( s | 0x7c00 );
		}

		return ( half )( s | ( e << 10 ) | ( m >> 13 ) );
	}
}

#define ZPL_VEC2_2OP( a, c, post ) \
	a->x = c.x post;               \
	a->y = c.y post;

#define ZPL_VEC2_3OP( a, b, op, c, post ) \
	a->x = b.x op c.x post;               \
	a->y = b.y op c.y post;

#define ZPL_VEC3_2OP( a, c, post ) \
	a->x = c.x post;               \
	a->y = c.y post;               \
	a->z = c.z post;

#define ZPL_VEC3_3OP( a, b, op, c, post ) \
	a->x = b.x op c.x post;               \
	a->y = b.y op c.y post;               \
	a->z = b.z op c.z post;

#define ZPL_VEC4_2OP( a, c, post ) \
	a->x = c.x post;               \
	a->y = c.y post;               \
	a->z = c.z post;               \
	a->w = c.w post;

#define ZPL_VEC4_3OP( a, b, op, c, post ) \
	a->x = b.x op c.x post;               \
	a->y = b.y op c.y post;               \
	a->z = b.z op c.z post;               \
	a->w = b.w op c.w post;

vec2 vec2f_zero( void )
{
	vec2 v = { 0, 0 };
	return v;
}

vec2 vec2f( f32 x, f32 y )
{
	vec2 v;
	v.x = x;
	v.y = y;
	return v;
}

vec2 vec2fv( f32 x[ 2 ] )
{
	vec2 v;
	v.x = x[ 0 ];
	v.y = x[ 1 ];
	return v;
}

vec3 vec3f_zero( void )
{
	vec3 v = { 0, 0, 0 };
	return v;
}

vec3 vec3f( f32 x, f32 y, f32 z )
{
	vec3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

vec3 vec3fv( f32 x[ 3 ] )
{
	vec3 v;
	v.x = x[ 0 ];
	v.y = x[ 1 ];
	v.z = x[ 2 ];
	return v;
}

vec4 vec4f_zero( void )
{
	vec4 v = { 0, 0, 0, 0 };
	return v;
}

vec4 vec4f( f32 x, f32 y, f32 z, f32 w )
{
	vec4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

vec4 vec4fv( f32 x[ 4 ] )
{
	vec4 v;
	v.x = x[ 0 ];
	v.y = x[ 1 ];
	v.z = x[ 2 ];
	v.w = x[ 3 ];
	return v;
}

f32 vec2_max( vec2 v )
{
	return max( v.x, v.y );
}

f32 vec2_side( vec2 p, vec2 q, vec2 r )
{
	return ( ( q.x - p.x ) * ( r.y - p.y ) - ( r.x - p.x ) * ( q.y - p.y ) );
}

void vec2_add( vec2* d, vec2 v0, vec2 v1 )
{
	ZPL_VEC2_3OP( d, v0, +, v1, +0 );
}

void vec2_sub( vec2* d, vec2 v0, vec2 v1 )
{
	ZPL_VEC2_3OP( d, v0, -, v1, +0 );
}

void vec2_mul( vec2* d, vec2 v, f32 s )
{
	ZPL_VEC2_2OP( d, v, *s );
}

void vec2_div( vec2* d, vec2 v, f32 s )
{
	ZPL_VEC2_2OP( d, v, / s );
}

f32 vec3_max( vec3 v )
{
	return max3( v.x, v.y, v.z );
}

void vec3_add( vec3* d, vec3 v0, vec3 v1 )
{
	ZPL_VEC3_3OP( d, v0, +, v1, +0 );
}

void vec3_sub( vec3* d, vec3 v0, vec3 v1 )
{
	ZPL_VEC3_3OP( d, v0, -, v1, +0 );
}

void vec3_mul( vec3* d, vec3 v, f32 s )
{
	ZPL_VEC3_2OP( d, v, *s );
}

void vec3_div( vec3* d, vec3 v, f32 s )
{
	ZPL_VEC3_2OP( d, v, / s );
}

void vec4_add( vec4* d, vec4 v0, vec4 v1 )
{
	ZPL_VEC4_3OP( d, v0, +, v1, +0 );
}

void vec4_sub( vec4* d, vec4 v0, vec4 v1 )
{
	ZPL_VEC4_3OP( d, v0, -, v1, +0 );
}

void vec4_mul( vec4* d, vec4 v, f32 s )
{
	ZPL_VEC4_2OP( d, v, *s );
}

void vec4_div( vec4* d, vec4 v, f32 s )
{
	ZPL_VEC4_2OP( d, v, / s );
}

void vec2_addeq( vec2* d, vec2 v )
{
	ZPL_VEC2_3OP( d, ( *d ), +, v, +0 );
}

void vec2_subeq( vec2* d, vec2 v )
{
	ZPL_VEC2_3OP( d, ( *d ), -, v, +0 );
}

void vec2_muleq( vec2* d, f32 s )
{
	ZPL_VEC2_2OP( d, ( *d ), *s );
}

void vec2_diveq( vec2* d, f32 s )
{
	ZPL_VEC2_2OP( d, ( *d ), / s );
}

void vec3_addeq( vec3* d, vec3 v )
{
	ZPL_VEC3_3OP( d, ( *d ), +, v, +0 );
}

void vec3_subeq( vec3* d, vec3 v )
{
	ZPL_VEC3_3OP( d, ( *d ), -, v, +0 );
}

void vec3_muleq( vec3* d, f32 s )
{
	ZPL_VEC3_2OP( d, ( *d ), *s );
}

void vec3_diveq( vec3* d, f32 s )
{
	ZPL_VEC3_2OP( d, ( *d ), / s );
}

void vec4_addeq( vec4* d, vec4 v )
{
	ZPL_VEC4_3OP( d, ( *d ), +, v, +0 );
}

void vec4_subeq( vec4* d, vec4 v )
{
	ZPL_VEC4_3OP( d, ( *d ), -, v, +0 );
}

void vec4_muleq( vec4* d, f32 s )
{
	ZPL_VEC4_2OP( d, ( *d ), *s );
}

void vec4_diveq( vec4* d, f32 s )
{
	ZPL_VEC4_2OP( d, ( *d ), / s );
}

#undef ZPL_VEC2_2OP
#undef ZPL_VEC2_3OP
#undef ZPL_VEC3_3OP
#undef ZPL_VEC3_2OP
#undef ZPL_VEC4_2OP
#undef ZPL_VEC4_3OP

f32 vec2_dot( vec2 v0, vec2 v1 )
{
	return v0.x * v1.x + v0.y * v1.y;
}

f32 vec3_dot( vec3 v0, vec3 v1 )
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

f32 vec4_dot( vec4 v0, vec4 v1 )
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
}

void vec2_cross( f32* d, vec2 v0, vec2 v1 )
{
	*d = v0.x * v1.y - v1.x * v0.y;
}

void vec3_cross( vec3* d, vec3 v0, vec3 v1 )
{
	d->x = v0.y * v1.z - v0.z * v1.y;
	d->y = v0.z * v1.x - v0.x * v1.z;
	d->z = v0.x * v1.y - v0.y * v1.x;
}

f32 vec2_mag2( vec2 v )
{
	return vec2_dot( v, v );
}

f32 vec3_mag2( vec3 v )
{
	return vec3_dot( v, v );
}

f32 vec4_mag2( vec4 v )
{
	return vec4_dot( v, v );
}

/* TODO: Create custom sqrt function */
f32 vec2_mag( vec2 v )
{
	return sqrt( vec2_dot( v, v ) );
}

f32 vec3_mag( vec3 v )
{
	return sqrt( vec3_dot( v, v ) );
}

f32 vec4_mag( vec4 v )
{
	return sqrt( vec4_dot( v, v ) );
}

void vec2_norm( vec2* d, vec2 v )
{
	f32 inv_mag = rsqrt( vec2_dot( v, v ) );
	vec2_mul( d, v, inv_mag );
}

void vec3_norm( vec3* d, vec3 v )
{
	f32 inv_mag = rsqrt( vec3_dot( v, v ) );
	vec3_mul( d, v, inv_mag );
}

void vec4_norm( vec4* d, vec4 v )
{
	f32 inv_mag = rsqrt( vec4_dot( v, v ) );
	vec4_mul( d, v, inv_mag );
}

void vec2_norm0( vec2* d, vec2 v )
{
	f32 mag = vec2_mag( v );
	if ( mag > 0 )
		vec2_div( d, v, mag );
	else
		*d = vec2f_zero();
}

void vec3_norm0( vec3* d, vec3 v )
{
	f32 mag = vec3_mag( v );
	if ( mag > 0 )
		vec3_div( d, v, mag );
	else
		*d = vec3f_zero();
}

void vec4_norm0( vec4* d, vec4 v )
{
	f32 mag = vec4_mag( v );
	if ( mag > 0 )
		vec4_div( d, v, mag );
	else
		*d = vec4f_zero();
}

void vec2_reflect( vec2* d, vec2 i, vec2 n )
{
	vec2 b = n;
	vec2_muleq( &b, 2.0f * vec2_dot( n, i ) );
	vec2_sub( d, i, b );
}

void vec3_reflect( vec3* d, vec3 i, vec3 n )
{
	vec3 b = n;
	vec3_muleq( &b, 2.0f * vec3_dot( n, i ) );
	vec3_sub( d, i, b );
}

void vec2_refract( vec2* d, vec2 i, vec2 n, f32 eta )
{
	vec2 a, b;
	f32  dv, k;

	dv = vec2_dot( n, i );
	k  = 1.0f - eta * eta * ( 1.0f - dv * dv );
	vec2_mul( &a, i, eta );
	vec2_mul( &b, n, eta * dv * sqrt( k ) );
	vec2_sub( d, a, b );
	vec2_muleq( d, ( float )( k >= 0.0f ) );
}

void vec3_refract( vec3* d, vec3 i, vec3 n, f32 eta )
{
	vec3 a, b;
	f32  dv, k;

	dv = vec3_dot( n, i );
	k  = 1.0f - eta * eta * ( 1.0f - dv * dv );
	vec3_mul( &a, i, eta );
	vec3_mul( &b, n, eta * dv * sqrt( k ) );
	vec3_sub( d, a, b );
	vec3_muleq( d, ( float )( k >= 0.0f ) );
}

f32 vec2_aspect_ratio( vec2 v )
{
	return ( v.y < 0.0001f ) ? 0.0f : v.x / v.y;
}

void mat2_transpose( mat2* m )
{
	float22_transpose( float22_m( m ) );
}

void mat2_identity( mat2* m )
{
	float22_identity( float22_m( m ) );
}

void mat2_mul( mat2* out, mat2* m1, mat2* m2 )
{
	float22_mul( float22_m( out ), float22_m( m1 ), float22_m( m2 ) );
}

void float22_identity( f32 m[ 2 ][ 2 ] )
{
	m[ 0 ][ 0 ] = 1;
	m[ 0 ][ 1 ] = 0;
	m[ 1 ][ 0 ] = 0;
	m[ 1 ][ 1 ] = 1;
}

void mat2_copy( mat2* out, mat2* m )
{
	memcopy( out, m, sizeof( mat3 ) );
}

void mat2_mul_vec2( vec2* out, mat2* m, vec2 in )
{
	float22_mul_vec2( out, float22_m( m ), in );
}

mat2* mat2_v( vec2 m[ 2 ] )
{
	return ( mat2* )m;
}

mat2* mat2_f( f32 m[ 2 ][ 2 ] )
{
	return ( mat2* )m;
}

float2* float22_m( mat2* m )
{
	return ( float2* )m;
}

float2* float22_v( vec2 m[ 2 ] )
{
	return ( float2* )m;
}

float2* float22_4( f32 m[ 4 ] )
{
	return ( float2* )m;
}

void float22_transpose( f32 ( *vec )[ 2 ] )
{
	int i, j;
	for ( j = 0; j < 2; j++ )
	{
		for ( i = j + 1; i < 2; i++ )
		{
			f32 t         = vec[ i ][ j ];
			vec[ i ][ j ] = vec[ j ][ i ];
			vec[ j ][ i ] = t;
		}
	}
}

void float22_mul( f32 ( *out )[ 2 ], f32 ( *mat1 )[ 2 ], f32 ( *mat2 )[ 2 ] )
{
	int i, j;
	f32 temp1[ 2 ][ 2 ], temp2[ 2 ][ 2 ];
	if ( mat1 == out )
	{
		memcopy( temp1, mat1, sizeof( temp1 ) );
		mat1 = temp1;
	}
	if ( mat2 == out )
	{
		memcopy( temp2, mat2, sizeof( temp2 ) );
		mat2 = temp2;
	}
	for ( j = 0; j < 2; j++ )
	{
		for ( i = 0; i < 2; i++ )
		{
			out[ j ][ i ] = mat1[ 0 ][ i ] * mat2[ j ][ 0 ] + mat1[ 1 ][ i ] * mat2[ j ][ 1 ];
		}
	}
}

void float22_mul_vec2( vec2* out, f32 m[ 2 ][ 2 ], vec2 v )
{
	out->x = m[ 0 ][ 0 ] * v.x + m[ 0 ][ 1 ] * v.y;
	out->y = m[ 1 ][ 0 ] * v.x + m[ 1 ][ 1 ] * v.y;
}

f32 mat2_determinate( mat2* m )
{
	float2* e = float22_m( m );
	return e[ 0 ][ 0 ] * e[ 1 ][ 1 ] - e[ 1 ][ 0 ] * e[ 0 ][ 1 ];
}

void mat2_inverse( mat2* out, mat2* in )
{
	float2* o = float22_m( out );
	float2* i = float22_m( in );

	f32 ood = 1.0f / mat2_determinate( in );

	o[ 0 ][ 0 ] = +i[ 1 ][ 1 ] * ood;
	o[ 0 ][ 1 ] = -i[ 0 ][ 1 ] * ood;
	o[ 1 ][ 0 ] = -i[ 1 ][ 0 ] * ood;
	o[ 1 ][ 1 ] = +i[ 0 ][ 0 ] * ood;
}

void mat3_transpose( mat3* m )
{
	float33_transpose( float33_m( m ) );
}

void mat3_identity( mat3* m )
{
	float33_identity( float33_m( m ) );
}

void mat3_copy( mat3* out, mat3* m )
{
	memcopy( out, m, sizeof( mat3 ) );
}

void mat3_mul( mat3* out, mat3* m1, mat3* m2 )
{
	float33_mul( float33_m( out ), float33_m( m1 ), float33_m( m2 ) );
}

void float33_identity( f32 m[ 3 ][ 3 ] )
{
	m[ 0 ][ 0 ] = 1;
	m[ 0 ][ 1 ] = 0;
	m[ 0 ][ 2 ] = 0;
	m[ 1 ][ 0 ] = 0;
	m[ 1 ][ 1 ] = 1;
	m[ 1 ][ 2 ] = 0;
	m[ 2 ][ 0 ] = 0;
	m[ 2 ][ 1 ] = 0;
	m[ 2 ][ 2 ] = 1;
}

void mat3_mul_vec3( vec3* out, mat3* m, vec3 in )
{
	float33_mul_vec3( out, float33_m( m ), in );
}

mat3* mat3_v( vec3 m[ 3 ] )
{
	return ( mat3* )m;
}

mat3* mat3_f( f32 m[ 3 ][ 3 ] )
{
	return ( mat3* )m;
}

float3* float33_m( mat3* m )
{
	return ( float3* )m;
}

float3* float33_v( vec3 m[ 3 ] )
{
	return ( float3* )m;
}

float3* float33_9( f32 m[ 9 ] )
{
	return ( float3* )m;
}

void float33_transpose( f32 ( *vec )[ 3 ] )
{
	int i, j;
	for ( j = 0; j < 3; j++ )
	{
		for ( i = j + 1; i < 3; i++ )
		{
			f32 t         = vec[ i ][ j ];
			vec[ i ][ j ] = vec[ j ][ i ];
			vec[ j ][ i ] = t;
		}
	}
}

void float33_mul( f32 ( *out )[ 3 ], f32 ( *mat1 )[ 3 ], f32 ( *mat2 )[ 3 ] )
{
	int i, j;
	f32 temp1[ 3 ][ 3 ], temp2[ 3 ][ 3 ];
	if ( mat1 == out )
	{
		memcopy( temp1, mat1, sizeof( temp1 ) );
		mat1 = temp1;
	}
	if ( mat2 == out )
	{
		memcopy( temp2, mat2, sizeof( temp2 ) );
		mat2 = temp2;
	}
	for ( j = 0; j < 3; j++ )
	{
		for ( i = 0; i < 3; i++ )
		{
			out[ j ][ i ] = mat1[ 0 ][ i ] * mat2[ j ][ 0 ] + mat1[ 1 ][ i ] * mat2[ j ][ 1 ] + mat1[ 2 ][ i ] * mat2[ j ][ 2 ];
		}
	}
}

void float33_mul_vec3( vec3* out, f32 m[ 3 ][ 3 ], vec3 v )
{
	out->x = m[ 0 ][ 0 ] * v.x + m[ 0 ][ 1 ] * v.y + m[ 0 ][ 2 ] * v.z;
	out->y = m[ 1 ][ 0 ] * v.x + m[ 1 ][ 1 ] * v.y + m[ 1 ][ 2 ] * v.z;
	out->z = m[ 2 ][ 0 ] * v.x + m[ 2 ][ 1 ] * v.y + m[ 2 ][ 2 ] * v.z;
}

f32 mat3_determinate( mat3* m )
{
	float3* e = float33_m( m );
	f32     d = +e[ 0 ][ 0 ] * ( e[ 1 ][ 1 ] * e[ 2 ][ 2 ] - e[ 1 ][ 2 ] * e[ 2 ][ 1 ] ) - e[ 0 ][ 1 ] * ( e[ 1 ][ 0 ] * e[ 2 ][ 2 ] - e[ 1 ][ 2 ] * e[ 2 ][ 0 ] )
	    + e[ 0 ][ 2 ] * ( e[ 1 ][ 0 ] * e[ 2 ][ 1 ] - e[ 1 ][ 1 ] * e[ 2 ][ 0 ] );
	return d;
}

void mat3_inverse( mat3* out, mat3* in )
{
	float3* o = float33_m( out );
	float3* i = float33_m( in );

	f32 ood = 1.0f / mat3_determinate( in );

	o[ 0 ][ 0 ] = +( i[ 1 ][ 1 ] * i[ 2 ][ 2 ] - i[ 2 ][ 1 ] * i[ 1 ][ 2 ] ) * ood;
	o[ 0 ][ 1 ] = -( i[ 1 ][ 0 ] * i[ 2 ][ 2 ] - i[ 2 ][ 0 ] * i[ 1 ][ 2 ] ) * ood;
	o[ 0 ][ 2 ] = +( i[ 1 ][ 0 ] * i[ 2 ][ 1 ] - i[ 2 ][ 0 ] * i[ 1 ][ 1 ] ) * ood;
	o[ 1 ][ 0 ] = -( i[ 0 ][ 1 ] * i[ 2 ][ 2 ] - i[ 2 ][ 1 ] * i[ 0 ][ 2 ] ) * ood;
	o[ 1 ][ 1 ] = +( i[ 0 ][ 0 ] * i[ 2 ][ 2 ] - i[ 2 ][ 0 ] * i[ 0 ][ 2 ] ) * ood;
	o[ 1 ][ 2 ] = -( i[ 0 ][ 0 ] * i[ 2 ][ 1 ] - i[ 2 ][ 0 ] * i[ 0 ][ 1 ] ) * ood;
	o[ 2 ][ 0 ] = +( i[ 0 ][ 1 ] * i[ 1 ][ 2 ] - i[ 1 ][ 1 ] * i[ 0 ][ 2 ] ) * ood;
	o[ 2 ][ 1 ] = -( i[ 0 ][ 0 ] * i[ 1 ][ 2 ] - i[ 1 ][ 0 ] * i[ 0 ][ 2 ] ) * ood;
	o[ 2 ][ 2 ] = +( i[ 0 ][ 0 ] * i[ 1 ][ 1 ] - i[ 1 ][ 0 ] * i[ 0 ][ 1 ] ) * ood;
}

void mat4_transpose( mat4* m )
{
	float44_transpose( float44_m( m ) );
}

void mat4_identity( mat4* m )
{
	float44_identity( float44_m( m ) );
}

void mat4_copy( mat4* out, mat4* m )
{
	memcopy( out, m, sizeof( mat4 ) );
}

void mat4_mul( mat4* out, mat4* m1, mat4* m2 )
{
	float44_mul( float44_m( out ), float44_m( m1 ), float44_m( m2 ) );
}

void float44_identity( f32 m[ 4 ][ 4 ] )
{
	m[ 0 ][ 0 ] = 1;
	m[ 0 ][ 1 ] = 0;
	m[ 0 ][ 2 ] = 0;
	m[ 0 ][ 3 ] = 0;
	m[ 1 ][ 0 ] = 0;
	m[ 1 ][ 1 ] = 1;
	m[ 1 ][ 2 ] = 0;
	m[ 1 ][ 3 ] = 0;
	m[ 2 ][ 0 ] = 0;
	m[ 2 ][ 1 ] = 0;
	m[ 2 ][ 2 ] = 1;
	m[ 2 ][ 3 ] = 0;
	m[ 3 ][ 0 ] = 0;
	m[ 3 ][ 1 ] = 0;
	m[ 3 ][ 2 ] = 0;
	m[ 3 ][ 3 ] = 1;
}

void mat4_mul_vec4( vec4* out, mat4* m, vec4 in )
{
	float44_mul_vec4( out, float44_m( m ), in );
}

mat4* mat4_v( vec4 m[ 4 ] )
{
	return ( mat4* )m;
}

mat4* mat4_f( f32 m[ 4 ][ 4 ] )
{
	return ( mat4* )m;
}

float4* float44_m( mat4* m )
{
	return ( float4* )m;
}

float4* float44_v( vec4 m[ 4 ] )
{
	return ( float4* )m;
}

float4* float44_16( f32 m[ 16 ] )
{
	return ( float4* )m;
}

void float44_transpose( f32 ( *vec )[ 4 ] )
{
	f32 tmp;
	tmp           = vec[ 1 ][ 0 ];
	vec[ 1 ][ 0 ] = vec[ 0 ][ 1 ];
	vec[ 0 ][ 1 ] = tmp;
	tmp           = vec[ 2 ][ 0 ];
	vec[ 2 ][ 0 ] = vec[ 0 ][ 2 ];
	vec[ 0 ][ 2 ] = tmp;
	tmp           = vec[ 3 ][ 0 ];
	vec[ 3 ][ 0 ] = vec[ 0 ][ 3 ];
	vec[ 0 ][ 3 ] = tmp;
	tmp           = vec[ 2 ][ 1 ];
	vec[ 2 ][ 1 ] = vec[ 1 ][ 2 ];
	vec[ 1 ][ 2 ] = tmp;
	tmp           = vec[ 3 ][ 1 ];
	vec[ 3 ][ 1 ] = vec[ 1 ][ 3 ];
	vec[ 1 ][ 3 ] = tmp;
	tmp           = vec[ 3 ][ 2 ];
	vec[ 3 ][ 2 ] = vec[ 2 ][ 3 ];
	vec[ 2 ][ 3 ] = tmp;
}

void float44_mul( f32 ( *out )[ 4 ], f32 ( *mat1 )[ 4 ], f32 ( *mat2 )[ 4 ] )
{
	int i, j;
	f32 temp1[ 4 ][ 4 ], temp2[ 4 ][ 4 ];
	if ( mat1 == out )
	{
		memcopy( temp1, mat1, sizeof( temp1 ) );
		mat1 = temp1;
	}
	if ( mat2 == out )
	{
		memcopy( temp2, mat2, sizeof( temp2 ) );
		mat2 = temp2;
	}
	for ( j = 0; j < 4; j++ )
	{
		for ( i = 0; i < 4; i++ )
		{
			out[ j ][ i ] = mat1[ 0 ][ i ] * mat2[ j ][ 0 ] + mat1[ 1 ][ i ] * mat2[ j ][ 1 ] + mat1[ 2 ][ i ] * mat2[ j ][ 2 ] + mat1[ 3 ][ i ] * mat2[ j ][ 3 ];
		}
	}
}

void float44_mul_vec4( vec4* out, f32 m[ 4 ][ 4 ], vec4 v )
{
	out->x = m[ 0 ][ 0 ] * v.x + m[ 1 ][ 0 ] * v.y + m[ 2 ][ 0 ] * v.z + m[ 3 ][ 0 ] * v.w;
	out->y = m[ 0 ][ 1 ] * v.x + m[ 1 ][ 1 ] * v.y + m[ 2 ][ 1 ] * v.z + m[ 3 ][ 1 ] * v.w;
	out->z = m[ 0 ][ 2 ] * v.x + m[ 1 ][ 2 ] * v.y + m[ 2 ][ 2 ] * v.z + m[ 3 ][ 2 ] * v.w;
	out->w = m[ 0 ][ 3 ] * v.x + m[ 1 ][ 3 ] * v.y + m[ 2 ][ 3 ] * v.z + m[ 3 ][ 3 ] * v.w;
}

void mat4_inverse( mat4* out, mat4* in )
{
	float4* o = float44_m( out );
	float4* m = float44_m( in );

	f32 ood;

	f32 sf00 = m[ 2 ][ 2 ] * m[ 3 ][ 3 ] - m[ 3 ][ 2 ] * m[ 2 ][ 3 ];
	f32 sf01 = m[ 2 ][ 1 ] * m[ 3 ][ 3 ] - m[ 3 ][ 1 ] * m[ 2 ][ 3 ];
	f32 sf02 = m[ 2 ][ 1 ] * m[ 3 ][ 2 ] - m[ 3 ][ 1 ] * m[ 2 ][ 2 ];
	f32 sf03 = m[ 2 ][ 0 ] * m[ 3 ][ 3 ] - m[ 3 ][ 0 ] * m[ 2 ][ 3 ];
	f32 sf04 = m[ 2 ][ 0 ] * m[ 3 ][ 2 ] - m[ 3 ][ 0 ] * m[ 2 ][ 2 ];
	f32 sf05 = m[ 2 ][ 0 ] * m[ 3 ][ 1 ] - m[ 3 ][ 0 ] * m[ 2 ][ 1 ];
	f32 sf06 = m[ 1 ][ 2 ] * m[ 3 ][ 3 ] - m[ 3 ][ 2 ] * m[ 1 ][ 3 ];
	f32 sf07 = m[ 1 ][ 1 ] * m[ 3 ][ 3 ] - m[ 3 ][ 1 ] * m[ 1 ][ 3 ];
	f32 sf08 = m[ 1 ][ 1 ] * m[ 3 ][ 2 ] - m[ 3 ][ 1 ] * m[ 1 ][ 2 ];
	f32 sf09 = m[ 1 ][ 0 ] * m[ 3 ][ 3 ] - m[ 3 ][ 0 ] * m[ 1 ][ 3 ];
	f32 sf10 = m[ 1 ][ 0 ] * m[ 3 ][ 2 ] - m[ 3 ][ 0 ] * m[ 1 ][ 2 ];
	f32 sf11 = m[ 1 ][ 1 ] * m[ 3 ][ 3 ] - m[ 3 ][ 1 ] * m[ 1 ][ 3 ];
	f32 sf12 = m[ 1 ][ 0 ] * m[ 3 ][ 1 ] - m[ 3 ][ 0 ] * m[ 1 ][ 1 ];
	f32 sf13 = m[ 1 ][ 2 ] * m[ 2 ][ 3 ] - m[ 2 ][ 2 ] * m[ 1 ][ 3 ];
	f32 sf14 = m[ 1 ][ 1 ] * m[ 2 ][ 3 ] - m[ 2 ][ 1 ] * m[ 1 ][ 3 ];
	f32 sf15 = m[ 1 ][ 1 ] * m[ 2 ][ 2 ] - m[ 2 ][ 1 ] * m[ 1 ][ 2 ];
	f32 sf16 = m[ 1 ][ 0 ] * m[ 2 ][ 3 ] - m[ 2 ][ 0 ] * m[ 1 ][ 3 ];
	f32 sf17 = m[ 1 ][ 0 ] * m[ 2 ][ 2 ] - m[ 2 ][ 0 ] * m[ 1 ][ 2 ];
	f32 sf18 = m[ 1 ][ 0 ] * m[ 2 ][ 1 ] - m[ 2 ][ 0 ] * m[ 1 ][ 1 ];

	o[ 0 ][ 0 ] = +( m[ 1 ][ 1 ] * sf00 - m[ 1 ][ 2 ] * sf01 + m[ 1 ][ 3 ] * sf02 );
	o[ 1 ][ 0 ] = -( m[ 1 ][ 0 ] * sf00 - m[ 1 ][ 2 ] * sf03 + m[ 1 ][ 3 ] * sf04 );
	o[ 2 ][ 0 ] = +( m[ 1 ][ 0 ] * sf01 - m[ 1 ][ 1 ] * sf03 + m[ 1 ][ 3 ] * sf05 );
	o[ 3 ][ 0 ] = -( m[ 1 ][ 0 ] * sf02 - m[ 1 ][ 1 ] * sf04 + m[ 1 ][ 2 ] * sf05 );

	o[ 0 ][ 1 ] = -( m[ 0 ][ 1 ] * sf00 - m[ 0 ][ 2 ] * sf01 + m[ 0 ][ 3 ] * sf02 );
	o[ 1 ][ 1 ] = +( m[ 0 ][ 0 ] * sf00 - m[ 0 ][ 2 ] * sf03 + m[ 0 ][ 3 ] * sf04 );
	o[ 2 ][ 1 ] = -( m[ 0 ][ 0 ] * sf01 - m[ 0 ][ 1 ] * sf03 + m[ 0 ][ 3 ] * sf05 );
	o[ 3 ][ 1 ] = +( m[ 0 ][ 0 ] * sf02 - m[ 0 ][ 1 ] * sf04 + m[ 0 ][ 2 ] * sf05 );

	o[ 0 ][ 2 ] = +( m[ 0 ][ 1 ] * sf06 - m[ 0 ][ 2 ] * sf07 + m[ 0 ][ 3 ] * sf08 );
	o[ 1 ][ 2 ] = -( m[ 0 ][ 0 ] * sf06 - m[ 0 ][ 2 ] * sf09 + m[ 0 ][ 3 ] * sf10 );
	o[ 2 ][ 2 ] = +( m[ 0 ][ 0 ] * sf11 - m[ 0 ][ 1 ] * sf09 + m[ 0 ][ 3 ] * sf12 );
	o[ 3 ][ 2 ] = -( m[ 0 ][ 0 ] * sf08 - m[ 0 ][ 1 ] * sf10 + m[ 0 ][ 2 ] * sf12 );

	o[ 0 ][ 3 ] = -( m[ 0 ][ 1 ] * sf13 - m[ 0 ][ 2 ] * sf14 + m[ 0 ][ 3 ] * sf15 );
	o[ 1 ][ 3 ] = +( m[ 0 ][ 0 ] * sf13 - m[ 0 ][ 2 ] * sf16 + m[ 0 ][ 3 ] * sf17 );
	o[ 2 ][ 3 ] = -( m[ 0 ][ 0 ] * sf14 - m[ 0 ][ 1 ] * sf16 + m[ 0 ][ 3 ] * sf18 );
	o[ 3 ][ 3 ] = +( m[ 0 ][ 0 ] * sf15 - m[ 0 ][ 1 ] * sf17 + m[ 0 ][ 2 ] * sf18 );

	ood = 1.0f / ( m[ 0 ][ 0 ] * o[ 0 ][ 0 ] + m[ 0 ][ 1 ] * o[ 1 ][ 0 ] + m[ 0 ][ 2 ] * o[ 2 ][ 0 ] + m[ 0 ][ 3 ] * o[ 3 ][ 0 ] );

	o[ 0 ][ 0 ] *= ood;
	o[ 1 ][ 0 ] *= ood;
	o[ 2 ][ 0 ] *= ood;
	o[ 3 ][ 0 ] *= ood;
	o[ 0 ][ 1 ] *= ood;
	o[ 1 ][ 1 ] *= ood;
	o[ 2 ][ 1 ] *= ood;
	o[ 3 ][ 1 ] *= ood;
	o[ 0 ][ 2 ] *= ood;
	o[ 1 ][ 2 ] *= ood;
	o[ 2 ][ 2 ] *= ood;
	o[ 3 ][ 2 ] *= ood;
	o[ 0 ][ 3 ] *= ood;
	o[ 1 ][ 3 ] *= ood;
	o[ 2 ][ 3 ] *= ood;
	o[ 3 ][ 3 ] *= ood;
}

void mat4_axis_angle( mat4* out, vec3 v, f32 angle_radians )
{
	f32     c, s;
	vec3    axis, t;
	float4* rot;

	c = cos( angle_radians );
	s = sin( angle_radians );

	vec3_norm( &axis, v );
	vec3_mul( &t, axis, 1.0f - c );

	mat4_identity( out );
	rot = float44_m( out );

	rot[ 0 ][ 0 ] = c + t.x * axis.x;
	rot[ 0 ][ 1 ] = 0 + t.x * axis.y + s * axis.z;
	rot[ 0 ][ 2 ] = 0 + t.x * axis.z - s * axis.y;
	rot[ 0 ][ 3 ] = 0;

	rot[ 1 ][ 0 ] = 0 + t.y * axis.x - s * axis.z;
	rot[ 1 ][ 1 ] = c + t.y * axis.y;
	rot[ 1 ][ 2 ] = 0 + t.y * axis.z + s * axis.x;
	rot[ 1 ][ 3 ] = 0;

	rot[ 2 ][ 0 ] = 0 + t.z * axis.x + s * axis.y;
	rot[ 2 ][ 1 ] = 0 + t.z * axis.y - s * axis.x;
	rot[ 2 ][ 2 ] = c + t.z * axis.z;
	rot[ 2 ][ 3 ] = 0;
}

void mat4_to_translate( mat4* out, vec3 v )
{
	mat4_identity( out );
	out->col[ 3 ].xyz = v;
}

void mat4_to_rotate( mat4* out, vec3 v, f32 angle_radians )
{
	mat4_axis_angle( out, v, angle_radians );
}

void mat4_to_scale( mat4* out, vec3 v )
{
	mat4_identity( out );
	out->col[ 0 ].x = v.x;
	out->col[ 1 ].y = v.y;
	out->col[ 2 ].z = v.z;
}

void mat4_to_scalef( mat4* out, f32 s )
{
	mat4_identity( out );
	out->col[ 0 ].x = s;
	out->col[ 1 ].y = s;
	out->col[ 2 ].z = s;
}

void mat4_translate( mat4* m, vec3 v )
{
	mat4 mm;
	mat4_to_translate( &mm, v );
	mat4_mul( m, m, &mm );
}

void mat4_rotate( mat4* m, vec3 v, f32 angle_radians )
{
	mat4 mm;
	mat4_axis_angle( &mm, v, angle_radians );
	mat4_mul( m, m, &mm );
}

void mat4_scale( mat4* m, vec3 v )
{
	mat4 mm;
	mat4_to_scale( &mm, v );
	mat4_mul( m, m, &mm );
}

void mat4_scalef( mat4* m, f32 s )
{
	mat4 mm;
	mat4_to_scalef( &mm, s );
	mat4_mul( m, m, &mm );
}

void mat4_ortho2d( mat4* out, f32 left, f32 right, f32 bottom, f32 top )
{
	float4* m;
	mat4_identity( out );
	m = float44_m( out );

	m[ 0 ][ 0 ] = 2.0f / ( right - left );
	m[ 1 ][ 1 ] = 2.0f / ( top - bottom );
	m[ 2 ][ 2 ] = -1.0f;
	m[ 3 ][ 0 ] = -( right + left ) / ( right - left );
	m[ 3 ][ 1 ] = -( top + bottom ) / ( top - bottom );
}

void mat4_ortho3d( mat4* out, f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far )
{
	float4* m;
	mat4_identity( out );
	m = float44_m( out );

	m[ 0 ][ 0 ] = +2.0f / ( right - left );
	m[ 1 ][ 1 ] = +2.0f / ( top - bottom );
	m[ 2 ][ 2 ] = -2.0f / ( z_far - z_near );
	m[ 3 ][ 0 ] = -( right + left ) / ( right - left );
	m[ 3 ][ 1 ] = -( top + bottom ) / ( top - bottom );
	m[ 3 ][ 2 ] = -( z_far + z_near ) / ( z_far - z_near );
}

void mat4_perspective( mat4* out, f32 fovy, f32 aspect, f32 z_near, f32 z_far )
{
	f32     tan_half_fovy = tan( 0.5f * fovy );
	mat4    zero_mat      = { 0 };
	float4* m             = float44_m( out );
	*out                  = zero_mat;

	m[ 0 ][ 0 ] = 1.0f / ( aspect * tan_half_fovy );
	m[ 1 ][ 1 ] = 1.0f / ( tan_half_fovy );
	m[ 2 ][ 2 ] = -( z_far + z_near ) / ( z_far - z_near );
	m[ 2 ][ 3 ] = -1.0f;
	m[ 3 ][ 2 ] = -2.0f * z_far * z_near / ( z_far - z_near );
}

void mat4_infinite_perspective( mat4* out, f32 fovy, f32 aspect, f32 z_near )
{
	f32     range    = tan( 0.5f * fovy ) * z_near;
	f32     left     = -range * aspect;
	f32     right    = range * aspect;
	f32     bottom   = -range;
	f32     top      = range;
	mat4    zero_mat = { 0 };
	float4* m        = float44_m( out );
	*out             = zero_mat;

	m[ 0 ][ 0 ] = ( 2.0f * z_near ) / ( right - left );
	m[ 1 ][ 1 ] = ( 2.0f * z_near ) / ( top - bottom );
	m[ 2 ][ 2 ] = -1.0f;
	m[ 2 ][ 3 ] = -1.0f;
	m[ 3 ][ 2 ] = -2.0f * z_near;
}

void mat4_ortho2d_dx( mat4* out, f32 left, f32 right, f32 bottom, f32 top )
{
	float4* m;
	mat4_identity( out );
	m = float44_m( out );

	m[ 0 ][ 0 ] = 2.0f / ( right - left );
	m[ 1 ][ 1 ] = 2.0f / ( top - bottom );
	m[ 2 ][ 2 ] = -1.0f;
	m[ 3 ][ 0 ] = -( right + left ) / ( right - left );
	m[ 3 ][ 1 ] = -( top + bottom ) / ( top - bottom );
}

void mat4_ortho3d_dx( mat4* out, f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far )
{
	float4* m;
	mat4_identity( out );
	m = float44_m( out );

	m[ 0 ][ 0 ] = +2.0f / ( right - left );
	m[ 1 ][ 1 ] = +2.0f / ( top - bottom );
	m[ 2 ][ 2 ] = -1.0f / ( z_far - z_near );
	m[ 3 ][ 0 ] = -( right + left ) / ( right - left );
	m[ 3 ][ 1 ] = -( top + bottom ) / ( top - bottom );
	m[ 3 ][ 2 ] = -( z_near ) / ( z_far - z_near );
}

void mat4_perspective_dx( mat4* out, f32 fovy, f32 aspect, f32 z_near, f32 z_far )
{
	f32     tan_half_fovy = tan( 0.5f * fovy );
	mat4    zero_mat      = { 0 };
	float4* m             = float44_m( out );
	*out                  = zero_mat;

	m[ 0 ][ 0 ] = 1.0f / ( aspect * tan_half_fovy );
	m[ 1 ][ 1 ] = 1.0f / ( tan_half_fovy );
	m[ 2 ][ 2 ] = -( z_far ) / ( z_far - z_near );
	m[ 2 ][ 3 ] = -1.0f;
	m[ 3 ][ 2 ] = -z_near / ( z_far - z_near );
}

void mat4_infinite_perspective_dx( mat4* out, f32 fovy, f32 aspect, f32 z_near )
{
	f32     tan_half_fovy = tan( 0.5f * fovy );
	mat4    zero_mat      = { 0 };
	float4* m             = float44_m( out );
	*out                  = zero_mat;

	m[ 0 ][ 0 ] = 1.0f / ( aspect * tan_half_fovy );
	m[ 1 ][ 1 ] = 1.0f / ( tan_half_fovy );
	m[ 2 ][ 2 ] = -1.0f;
	m[ 2 ][ 3 ] = -1.0f;
	m[ 3 ][ 2 ] = -z_near;
}

void mat4_look_at( mat4* out, vec3 eye, vec3 centre, vec3 up )
{
	vec3    f, s, u;
	float4* m;

	vec3_sub( &f, centre, eye );
	vec3_norm( &f, f );

	vec3_cross( &s, f, up );
	vec3_norm( &s, s );

	vec3_cross( &u, s, f );

	mat4_identity( out );
	m = float44_m( out );

	m[ 0 ][ 0 ] = +s.x;
	m[ 1 ][ 0 ] = +s.y;
	m[ 2 ][ 0 ] = +s.z;

	m[ 0 ][ 1 ] = +u.x;
	m[ 1 ][ 1 ] = +u.y;
	m[ 2 ][ 1 ] = +u.z;

	m[ 0 ][ 2 ] = -f.x;
	m[ 1 ][ 2 ] = -f.y;
	m[ 2 ][ 2 ] = -f.z;

	m[ 3 ][ 0 ] = -vec3_dot( s, eye );
	m[ 3 ][ 1 ] = -vec3_dot( u, eye );
	m[ 3 ][ 2 ] = +vec3_dot( f, eye );
}

void mat4_look_at_lh( mat4* out, vec3 eye, vec3 centre, vec3 up )
{
	vec3    f, s, u;
	float4* m;

	vec3_sub( &f, centre, eye );
	vec3_norm( &f, f );

	vec3_cross( &s, up, f );
	vec3_norm( &s, s );

	vec3_cross( &u, f, s );

	mat4_identity( out );
	m = float44_m( out );

	m[ 0 ][ 0 ] = +s.x;
	m[ 1 ][ 0 ] = +s.y;
	m[ 2 ][ 0 ] = +s.z;

	m[ 0 ][ 1 ] = +u.x;
	m[ 1 ][ 1 ] = +u.y;
	m[ 2 ][ 1 ] = +u.z;

	m[ 0 ][ 2 ] = +f.x;
	m[ 1 ][ 2 ] = +f.y;
	m[ 2 ][ 2 ] = +f.z;

	m[ 3 ][ 0 ] = -vec3_dot( s, eye );
	m[ 3 ][ 1 ] = -vec3_dot( u, eye );
	m[ 3 ][ 2 ] = -vec3_dot( f, eye );
}

quat quatf( f32 x, f32 y, f32 z, f32 w )
{
	quat q;
	q.x = x;
	q.y = y;
	q.z = z;
	q.w = w;
	return q;
}

quat quatfv( f32 e[ 4 ] )
{
	quat q;
	q.x = e[ 0 ];
	q.y = e[ 1 ];
	q.z = e[ 2 ];
	q.w = e[ 3 ];
	return q;
}

quat quat_axis_angle( vec3 axis, f32 angle_radians )
{
	quat q;
	vec3_norm( &q.xyz, axis );
	vec3_muleq( &q.xyz, sin( 0.5f * angle_radians ) );
	q.w = cos( 0.5f * angle_radians );
	return q;
}

quat quat_euler_angles( f32 pitch, f32 yaw, f32 roll )
{
	/* TODO: Do without multiplication, i.e. make it faster */
	quat q, p, y, r;
	p = quat_axis_angle( vec3f( 1, 0, 0 ), pitch );
	y = quat_axis_angle( vec3f( 0, 1, 0 ), yaw );
	r = quat_axis_angle( vec3f( 0, 0, 1 ), roll );

	quat_mul( &q, y, p );
	quat_muleq( &q, r );

	return q;
}

quat quat_identity( void )
{
	quat q = { 0, 0, 0, 1 };
	return q;
}

void quat_add( quat* d, quat q0, quat q1 )
{
	vec4_add( &d->xyzw, q0.xyzw, q1.xyzw );
}

void quat_sub( quat* d, quat q0, quat q1 )
{
	vec4_sub( &d->xyzw, q0.xyzw, q1.xyzw );
}

void quat_mul( quat* d, quat q0, quat q1 )
{
	d->x = q0.w * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y;
	d->y = q0.w * q1.y - q0.x * q1.z + q0.y * q1.w + q0.z * q1.x;
	d->z = q0.w * q1.z + q0.x * q1.y - q0.y * q1.x + q0.z * q1.w;
	d->w = q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z;
}

void quat_div( quat* d, quat q0, quat q1 )
{
	quat iq1;
	quat_inverse( &iq1, q1 );
	quat_mul( d, q0, iq1 );
}

void quat_mulf( quat* d, quat q0, f32 s )
{
	vec4_mul( &d->xyzw, q0.xyzw, s );
}

void quat_divf( quat* d, quat q0, f32 s )
{
	vec4_div( &d->xyzw, q0.xyzw, s );
}

void quat_addeq( quat* d, quat q )
{
	vec4_addeq( &d->xyzw, q.xyzw );
}

void quat_subeq( quat* d, quat q )
{
	vec4_subeq( &d->xyzw, q.xyzw );
}

void quat_muleq( quat* d, quat q )
{
	quat_mul( d, *d, q );
}

void quat_diveq( quat* d, quat q )
{
	quat_div( d, *d, q );
}

void quat_muleqf( quat* d, f32 s )
{
	vec4_muleq( &d->xyzw, s );
}

void quat_diveqf( quat* d, f32 s )
{
	vec4_diveq( &d->xyzw, s );
}

f32 quat_dot( quat q0, quat q1 )
{
	f32 r = vec3_dot( q0.xyz, q1.xyz ) + q0.w * q1.w;
	return r;
}

f32 quat_mag( quat q )
{
	f32 r = sqrt( quat_dot( q, q ) );
	return r;
}

void quat_norm( quat* d, quat q )
{
	quat_divf( d, q, quat_mag( q ) );
}

void quat_conj( quat* d, quat q )
{
	d->xyz = vec3f( -q.x, -q.y, -q.z );
	d->w   = q.w;
}

void quat_inverse( quat* d, quat q )
{
	quat_conj( d, q );
	quat_diveqf( d, quat_dot( q, q ) );
}

void quat_axis( vec3* axis, quat q )
{
	quat n;
	quat_norm( &n, q );
	vec3_div( axis, n.xyz, sin( arccos( q.w ) ) );
}

f32 quat_angle( quat q )
{
	f32 mag   = quat_mag( q );
	f32 c     = q.w * ( 1.0f / mag );
	f32 angle = 2.0f * arccos( c );
	return angle;
}

f32 quat_roll( quat q )
{
	return arctan2( 2.0f * q.x * q.y + q.z * q.w, q.x * q.x + q.w * q.w - q.y * q.y - q.z * q.z );
}

f32 quat_pitch( quat q )
{
	return arctan2( 2.0f * q.y * q.z + q.w * q.x, q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z );
}

f32 quat_yaw( quat q )
{
	return arcsin( -2.0f * ( q.x * q.z - q.w * q.y ) );
}

void quat_rotate_vec3( vec3* d, quat q, vec3 v )
{
	/* zpl_vec3 t = 2.0f * cross(q.xyz, v);
	 * *d = q.w*t + v + cross(q.xyz, t);
	 */
	vec3 t, p;
	vec3_cross( &t, q.xyz, v );
	vec3_muleq( &t, 2.0f );

	vec3_cross( &p, q.xyz, t );

	vec3_mul( d, t, q.w );
	vec3_addeq( d, v );
	vec3_addeq( d, p );
}

void mat4_from_quat( mat4* out, quat q )
{
	float4* m;
	quat    a;
	f32     xx, yy, zz, xy, xz, yz, wx, wy, wz;

	quat_norm( &a, q );
	xx = a.x * a.x;
	yy = a.y * a.y;
	zz = a.z * a.z;
	xy = a.x * a.y;
	xz = a.x * a.z;
	yz = a.y * a.z;
	wx = a.w * a.x;
	wy = a.w * a.y;
	wz = a.w * a.z;

	mat4_identity( out );
	m = float44_m( out );

	m[ 0 ][ 0 ] = 1.0f - 2.0f * ( yy + zz );
	m[ 0 ][ 1 ] = 2.0f * ( xy + wz );
	m[ 0 ][ 2 ] = 2.0f * ( xz - wy );

	m[ 1 ][ 0 ] = 2.0f * ( xy - wz );
	m[ 1 ][ 1 ] = 1.0f - 2.0f * ( xx + zz );
	m[ 1 ][ 2 ] = 2.0f * ( yz + wx );

	m[ 2 ][ 0 ] = 2.0f * ( xz + wy );
	m[ 2 ][ 1 ] = 2.0f * ( yz - wx );
	m[ 2 ][ 2 ] = 1.0f - 2.0f * ( xx + yy );
}

void quat_from_mat4( quat* out, mat4* mat )
{
	float4* m;
	f32     four_x_squared_minus_1, four_y_squared_minus_1, four_z_squared_minus_1, four_w_squared_minus_1, four_biggest_squared_minus_1;
	int     biggest_index = 0;
	f32     biggest_value, mult;

	m = float44_m( mat );

	four_x_squared_minus_1 = m[ 0 ][ 0 ] - m[ 1 ][ 1 ] - m[ 2 ][ 2 ];
	four_y_squared_minus_1 = m[ 1 ][ 1 ] - m[ 0 ][ 0 ] - m[ 2 ][ 2 ];
	four_z_squared_minus_1 = m[ 2 ][ 2 ] - m[ 0 ][ 0 ] - m[ 1 ][ 1 ];
	four_w_squared_minus_1 = m[ 0 ][ 0 ] + m[ 1 ][ 1 ] + m[ 2 ][ 2 ];

	four_biggest_squared_minus_1 = four_w_squared_minus_1;
	if ( four_x_squared_minus_1 > four_biggest_squared_minus_1 )
	{
		four_biggest_squared_minus_1 = four_x_squared_minus_1;
		biggest_index                = 1;
	}
	if ( four_y_squared_minus_1 > four_biggest_squared_minus_1 )
	{
		four_biggest_squared_minus_1 = four_y_squared_minus_1;
		biggest_index                = 2;
	}
	if ( four_z_squared_minus_1 > four_biggest_squared_minus_1 )
	{
		four_biggest_squared_minus_1 = four_z_squared_minus_1;
		biggest_index                = 3;
	}

	biggest_value = sqrt( four_biggest_squared_minus_1 + 1.0f ) * 0.5f;
	mult          = 0.25f / biggest_value;

	switch ( biggest_index )
	{
		case 0 :
			out->w = biggest_value;
			out->x = ( m[ 1 ][ 2 ] - m[ 2 ][ 1 ] ) * mult;
			out->y = ( m[ 2 ][ 0 ] - m[ 0 ][ 2 ] ) * mult;
			out->z = ( m[ 0 ][ 1 ] - m[ 1 ][ 0 ] ) * mult;
			break;
		case 1 :
			out->w = ( m[ 1 ][ 2 ] - m[ 2 ][ 1 ] ) * mult;
			out->x = biggest_value;
			out->y = ( m[ 0 ][ 1 ] + m[ 1 ][ 0 ] ) * mult;
			out->z = ( m[ 2 ][ 0 ] + m[ 0 ][ 2 ] ) * mult;
			break;
		case 2 :
			out->w = ( m[ 2 ][ 0 ] - m[ 0 ][ 2 ] ) * mult;
			out->x = ( m[ 0 ][ 1 ] + m[ 1 ][ 0 ] ) * mult;
			out->y = biggest_value;
			out->z = ( m[ 1 ][ 2 ] + m[ 2 ][ 1 ] ) * mult;
			break;
		case 3 :
			out->w = ( m[ 0 ][ 1 ] - m[ 1 ][ 0 ] ) * mult;
			out->x = ( m[ 2 ][ 0 ] + m[ 0 ][ 2 ] ) * mult;
			out->y = ( m[ 1 ][ 2 ] + m[ 2 ][ 1 ] ) * mult;
			out->z = biggest_value;
			break;
	}
}

f32 plane_distance( plane* p, vec3 v )
{
	return ( p->a * v.x + p->b * v.y + p->c * v.z + p->d );
}

void frustum_create( frustum* out, mat4* camera, mat4* proj )
{
	mat4 pv;

	mat4_mul( &pv, camera, proj );

	plane* fp = 0;
	f32    rmag;

	fp    = &out->x1;
	fp->a = pv.x.w + pv.x.x;
	fp->b = pv.y.w + pv.x.y;
	fp->c = pv.z.w + pv.x.z;
	fp->d = pv.w.w + pv.x.w;

	rmag = rsqrt( square( fp->a ) + square( fp->b ) + square( fp->c ) );

	fp->a *= rmag;
	fp->b *= rmag;
	fp->c *= rmag;
	fp->d *= rmag;

	fp = &out->x2;

	fp->a = pv.x.w - pv.x.x;
	fp->b = pv.y.w - pv.x.y;
	fp->c = pv.z.w - pv.x.z;
	fp->d = pv.w.w - pv.x.w;

	rmag = rsqrt( square( fp->a ) + square( fp->b ) + square( fp->c ) );

	fp->a *= rmag;
	fp->b *= rmag;
	fp->c *= rmag;
	fp->d *= rmag;

	fp = &out->y1;

	fp->a = pv.x.w - pv.y.x;
	fp->b = pv.y.w - pv.y.y;
	fp->c = pv.z.w - pv.y.w;
	fp->d = pv.w.w - pv.y.z;

	rmag = rsqrt( square( fp->a ) + square( fp->b ) + square( fp->c ) );

	fp->a *= rmag;
	fp->b *= rmag;
	fp->c *= rmag;
	fp->d *= rmag;

	fp = &out->y2;

	fp->a = pv.x.w + pv.y.x;
	fp->b = pv.y.w + pv.y.y;
	fp->c = pv.z.w + pv.y.z;
	fp->d = pv.w.w + pv.y.w;

	rmag = rsqrt( square( fp->a ) + square( fp->b ) + square( fp->c ) );

	fp->a *= rmag;
	fp->b *= rmag;
	fp->c *= rmag;
	fp->d *= rmag;
	;

	fp = &out->z1;

	fp->a = pv.x.w + pv.z.x;
	fp->b = pv.y.w + pv.z.y;
	fp->c = pv.z.w + pv.z.z;
	fp->d = pv.w.w + pv.z.w;

	rmag = rsqrt( square( fp->a ) + square( fp->b ) + square( fp->c ) );

	fp->a *= rmag;
	fp->b *= rmag;
	fp->c *= rmag;
	fp->d *= rmag;

	fp = &out->z2;

	fp->a = pv.x.w - pv.z.x;
	fp->b = pv.y.w - pv.z.y;
	fp->c = pv.z.w - pv.z.z;
	fp->d = pv.w.w - pv.z.w;

	rmag = rsqrt( square( fp->a ) + square( fp->b ) + square( fp->c ) );

	fp->a *= rmag;
	fp->b *= rmag;
	fp->c *= rmag;
	fp->d *= rmag;
}

b8 frustum_sphere_inside( frustum* frustum, vec3 center, f32 radius )
{
	if ( plane_distance( &frustum->x1, center ) <= -radius )
		return 0;
	if ( plane_distance( &frustum->x2, center ) <= -radius )
		return 0;
	if ( plane_distance( &frustum->y1, center ) <= -radius )
		return 0;
	if ( plane_distance( &frustum->y2, center ) <= -radius )
		return 0;
	if ( plane_distance( &frustum->z1, center ) <= -radius )
		return 0;
	if ( plane_distance( &frustum->z2, center ) <= -radius )
		return 0;

	return 1;
}

b8 frustum_point_inside( frustum* frustum, vec3 point )
{
	return frustum_sphere_inside( frustum, point, 0.0f );
}

b8 frustum_box_inside( frustum* frustum, aabb3 aabb )
{
	vec3 box, center;
	vec3 v, b;
	vec3_sub( &box, aabb.max, aabb.min );
	vec3_diveq( &box, 2.0f );
	vec3_add( &center, aabb.min, box );

	b = vec3f( -box.x, -box.y, -box.z );
	vec3_add( &v, b, center );

	if ( frustum_point_inside( frustum, v ) )
		return 1;

	b = vec3f( +box.x, -box.y, -box.z );
	vec3_add( &v, b, center );

	if ( frustum_point_inside( frustum, v ) )
		return 1;

	b = vec3f( -box.x, +box.y, -box.z );
	vec3_add( &v, b, center );

	if ( frustum_point_inside( frustum, v ) )
		return 1;

	b = vec3f( +box.x, +box.y, -box.z );
	vec3_add( &v, b, center );

	if ( frustum_point_inside( frustum, v ) )
		return 1;

	b = vec3f( +box.x, +box.y, +box.z );
	vec3_add( &v, b, center );

	if ( frustum_point_inside( frustum, v ) )
		return 1;

	b = vec3f( -box.x, +box.y, +box.z );
	vec3_add( &v, b, center );

	if ( frustum_point_inside( frustum, v ) )
		return 1;

	b = vec3f( -box.x, -box.y, +box.z );
	vec3_add( &v, b, center );

	if ( frustum_point_inside( frustum, v ) )
		return 1;

	b = vec3f( +box.x, -box.y, +box.z );
	vec3_add( &v, b, center );

	if ( frustum_point_inside( frustum, v ) )
		return 1;

	return 0;
}

f32 lerp( f32 a, f32 b, f32 t )
{
	return a * ( 1.0f - t ) + b * t;
}

f32 unlerp( f32 t, f32 a, f32 b )
{
	return ( t - a ) / ( b - a );
}

f32 smooth_step( f32 a, f32 b, f32 t )
{
	f32 x = ( t - a ) / ( b - a );
	return x * x * ( 3.0f - 2.0f * x );
}

f32 smoother_step( f32 a, f32 b, f32 t )
{
	f32 x = ( t - a ) / ( b - a );
	return x * x * x * ( x * ( 6.0f * x - 15.0f ) + 10.0f );
}

#define ZPL_VEC_LERPN( N, d, a, b, t ) \
	vec##N db;                         \
	vec##N##_sub( &db, b, a );         \
	vec##N##_muleq( &db, t );          \
	vec##N##_add( d, a, db )

void vec2_lerp( vec2* d, vec2 a, vec2 b, f32 t )
{
	ZPL_VEC_LERPN( 2, d, a, b, t );
}

void vec3_lerp( vec3* d, vec3 a, vec3 b, f32 t )
{
	ZPL_VEC_LERPN( 3, d, a, b, t );
}

void vec4_lerp( vec4* d, vec4 a, vec4 b, f32 t )
{
	ZPL_VEC_LERPN( 4, d, a, b, t );
}

#undef ZPL_VEC_LERPN

void vec2_cslerp( vec2* d, vec2 a, vec2 v0, vec2 b, vec2 v1, f32 t )
{
	f32 t2  = t * t;
	f32 ti  = ( t - 1 );
	f32 ti2 = ti * ti;

	f32 h00 = ( 1 + 2 * t ) * ti2;
	f32 h10 = t * ti2;
	f32 h01 = t2 * ( 3 - 2 * t );
	f32 h11 = t2 * ti;

	d->x = h00 * a.x + h10 * v0.x + h01 * b.x + h11 * v1.x;
	d->y = h00 * a.y + h10 * v0.y + h01 * b.y + h11 * v1.y;
}

void vec3_cslerp( vec3* d, vec3 a, vec3 v0, vec3 b, vec3 v1, f32 t )
{
	f32 t2  = t * t;
	f32 ti  = ( t - 1 );
	f32 ti2 = ti * ti;

	f32 h00 = ( 1 + 2 * t ) * ti2;
	f32 h10 = t * ti2;
	f32 h01 = t2 * ( 3 - 2 * t );
	f32 h11 = t2 * ti;

	d->x = h00 * a.x + h10 * v0.x + h01 * b.x + h11 * v1.x;
	d->y = h00 * a.y + h10 * v0.y + h01 * b.y + h11 * v1.y;
	d->z = h00 * a.z + h10 * v0.z + h01 * b.z + h11 * v1.z;
}

void vec2_dcslerp( vec2* d, vec2 a, vec2 v0, vec2 b, vec2 v1, f32 t )
{
	f32 t2 = t * t;

	f32 dh00 = 6 * t2 - 6 * t;
	f32 dh10 = 3 * t2 - 4 * t + 1;
	f32 dh01 = -6 * t2 + 6 * t;
	f32 dh11 = 3 * t2 - 2 * t;

	d->x = dh00 * a.x + dh10 * v0.x + dh01 * b.x + dh11 * v1.x;
	d->y = dh00 * a.y + dh10 * v0.y + dh01 * b.y + dh11 * v1.y;
}

void vec3_dcslerp( vec3* d, vec3 a, vec3 v0, vec3 b, vec3 v1, f32 t )
{
	f32 t2 = t * t;

	f32 dh00 = 6 * t2 - 6 * t;
	f32 dh10 = 3 * t2 - 4 * t + 1;
	f32 dh01 = -6 * t2 + 6 * t;
	f32 dh11 = 3 * t2 - 2 * t;

	d->x = dh00 * a.x + dh10 * v0.x + dh01 * b.x + dh11 * v1.x;
	d->y = dh00 * a.y + dh10 * v0.y + dh01 * b.y + dh11 * v1.y;
	d->z = dh00 * a.z + dh10 * v0.z + dh01 * b.z + dh11 * v1.z;
}

void quat_lerp( quat* d, quat a, quat b, f32 t )
{
	vec4_lerp( &d->xyzw, a.xyzw, b.xyzw, t );
}

void quat_nlerp( quat* d, quat a, quat b, f32 t )
{
	quat_lerp( d, a, b, t );
	quat_norm( d, *d );
}

void quat_slerp( quat* d, quat a, quat b, f32 t )
{
	quat x, y, z;
	f32  cos_theta, angle;
	f32  s1, s0, is;

	z         = b;
	cos_theta = quat_dot( a, b );

	if ( cos_theta < 0.0f )
	{
		z         = quatf( -b.x, -b.y, -b.z, -b.w );
		cos_theta = -cos_theta;
	}

	if ( cos_theta > 1.0f )
	{
		/* NOTE: Use lerp not nlerp as it's not a real angle or they are not normalized */
		quat_lerp( d, a, b, t );
	}

	angle = arccos( cos_theta );

	s1 = sin( ( 1.0f - t ) * angle );
	s0 = sin( t * angle );
	is = 1.0f / sin( angle );
	quat_mulf( &x, a, s1 );
	quat_mulf( &y, z, s0 );
	quat_add( d, x, y );
	quat_muleqf( d, is );
}

void quat_slerp_approx( quat* d, quat a, quat b, f32 t )
{
	/* NOTE: Derived by taylor expanding the geometric interpolation equation
	 *             Even works okay for nearly anti-parallel versors!!!
	 */
	/* NOTE: Extra interations cannot be used as they require angle^4 which is not worth it to approximate */
	f32 tp = t + ( 1.0f - quat_dot( a, b ) ) / 3.0f * t * ( -2.0f * t * t + 3.0f * t - 1.0f );
	quat_nlerp( d, a, b, tp );
}

void quat_nquad( quat* d, quat p, quat a, quat b, quat q, f32 t )
{
	quat x, y;
	quat_nlerp( &x, p, q, t );
	quat_nlerp( &y, a, b, t );
	quat_nlerp( d, x, y, 2.0f * t * ( 1.0f - t ) );
}

void quat_squad( quat* d, quat p, quat a, quat b, quat q, f32 t )
{
	quat x, y;
	quat_slerp( &x, p, q, t );
	quat_slerp( &y, a, b, t );
	quat_slerp( d, x, y, 2.0f * t * ( 1.0f - t ) );
}

void quat_squad_approx( quat* d, quat p, quat a, quat b, quat q, f32 t )
{
	quat x, y;
	quat_slerp_approx( &x, p, q, t );
	quat_slerp_approx( &y, a, b, t );
	quat_slerp_approx( d, x, y, 2.0f * t * ( 1.0f - t ) );
}

rect2 rect2f( vec2 pos, vec2 dim )
{
	rect2 r;
	r.pos = pos;
	r.dim = dim;
	return r;
}

rect3 rect3f( vec3 pos, vec3 dim )
{
	rect3 r;
	r.pos = pos;
	r.dim = dim;
	return r;
}

aabb2 aabb2f( f32 minx, f32 miny, f32 maxx, f32 maxy )
{
	aabb2 r;
	r.min = vec2f( minx, miny );
	r.max = vec2f( maxx, maxy );
	return r;
}

aabb3 aabb3f( f32 minx, f32 miny, f32 minz, f32 maxx, f32 maxy, f32 maxz )
{
	aabb3 r;
	r.min = vec3f( minx, miny, minz );
	r.max = vec3f( maxx, maxy, maxz );
	return r;
}

aabb2 aabb2_rect2( rect2 a )
{
	aabb2 r;
	r.min = a.pos;
	vec2_add( &r.max, a.pos, a.dim );
	return r;
}

aabb3 aabb3_rect3( rect3 a )
{
	aabb3 r;
	r.min = a.pos;
	vec3_add( &r.max, a.pos, a.dim );
	return r;
}

rect2 rect2_aabb2( aabb2 a )
{
	rect2 r;
	r.pos = a.min;
	vec2_sub( &r.dim, a.max, a.min );
	return r;
}

rect3 rect3_aabb3( aabb3 a )
{
	rect3 r;
	r.pos = a.min;
	vec3_sub( &r.dim, a.max, a.min );
	return r;
}

int rect2_contains( rect2 a, f32 x, f32 y )
{
	f32 min_x  = min( a.pos.x, a.pos.x + a.dim.x );
	f32 max_x  = max( a.pos.x, a.pos.x + a.dim.x );
	f32 min_y  = min( a.pos.y, a.pos.y + a.dim.y );
	f32 max_y  = max( a.pos.y, a.pos.y + a.dim.y );
	int result = ( x >= min_x ) & ( x < max_x ) & ( y >= min_y ) & ( y < max_y );
	return result;
}

int rect2_contains_vec2( rect2 a, vec2 p )
{
	return rect2_contains( a, p.x, p.y );
}

int rect2_intersects( rect2 a, rect2 b )
{
	rect2 r = { 0 };
	return rect2_intersection_result( a, b, &r );
}

int rect2_intersection_result( rect2 a, rect2 b, rect2* intersection )
{
	f32 a_min_x = min( a.pos.x, a.pos.x + a.dim.x );
	f32 a_max_x = max( a.pos.x, a.pos.x + a.dim.x );
	f32 a_min_y = min( a.pos.y, a.pos.y + a.dim.y );
	f32 a_max_y = max( a.pos.y, a.pos.y + a.dim.y );

	f32 b_min_x = min( b.pos.x, b.pos.x + b.dim.x );
	f32 b_max_x = max( b.pos.x, b.pos.x + b.dim.x );
	f32 b_min_y = min( b.pos.y, b.pos.y + b.dim.y );
	f32 b_max_y = max( b.pos.y, b.pos.y + b.dim.y );

	f32 x0 = max( a_min_x, b_min_x );
	f32 y0 = max( a_min_y, b_min_y );
	f32 x1 = min( a_max_x, b_max_x );
	f32 y1 = min( a_max_y, b_max_y );

	if ( ( x0 < x1 ) && ( y0 < y1 ) )
	{
		rect2 r       = rect2f( vec2f( x0, y0 ), vec2f( x1 - x0, y1 - y0 ) );
		*intersection = r;
		return 1;
	}
	else
	{
		rect2 r       = { 0 };
		*intersection = r;
		return 0;
	}
}

int aabb2_contains( aabb2 a, f32 x, f32 y )
{
	return ( is_between_limit( x, a.min.x, a.max.x ) && is_between_limit( y, a.min.y, a.max.y ) );
}

int aabb3_contains( aabb3 a, f32 x, f32 y, f32 z )
{
	return ( is_between_limit( x, a.min.x, a.max.x ) && is_between_limit( y, a.min.y, a.max.y ) && is_between_limit( z, a.min.z, a.max.z ) );
}

aabb2 aabb2_cut_left( aabb2* a, f32 b )
{
	f32 minx = a->min.x;
	a->min.x = min( a->max.x, a->min.x + b );
	return aabb2f( minx, a->min.y, a->min.x, a->max.y );
}

aabb2 aabb2_cut_right( aabb2* a, f32 b )
{
	f32 maxx = a->max.x;
	a->max.x = max( a->min.x, a->max.x - b );
	return aabb2f( a->max.x, a->min.y, maxx, a->max.y );
}

aabb2 aabb2_cut_top( aabb2* a, f32 b )
{
	f32 miny = a->min.y;
	a->min.y = min( a->max.y, a->min.y + b );
	return aabb2f( a->min.x, miny, a->max.x, a->min.y );
}

aabb2 aabb2_cut_bottom( aabb2* a, f32 b )
{
	f32 maxy = a->max.y;
	a->max.y = max( a->min.y, a->max.y - b );
	return aabb2f( a->min.x, a->max.y, a->max.x, maxy );
}

aabb2 aabb2_get_left( const aabb2* a, f32 b )
{
	f32 minx  = a->min.x;
	f32 aminx = min( a->max.x, a->min.x + b );
	return aabb2f( minx, a->min.y, aminx, a->max.y );
}

aabb2 aabb2_get_right( const aabb2* a, f32 b )
{
	f32 maxx  = a->max.x;
	f32 amaxx = max( a->min.x, a->max.x - b );
	return aabb2f( amaxx, a->min.y, maxx, a->max.y );
}

aabb2 aabb2_get_top( const aabb2* a, f32 b )
{
	f32 miny  = a->min.y;
	f32 aminy = min( a->max.y, a->min.y + b );
	return aabb2f( a->min.x, miny, a->max.x, aminy );
}

aabb2 aabb2_get_bottom( const aabb2* a, f32 b )
{
	f32 maxy  = a->max.y;
	f32 amaxy = max( a->min.y, a->max.y - b );
	return aabb2f( a->min.x, amaxy, a->max.x, maxy );
}

aabb2 aabb2_add_left( const aabb2* a, f32 b )
{
	return aabb2f( a->min.x - b, a->min.y, a->min.x, a->max.y );
}

aabb2 aabb2_add_right( const aabb2* a, f32 b )
{
	return aabb2f( a->max.x, a->min.y, a->max.x + b, a->max.y );
}

aabb2 aabb2_add_top( const aabb2* a, f32 b )
{
	return aabb2f( a->min.x, a->min.y - b, a->max.x, a->min.y );
}

aabb2 aabb2_add_bottom( const aabb2* a, f32 b )
{
	return aabb2f( a->min.x, a->max.y, a->max.x, a->max.y + b );
}

aabb2 aabb2_contract( const aabb2* a, f32 b )
{
	aabb2 r  = *a;
	vec2  vb = vec2f( b, b );
	vec2_addeq( &r.min, vb );
	vec2_subeq( &r.max, vb );

	if ( vec2_mag2( r.min ) > vec2_mag2( r.max ) )
	{
		return aabb2f( 0, 0, 0, 0 );
	}
	return r;
}

aabb2 aabb2_expand( const aabb2* a, f32 b )
{
	return aabb2_contract( a, -b );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
