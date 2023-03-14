// file: header/math.h

/** @file math.c
@brief Math operations
@defgroup math Math operations

OpenGL gamedev friendly library for math.

@{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef union vec2
{
	struct
	{
		f32 x, y;
	};
	struct
	{
		f32 s, t;
	};
	f32 e[ 2 ];
} vec2;

typedef union vec3
{
	struct
	{
		f32 x, y, z;
	};
	struct
	{
		f32 r, g, b;
	};
	struct
	{
		f32 s, t, p;
	};

	vec2 xy;
	vec2 st;
	f32  e[ 3 ];
} vec3;

typedef union vec4
{
	struct
	{
		f32 x, y, z, w;
	};
	struct
	{
		f32 r, g, b, a;
	};
	struct
	{
		f32 s, t, p, q;
	};
	struct
	{
		vec2 xy, zw;
	};
	struct
	{
		vec2 st, pq;
	};
	vec3 xyz;
	vec3 rgb;
	f32  e[ 4 ];
} vec4;

typedef union mat2
{
	struct
	{
		vec2 x, y;
	};
	vec2 col[ 2 ];
	f32  e[ 4 ];
} mat2;

typedef union mat3
{
	struct
	{
		vec3 x, y, z;
	};
	vec3 col[ 3 ];
	f32  e[ 9 ];
} mat3;

typedef union mat4
{
	struct
	{
		vec4 x, y, z, w;
	};
	vec4 col[ 4 ];
	f32  e[ 16 ];
} mat4;

typedef union quat
{
	struct
	{
		f32 x, y, z, w;
	};
	vec4 xyzw;
	vec3 xyz;
	f32  e[ 4 ];
} quat;

typedef union plane
{
	struct
	{
		f32 a, b, c, d;
	};
	vec4 xyzw;
	vec3 n;
	f32  e[ 4 ];
} plane;

typedef struct frustum
{
	plane x1;
	plane x2;
	plane y1;
	plane y2;
	plane z1;
	plane z2;
} frustum;

typedef f32 float2[ 2 ];
typedef f32 float3[ 3 ];
typedef f32 float4[ 4 ];

typedef struct rect2
{
	vec2 pos, dim;
} rect2;
typedef struct rect3
{
	vec3 pos, dim;
} rect3;

typedef struct aabb2
{
	vec2 min, max;
} aabb2;
typedef struct aabb3
{
	vec3 min, max;
} aabb3;

typedef short half;

#ifndef ZPL_CONSTANTS
#define ZPL_CONSTANTS
#define ZPL_EPSILON    1.19209290e-7f
#define ZPL_ZERO       0.0f
#define ZPL_ONE        1.0f
#define ZPL_TWO_THIRDS 0.666666666666666666666666666666666666667f

#define ZPL_TAU          6.28318530717958647692528676655900576f
#define ZPL_PI           3.14159265358979323846264338327950288f
#define ZPL_ONE_OVER_TAU 0.636619772367581343075535053490057448f
#define ZPL_ONE_OVER_PI  0.159154943091895335768883763372514362f

#define ZPL_TAU_OVER_2 3.14159265358979323846264338327950288f
#define ZPL_TAU_OVER_4 1.570796326794896619231321691639751442f
#define ZPL_TAU_OVER_8 0.785398163397448309615660845819875721f

#define ZPL_E          2.71828182845904523536f
#define ZPL_SQRT_TWO   1.41421356237309504880168872420969808f
#define ZPL_SQRT_THREE 1.73205080756887729352744634150587236f
#define ZPL_SQRT_FIVE  2.23606797749978969640917366873127623f

#define ZPL_LOG_TWO 0.693147180559945309417232121458176568f
#define ZPL_LOG_TEN 2.30258509299404568401799145468436421f
#endif // ZPL_CONSTANTS

#ifndef square
#define square( x ) ( ( x ) * ( x ) )
#endif

#ifndef cube
#define cube( x ) ( ( x ) * ( x ) * ( x ) )
#endif

#ifndef sign
#define sign( x ) ( ( x ) >= 0.0f ? 1.0f : -1.0f )
#endif

#ifndef sign0
#define sign0( x ) ( ( x == 0.0f ) ? 0.0f : ( ( x ) >= 0.0f ? 1.0f : -1.0f ) )
#endif

ZPL_DEF f32 to_radians( f32 degrees );
ZPL_DEF f32 to_degrees( f32 radians );

/* NOTE: Because to interpolate angles */
ZPL_DEF f32 angle_diff( f32 radians_a, f32 radians_b );

ZPL_DEF f32 copy_sign( f32 x, f32 y );
ZPL_DEF f32 remainder( f32 x, f32 y );
ZPL_DEF f32 mod( f32 x, f32 y );
ZPL_DEF f64 copy_sign64( f64 x, f64 y );
ZPL_DEF f64 floor64( f64 x );
ZPL_DEF f64 ceil64( f64 x );
ZPL_DEF f64 round64( f64 x );
ZPL_DEF f64 remainder64( f64 x, f64 y );
ZPL_DEF f64 abs64( f64 x );
ZPL_DEF f64 sign64( f64 x );
ZPL_DEF f64 mod64( f64 x, f64 y );
ZPL_DEF f32 sqrt( f32 a );
ZPL_DEF f32 rsqrt( f32 a );
ZPL_DEF f32 quake_rsqrt( f32 a ); /* NOTE: It's probably better to use 1.0f/sqrt(a)
                                   * And for simd, there is usually isqrt functions too!
                                   */
ZPL_DEF f32 sin( f32 radians );
ZPL_DEF f32 cos( f32 radians );
ZPL_DEF f32 tan( f32 radians );
ZPL_DEF f32 arcsin( f32 a );
ZPL_DEF f32 arccos( f32 a );
ZPL_DEF f32 arctan( f32 a );
ZPL_DEF f32 arctan2( f32 y, f32 x );

ZPL_DEF f32 exp( f32 x );
ZPL_DEF f32 exp2( f32 x );
ZPL_DEF f32 log( f32 x );
ZPL_DEF f32 log2( f32 x );
ZPL_DEF f32 fast_exp( f32 x ); /* NOTE: Only valid from -1 <= x <= +1 */
ZPL_DEF f32 fast_exp2( f32 x ); /* NOTE: Only valid from -1 <= x <= +1 */
ZPL_DEF f32 pow( f32 x, f32 y ); /* x^y */

ZPL_DEF f32 round( f32 x );
ZPL_DEF f32 floor( f32 x );
ZPL_DEF f32 ceil( f32 x );

ZPL_DEF f32  half_to_float( half value );
ZPL_DEF half float_to_half( f32 value );

ZPL_DEF vec2 vec2f_zero( void );
ZPL_DEF vec2 vec2f( f32 x, f32 y );
ZPL_DEF vec2 vec2fv( f32 x[ 2 ] );

ZPL_DEF vec3 vec3f_zero( void );
ZPL_DEF vec3 vec3f( f32 x, f32 y, f32 z );
ZPL_DEF vec3 vec3fv( f32 x[ 3 ] );

ZPL_DEF vec4 vec4f_zero( void );
ZPL_DEF vec4 vec4f( f32 x, f32 y, f32 z, f32 w );
ZPL_DEF vec4 vec4fv( f32 x[ 4 ] );

ZPL_DEF f32  vec2_max( vec2 v );
ZPL_DEF f32  vec2_side( vec2 p, vec2 q, vec2 r );
ZPL_DEF void vec2_add( vec2* d, vec2 v0, vec2 v1 );
ZPL_DEF void vec2_sub( vec2* d, vec2 v0, vec2 v1 );
ZPL_DEF void vec2_mul( vec2* d, vec2 v, f32 s );
ZPL_DEF void vec2_div( vec2* d, vec2 v, f32 s );

ZPL_DEF f32  vec3_max( vec3 v );
ZPL_DEF void vec3_add( vec3* d, vec3 v0, vec3 v1 );
ZPL_DEF void vec3_sub( vec3* d, vec3 v0, vec3 v1 );
ZPL_DEF void vec3_mul( vec3* d, vec3 v, f32 s );
ZPL_DEF void vec3_div( vec3* d, vec3 v, f32 s );

ZPL_DEF void vec4_add( vec4* d, vec4 v0, vec4 v1 );
ZPL_DEF void vec4_sub( vec4* d, vec4 v0, vec4 v1 );
ZPL_DEF void vec4_mul( vec4* d, vec4 v, f32 s );
ZPL_DEF void vec4_div( vec4* d, vec4 v, f32 s );

ZPL_DEF void vec2_addeq( vec2* d, vec2 v );
ZPL_DEF void vec2_subeq( vec2* d, vec2 v );
ZPL_DEF void vec2_muleq( vec2* d, f32 s );
ZPL_DEF void vec2_diveq( vec2* d, f32 s );

ZPL_DEF void vec3_addeq( vec3* d, vec3 v );
ZPL_DEF void vec3_subeq( vec3* d, vec3 v );
ZPL_DEF void vec3_muleq( vec3* d, f32 s );
ZPL_DEF void vec3_diveq( vec3* d, f32 s );

ZPL_DEF void vec4_addeq( vec4* d, vec4 v );
ZPL_DEF void vec4_subeq( vec4* d, vec4 v );
ZPL_DEF void vec4_muleq( vec4* d, f32 s );
ZPL_DEF void vec4_diveq( vec4* d, f32 s );

ZPL_DEF f32 vec2_dot( vec2 v0, vec2 v1 );
ZPL_DEF f32 vec3_dot( vec3 v0, vec3 v1 );
ZPL_DEF f32 vec4_dot( vec4 v0, vec4 v1 );

ZPL_DEF void vec2_cross( f32* d, vec2 v0, vec2 v1 );
ZPL_DEF void vec3_cross( vec3* d, vec3 v0, vec3 v1 );

ZPL_DEF f32 vec2_mag2( vec2 v );
ZPL_DEF f32 vec3_mag2( vec3 v );
ZPL_DEF f32 vec4_mag2( vec4 v );

ZPL_DEF f32 vec2_mag( vec2 v );
ZPL_DEF f32 vec3_mag( vec3 v );
ZPL_DEF f32 vec4_mag( vec4 v );

ZPL_DEF void vec2_norm( vec2* d, vec2 v );
ZPL_DEF void vec3_norm( vec3* d, vec3 v );
ZPL_DEF void vec4_norm( vec4* d, vec4 v );

ZPL_DEF void vec2_norm0( vec2* d, vec2 v );
ZPL_DEF void vec3_norm0( vec3* d, vec3 v );
ZPL_DEF void vec4_norm0( vec4* d, vec4 v );

ZPL_DEF void vec2_reflect( vec2* d, vec2 i, vec2 n );
ZPL_DEF void vec3_reflect( vec3* d, vec3 i, vec3 n );
ZPL_DEF void vec2_refract( vec2* d, vec2 i, vec2 n, f32 eta );
ZPL_DEF void vec3_refract( vec3* d, vec3 i, vec3 n, f32 eta );

ZPL_DEF f32 vec2_aspect_ratio( vec2 v );

ZPL_DEF void mat2_identity( mat2* m );
ZPL_DEF void float22_identity( f32 m[ 2 ][ 2 ] );

ZPL_DEF void mat2_transpose( mat2* m );
ZPL_DEF void mat2_mul( mat2* out, mat2* m1, mat2* m2 );
ZPL_DEF void mat2_mul_vec2( vec2* out, mat2* m, vec2 in );
ZPL_DEF void mat2_inverse( mat2* out, mat2* in );
ZPL_DEF f32  mat2_determinate( mat2* m );

ZPL_DEF mat2*   mat2_v( vec2 m[ 2 ] );
ZPL_DEF mat2*   mat2_f( f32 m[ 2 ][ 2 ] );
ZPL_DEF float2* float22_m( mat2* m );
ZPL_DEF float2* float22_v( vec2 m[ 2 ] );
ZPL_DEF float2* float22_4( f32 m[ 4 ] );

ZPL_DEF void float22_transpose( f32 ( *vec )[ 2 ] );
ZPL_DEF void float22_mul( f32 ( *out )[ 2 ], f32 ( *mat1 )[ 2 ], f32 ( *mat2 )[ 2 ] );
ZPL_DEF void float22_mul_vec2( vec2* out, f32 m[ 2 ][ 2 ], vec2 in );

ZPL_DEF void mat3_identity( mat3* m );
ZPL_DEF void float33_identity( f32 m[ 3 ][ 3 ] );

ZPL_DEF void mat3_transpose( mat3* m );
ZPL_DEF void mat3_mul( mat3* out, mat3* m1, mat3* m2 );
ZPL_DEF void mat3_mul_vec3( vec3* out, mat3* m, vec3 in );
ZPL_DEF void mat3_inverse( mat3* out, mat3* in );
ZPL_DEF f32  mat3_determinate( mat3* m );

ZPL_DEF mat3* mat3_v( vec3 m[ 3 ] );
ZPL_DEF mat3* mat3_f( f32 m[ 3 ][ 3 ] );

ZPL_DEF float3* float33_m( mat3* m );
ZPL_DEF float3* float33_v( vec3 m[ 3 ] );
ZPL_DEF float3* float33_9( f32 m[ 9 ] );

ZPL_DEF void float33_transpose( f32 ( *vec )[ 3 ] );
ZPL_DEF void float33_mul( f32 ( *out )[ 3 ], f32 ( *mat1 )[ 3 ], f32 ( *mat2 )[ 3 ] );
ZPL_DEF void float33_mul_vec3( vec3* out, f32 m[ 3 ][ 3 ], vec3 in );

ZPL_DEF void mat4_identity( mat4* m );
ZPL_DEF void float44_identity( f32 m[ 4 ][ 4 ] );
ZPL_DEF void mat4_copy( mat4* out, mat4* m );

ZPL_DEF void mat4_transpose( mat4* m );
ZPL_DEF void mat4_mul( mat4* out, mat4* m1, mat4* m2 );
ZPL_DEF void mat4_mul_vec4( vec4* out, mat4* m, vec4 in );
ZPL_DEF void mat4_inverse( mat4* out, mat4* in );

ZPL_DEF mat4* mat4_v( vec4 m[ 4 ] );
ZPL_DEF mat4* mat4_f( f32 m[ 4 ][ 4 ] );

ZPL_DEF float4* float44_m( mat4* m );
ZPL_DEF float4* float44_v( vec4 m[ 4 ] );
ZPL_DEF float4* float44_16( f32 m[ 16 ] );

ZPL_DEF void float44_transpose( f32 ( *vec )[ 4 ] );
ZPL_DEF void float44_mul( f32 ( *out )[ 4 ], f32 ( *mat1 )[ 4 ], f32 ( *mat2 )[ 4 ] );
ZPL_DEF void float44_mul_vec4( vec4* out, f32 m[ 4 ][ 4 ], vec4 in );

ZPL_DEF void mat4_axis_angle( mat4* out, vec3 v, f32 angle_radians );
ZPL_DEF void mat4_to_translate( mat4* out, vec3 v );
ZPL_DEF void mat4_to_rotate( mat4* out, vec3 v, f32 angle_radians );
ZPL_DEF void mat4_to_scale( mat4* out, vec3 v );
ZPL_DEF void mat4_to_scalef( mat4* out, f32 s );
ZPL_DEF void mat4_translate( mat4* out, vec3 v );
ZPL_DEF void mat4_rotate( mat4* out, vec3 v, f32 angle_radians );
ZPL_DEF void mat4_scale( mat4* out, vec3 v );
ZPL_DEF void mat4_scalef( mat4* out, f32 s );
ZPL_DEF void mat4_ortho2d( mat4* out, f32 left, f32 right, f32 bottom, f32 top );
ZPL_DEF void mat4_ortho3d( mat4* out, f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far );
ZPL_DEF void mat4_perspective( mat4* out, f32 fovy, f32 aspect, f32 z_near, f32 z_far );
ZPL_DEF void mat4_infinite_perspective( mat4* out, f32 fovy, f32 aspect, f32 z_near );

ZPL_DEF void mat4_ortho2d_dx( mat4* out, f32 left, f32 right, f32 bottom, f32 top );
ZPL_DEF void mat4_ortho3d_dx( mat4* out, f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far );
ZPL_DEF void mat4_perspective_dx( mat4* out, f32 fovy, f32 aspect, f32 z_near, f32 z_far );
ZPL_DEF void mat4_infinite_perspective_dx( mat4* out, f32 fovy, f32 aspect, f32 z_near );

ZPL_DEF void mat4_look_at( mat4* out, vec3 eye, vec3 centre, vec3 up );

ZPL_DEF void mat4_look_at_lh( mat4* out, vec3 eye, vec3 centre, vec3 up );

ZPL_DEF quat quatf( f32 x, f32 y, f32 z, f32 w );
ZPL_DEF quat quatfv( f32 e[ 4 ] );
ZPL_DEF quat quat_axis_angle( vec3 axis, f32 angle_radians );
ZPL_DEF quat quat_euler_angles( f32 pitch, f32 yaw, f32 roll );
ZPL_DEF quat quat_identity( void );

ZPL_DEF void quat_add( quat* d, quat q0, quat q1 );
ZPL_DEF void quat_sub( quat* d, quat q0, quat q1 );
ZPL_DEF void quat_mul( quat* d, quat q0, quat q1 );
ZPL_DEF void quat_div( quat* d, quat q0, quat q1 );

ZPL_DEF void quat_mulf( quat* d, quat q, f32 s );
ZPL_DEF void quat_divf( quat* d, quat q, f32 s );

ZPL_DEF void quat_addeq( quat* d, quat q );
ZPL_DEF void quat_subeq( quat* d, quat q );
ZPL_DEF void quat_muleq( quat* d, quat q );
ZPL_DEF void quat_diveq( quat* d, quat q );

ZPL_DEF void quat_muleqf( quat* d, f32 s );
ZPL_DEF void quat_diveqf( quat* d, f32 s );

ZPL_DEF f32 quat_dot( quat q0, quat q1 );
ZPL_DEF f32 quat_mag( quat q );

ZPL_DEF void quat_norm( quat* d, quat q );
ZPL_DEF void quat_conj( quat* d, quat q );
ZPL_DEF void quat_inverse( quat* d, quat q );

ZPL_DEF void quat_axis( vec3* axis, quat q );
ZPL_DEF f32  quat_angle( quat q );

ZPL_DEF f32 quat_pitch( quat q );
ZPL_DEF f32 quat_yaw( quat q );
ZPL_DEF f32 quat_roll( quat q );

/* NOTE: Rotate v by q */
ZPL_DEF void quat_rotate_vec3( vec3* d, quat q, vec3 v );
ZPL_DEF void mat4_from_quat( mat4* out, quat q );
ZPL_DEF void quat_from_mat4( quat* out, mat4* m );

/* Plane math. */
ZPL_DEF f32 plane_distance( plane* p, vec3 v );

/* Frustum culling. */
ZPL_DEF void frustum_create( frustum* out, mat4* camera, mat4* proj );
ZPL_DEF b8   frustum_sphere_inside( frustum* frustum, vec3 center, f32 radius );
ZPL_DEF b8   frustum_point_inside( frustum* frustum, vec3 point );
ZPL_DEF b8   frustum_box_inside( frustum* frustum, aabb3 box );

/* Interpolations */
ZPL_DEF f32 lerp( f32 a, f32 b, f32 t );
ZPL_DEF f32 unlerp( f32 t, f32 a, f32 b );
ZPL_DEF f32 smooth_step( f32 a, f32 b, f32 t );
ZPL_DEF f32 smoother_step( f32 a, f32 b, f32 t );

ZPL_DEF void vec2_lerp( vec2* d, vec2 a, vec2 b, f32 t );
ZPL_DEF void vec3_lerp( vec3* d, vec3 a, vec3 b, f32 t );
ZPL_DEF void vec4_lerp( vec4* d, vec4 a, vec4 b, f32 t );

ZPL_DEF void vec2_cslerp( vec2* d, vec2 a, vec2 v0, vec2 b, vec2 v1, f32 t );
ZPL_DEF void vec3_cslerp( vec3* d, vec3 a, vec3 v0, vec3 b, vec3 v1, f32 t );
ZPL_DEF void vec2_dcslerp( vec2* d, vec2 a, vec2 v0, vec2 b, vec2 v1, f32 t );
ZPL_DEF void vec3_dcslerp( vec3* d, vec3 a, vec3 v0, vec3 b, vec3 v1, f32 t );

ZPL_DEF void quat_lerp( quat* d, quat a, quat b, f32 t );
ZPL_DEF void quat_nlerp( quat* d, quat a, quat b, f32 t );
ZPL_DEF void quat_slerp( quat* d, quat a, quat b, f32 t );
ZPL_DEF void quat_nquad( quat* d, quat p, quat a, quat b, quat q, f32 t );
ZPL_DEF void quat_squad( quat* d, quat p, quat a, quat b, quat q, f32 t );
ZPL_DEF void quat_slerp_approx( quat* d, quat a, quat b, f32 t );
ZPL_DEF void quat_squad_approx( quat* d, quat p, quat a, quat b, quat q, f32 t );

/* rects */
ZPL_DEF rect2 rect2f( vec2 pos, vec2 dim );
ZPL_DEF rect3 rect3f( vec3 pos, vec3 dim );

ZPL_DEF aabb2 aabb2f( f32 minx, f32 miny, f32 maxx, f32 maxy );
ZPL_DEF aabb3 aabb3f( f32 minx, f32 miny, f32 minz, f32 maxx, f32 maxy, f32 maxz );

ZPL_DEF aabb2 aabb2_rect2( rect2 a );
ZPL_DEF aabb3 aabb3_rect3( rect3 a );
ZPL_DEF rect2 rect2_aabb2( aabb2 a );
ZPL_DEF rect3 rect3_aabb3( aabb3 a );

ZPL_DEF int rect2_contains( rect2 a, f32 x, f32 y );
ZPL_DEF int rect2_contains_vec2( rect2 a, vec2 p );
ZPL_DEF int rect2_intersects( rect2 a, rect2 b );
ZPL_DEF int rect2_intersection_result( rect2 a, rect2 b, rect2* intersection );
ZPL_DEF int aabb2_contains( aabb2 a, f32 x, f32 y );
ZPL_DEF int aabb3_contains( aabb3 a, f32 x, f32 y, f32 z );

/* rectangle partitioning: based on https://halt.software/dead-simple-layouts/ */
ZPL_DEF aabb2 aabb2_cut_left( aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_cut_right( aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_cut_top( aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_cut_bottom( aabb2* a, f32 b );

ZPL_DEF aabb2 aabb2_get_left( const aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_get_right( const aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_get_top( const aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_get_bottom( const aabb2* a, f32 b );

ZPL_DEF aabb2 aabb2_add_left( const aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_add_right( const aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_add_top( const aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_add_bottom( const aabb2* a, f32 b );

ZPL_DEF aabb2 aabb2_contract( const aabb2* a, f32 b );
ZPL_DEF aabb2 aabb2_expand( const aabb2* a, f32 b );

//! @}
ZPL_END_C_DECLS
#if defined( __cplusplus )
ZPL_INLINE bool operator==( vec2 a, vec2 b )
{
	return ( a.x == b.x ) && ( a.y == b.y );
}
ZPL_INLINE bool operator!=( vec2 a, vec2 b )
{
	return ! operator==( a, b );
}

ZPL_INLINE vec2 operator+( vec2 a )
{
	return a;
}
ZPL_INLINE vec2 operator-( vec2 a )
{
	vec2 r = { -a.x, -a.y };
	return r;
}

ZPL_INLINE vec2 operator+( vec2 a, vec2 b )
{
	vec2 r;
	vec2_add( &r, a, b );
	return r;
}
ZPL_INLINE vec2 operator-( vec2 a, vec2 b )
{
	vec2 r;
	vec2_sub( &r, a, b );
	return r;
}

ZPL_INLINE vec2 operator*( vec2 a, float scalar )
{
	vec2 r;
	vec2_mul( &r, a, scalar );
	return r;
}
ZPL_INLINE vec2 operator*( float scalar, vec2 a )
{
	return operator*( a, scalar );
}

ZPL_INLINE vec2 operator/( vec2 a, float scalar )
{
	return operator*( a, 1.0f / scalar );
}

/* Hadamard Product */
ZPL_INLINE vec2 operator*( vec2 a, vec2 b )
{
	vec2 r = { a.x * b.x, a.y * b.y };
	return r;
}
ZPL_INLINE vec2 operator/( vec2 a, vec2 b )
{
	vec2 r = { a.x / b.x, a.y / b.y };
	return r;
}

ZPL_INLINE vec2& operator+=( vec2& a, vec2 b )
{
	return ( a = a + b );
}
ZPL_INLINE vec2& operator-=( vec2& a, vec2 b )
{
	return ( a = a - b );
}
ZPL_INLINE vec2& operator*=( vec2& a, float scalar )
{
	return ( a = a * scalar );
}
ZPL_INLINE vec2& operator/=( vec2& a, float scalar )
{
	return ( a = a / scalar );
}

ZPL_INLINE bool operator==( vec3 a, vec3 b )
{
	return ( a.x == b.x ) && ( a.y == b.y ) && ( a.z == b.z );
}
ZPL_INLINE bool operator!=( vec3 a, vec3 b )
{
	return ! operator==( a, b );
}

ZPL_INLINE vec3 operator+( vec3 a )
{
	return a;
}
ZPL_INLINE vec3 operator-( vec3 a )
{
	vec3 r = { -a.x, -a.y, -a.z };
	return r;
}

ZPL_INLINE vec3 operator+( vec3 a, vec3 b )
{
	vec3 r;
	vec3_add( &r, a, b );
	return r;
}
ZPL_INLINE vec3 operator-( vec3 a, vec3 b )
{
	vec3 r;
	vec3_sub( &r, a, b );
	return r;
}

ZPL_INLINE vec3 operator*( vec3 a, float scalar )
{
	vec3 r;
	vec3_mul( &r, a, scalar );
	return r;
}
ZPL_INLINE vec3 operator*( float scalar, vec3 a )
{
	return operator*( a, scalar );
}

ZPL_INLINE vec3 operator/( vec3 a, float scalar )
{
	return operator*( a, 1.0f / scalar );
}

/* Hadamard Product */
ZPL_INLINE vec3 operator*( vec3 a, vec3 b )
{
	vec3 r = { a.x * b.x, a.y * b.y, a.z * b.z };
	return r;
}
ZPL_INLINE vec3 operator/( vec3 a, vec3 b )
{
	vec3 r = { a.x / b.x, a.y / b.y, a.z / b.z };
	return r;
}

ZPL_INLINE vec3& operator+=( vec3& a, vec3 b )
{
	return ( a = a + b );
}
ZPL_INLINE vec3& operator-=( vec3& a, vec3 b )
{
	return ( a = a - b );
}
ZPL_INLINE vec3& operator*=( vec3& a, float scalar )
{
	return ( a = a * scalar );
}
ZPL_INLINE vec3& operator/=( vec3& a, float scalar )
{
	return ( a = a / scalar );
}

ZPL_INLINE bool operator==( vec4 a, vec4 b )
{
	return ( a.x == b.x ) && ( a.y == b.y ) && ( a.z == b.z ) && ( a.w == b.w );
}
ZPL_INLINE bool operator!=( vec4 a, vec4 b )
{
	return ! operator==( a, b );
}

ZPL_INLINE vec4 operator+( vec4 a )
{
	return a;
}
ZPL_INLINE vec4 operator-( vec4 a )
{
	vec4 r = { -a.x, -a.y, -a.z, -a.w };
	return r;
}

ZPL_INLINE vec4 operator+( vec4 a, vec4 b )
{
	vec4 r;
	vec4_add( &r, a, b );
	return r;
}
ZPL_INLINE vec4 operator-( vec4 a, vec4 b )
{
	vec4 r;
	vec4_sub( &r, a, b );
	return r;
}

ZPL_INLINE vec4 operator*( vec4 a, float scalar )
{
	vec4 r;
	vec4_mul( &r, a, scalar );
	return r;
}
ZPL_INLINE vec4 operator*( float scalar, vec4 a )
{
	return operator*( a, scalar );
}

ZPL_INLINE vec4 operator/( vec4 a, float scalar )
{
	return operator*( a, 1.0f / scalar );
}

/* Hadamard Product */
ZPL_INLINE vec4 operator*( vec4 a, vec4 b )
{
	vec4 r = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
	return r;
}
ZPL_INLINE vec4 operator/( vec4 a, vec4 b )
{
	vec4 r = { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
	return r;
}

ZPL_INLINE vec4& operator+=( vec4& a, vec4 b )
{
	return ( a = a + b );
}
ZPL_INLINE vec4& operator-=( vec4& a, vec4 b )
{
	return ( a = a - b );
}
ZPL_INLINE vec4& operator*=( vec4& a, float scalar )
{
	return ( a = a * scalar );
}
ZPL_INLINE vec4& operator/=( vec4& a, float scalar )
{
	return ( a = a / scalar );
}

ZPL_INLINE mat2 operator+( mat2 const & a, mat2 const & b )
{
	int  i, j;
	mat2 r = { 0 };
	for ( j = 0; j < 2; j++ )
	{
		for ( i = 0; i < 2; i++ )
			r.e[ 2 * j + i ] = a.e[ 2 * j + i ] + b.e[ 2 * j + i ];
	}
	return r;
}

ZPL_INLINE mat2 operator-( mat2 const & a, mat2 const & b )
{
	int  i, j;
	mat2 r = { 0 };
	for ( j = 0; j < 2; j++ )
	{
		for ( i = 0; i < 2; i++ )
			r.e[ 2 * j + i ] = a.e[ 2 * j + i ] - b.e[ 2 * j + i ];
	}
	return r;
}

ZPL_INLINE mat2 operator*( mat2 const & a, mat2 const & b )
{
	mat2 r;
	mat2_mul( &r, (mat2*)&a, (mat2*)&b );
	return r;
}
ZPL_INLINE vec2 operator*( mat2 const & a, vec2 v )
{
	vec2 r;
	mat2_mul_vec2( &r, (mat2*)&a, v );
	return r;
}
ZPL_INLINE mat2 operator*( mat2 const & a, float scalar )
{
	mat2 r = { 0 };
	int  i;
	for ( i = 0; i < 2 * 2; i++ )
		r.e[ i ] = a.e[ i ] * scalar;
	return r;
}
ZPL_INLINE mat2 operator*( float scalar, mat2 const & a )
{
	return operator*( a, scalar );
}
ZPL_INLINE mat2 operator/( mat2 const & a, float scalar )
{
	return operator*( a, 1.0f / scalar );
}

ZPL_INLINE mat2& operator+=( mat2& a, mat2 const & b )
{
	return ( a = a + b );
}
ZPL_INLINE mat2& operator-=( mat2& a, mat2 const & b )
{
	return ( a = a - b );
}
ZPL_INLINE mat2& operator*=( mat2& a, mat2 const & b )
{
	return ( a = a * b );
}

ZPL_INLINE mat3 operator+( mat3 const & a, mat3 const & b )
{
	int  i, j;
	mat3 r = { 0 };
	for ( j = 0; j < 3; j++ )
	{
		for ( i = 0; i < 3; i++ )
			r.e[ 3 * j + i ] = a.e[ 3 * j + i ] + b.e[ 3 * j + i ];
	}
	return r;
}

ZPL_INLINE mat3 operator-( mat3 const & a, mat3 const & b )
{
	int  i, j;
	mat3 r = { 0 };
	for ( j = 0; j < 3; j++ )
	{
		for ( i = 0; i < 3; i++ )
			r.e[ 3 * j + i ] = a.e[ 3 * j + i ] - b.e[ 3 * j + i ];
	}
	return r;
}

ZPL_INLINE mat3 operator*( mat3 const & a, mat3 const & b )
{
	mat3 r;
	mat3_mul( &r, (mat3*)&a, (mat3*)&b );
	return r;
}
ZPL_INLINE vec3 operator*( mat3 const & a, vec3 v )
{
	vec3 r;
	mat3_mul_vec3( &r, (mat3*)&a, v );
	return r;
}
ZPL_INLINE mat3 operator*( mat3 const & a, float scalar )
{
	mat3 r = { 0 };
	int  i;
	for ( i = 0; i < 3 * 3; i++ )
		r.e[ i ] = a.e[ i ] * scalar;
	return r;
}
ZPL_INLINE mat3 operator*( float scalar, mat3 const & a )
{
	return operator*( a, scalar );
}
ZPL_INLINE mat3 operator/( mat3 const & a, float scalar )
{
	return operator*( a, 1.0f / scalar );
}

ZPL_INLINE mat3& operator+=( mat3& a, mat3 const & b )
{
	return ( a = a + b );
}
ZPL_INLINE mat3& operator-=( mat3& a, mat3 const & b )
{
	return ( a = a - b );
}
ZPL_INLINE mat3& operator*=( mat3& a, mat3 const & b )
{
	return ( a = a * b );
}

ZPL_INLINE mat4 operator+( mat4 const & a, mat4 const & b )
{
	int  i, j;
	mat4 r = { 0 };
	for ( j = 0; j < 4; j++ )
	{
		for ( i = 0; i < 4; i++ )
			r.e[ 4 * j + i ] = a.e[ 4 * j + i ] + b.e[ 4 * j + i ];
	}
	return r;
}

ZPL_INLINE mat4 operator-( mat4 const & a, mat4 const & b )
{
	int  i, j;
	mat4 r = { 0 };
	for ( j = 0; j < 4; j++ )
	{
		for ( i = 0; i < 4; i++ )
			r.e[ 4 * j + i ] = a.e[ 4 * j + i ] - b.e[ 4 * j + i ];
	}
	return r;
}

ZPL_INLINE mat4 operator*( mat4 const & a, mat4 const & b )
{
	mat4 r;
	mat4_mul( &r, (mat4*)&a, (mat4*)&b );
	return r;
}
ZPL_INLINE vec4 operator*( mat4 const & a, vec4 v )
{
	vec4 r;
	mat4_mul_vec4( &r, (mat4*)&a, v );
	return r;
}
ZPL_INLINE mat4 operator*( mat4 const & a, float scalar )
{
	mat4 r = { 0 };
	int  i;
	for ( i = 0; i < 4 * 4; i++ )
		r.e[ i ] = a.e[ i ] * scalar;
	return r;
}
ZPL_INLINE mat4 operator*( float scalar, mat4 const & a )
{
	return operator*( a, scalar );
}
ZPL_INLINE mat4 operator/( mat4 const & a, float scalar )
{
	return operator*( a, 1.0f / scalar );
}

ZPL_INLINE mat4& operator+=( mat4& a, mat4 const & b )
{
	return ( a = a + b );
}
ZPL_INLINE mat4& operator-=( mat4& a, mat4 const & b )
{
	return ( a = a - b );
}
ZPL_INLINE mat4& operator*=( mat4& a, mat4 const & b )
{
	return ( a = a * b );
}

ZPL_INLINE bool operator==( quat a, quat b )
{
	return a.xyzw == b.xyzw;
}
ZPL_INLINE bool operator!=( quat a, quat b )
{
	return ! operator==( a, b );
}

ZPL_INLINE quat operator+( quat q )
{
	return q;
}
ZPL_INLINE quat operator-( quat q )
{
	return quatf( -q.x, -q.y, -q.z, -q.w );
}

ZPL_INLINE quat operator+( quat a, quat b )
{
	quat r;
	quat_add( &r, a, b );
	return r;
}
ZPL_INLINE quat operator-( quat a, quat b )
{
	quat r;
	quat_sub( &r, a, b );
	return r;
}

ZPL_INLINE quat operator*( quat a, quat b )
{
	quat r;
	quat_mul( &r, a, b );
	return r;
}
ZPL_INLINE quat operator*( quat q, float s )
{
	quat r;
	quat_mulf( &r, q, s );
	return r;
}
ZPL_INLINE quat operator*( float s, quat q )
{
	return operator*( q, s );
}
ZPL_INLINE quat operator/( quat q, float s )
{
	quat r;
	quat_divf( &r, q, s );
	return r;
}

ZPL_INLINE quat& operator+=( quat& a, quat b )
{
	quat_addeq( &a, b );
	return a;
}
ZPL_INLINE quat& operator-=( quat& a, quat b )
{
	quat_subeq( &a, b );
	return a;
}
ZPL_INLINE quat& operator*=( quat& a, quat b )
{
	quat_muleq( &a, b );
	return a;
}
ZPL_INLINE quat& operator/=( quat& a, quat b )
{
	quat_diveq( &a, b );
	return a;
}

ZPL_INLINE quat& operator*=( quat& a, float b )
{
	quat_muleqf( &a, b );
	return a;
}
ZPL_INLINE quat& operator/=( quat& a, float b )
{
	quat_diveqf( &a, b );
	return a;
}

/* Rotate v by a */
ZPL_INLINE vec3 operator*( quat q, vec3 v )
{
	vec3 r;
	quat_rotate_vec3( &r, q, v );
	return r;
}
#endif

ZPL_END_NAMESPACE
