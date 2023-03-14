// a_file: header/core/time.h

/** @a_file time.c
@brief Time helper methods.
@defgroup time Time helpers

 Helper methods for retrieving the current time in many forms under different precisions. It also offers a simple to use timer library.

 @{
 */


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

//! Return CPU timestamp.
ZPL_DEF u64 rdtsc( void );

//! Return relative time (in seconds) since the application start.
ZPL_DEF f64 time_rel( void );

//! Return relative time since the application start.
ZPL_DEF u64 time_rel_ms( void );

//! Return time (in seconds) since 1601-01-01 UTC.
ZPL_DEF f64 time_utc( void );

//! Return time since 1601-01-01 UTC.
ZPL_DEF u64 time_utc_ms( void );

//! Return local system time since 1601-01-01
ZPL_DEF u64 time_tz_ms( void );

//! Return local system time in seconds since 1601-01-01
ZPL_DEF f64 time_tz( void );

//! Convert Win32 epoch (1601-01-01 UTC) to UNIX (1970-01-01 UTC)
ZPL_DEF_INLINE u64 time_win32_to_unix( u64 ms );

//! Convert UNIX (1970-01-01 UTC) to Win32 epoch (1601-01-01 UTC)
ZPL_DEF_INLINE u64 time_unix_to_win32( u64 ms );

//! Sleep for specified number of milliseconds.
ZPL_DEF void sleep_ms( u32 ms );

//! Sleep for specified number of seconds.
ZPL_DEF_INLINE void sleep( f32 s );

// Deprecated methods
ZPL_DEPRECATED_FOR( 10.9.0, time_rel )
ZPL_DEF_INLINE f64 time_now( void );

ZPL_DEPRECATED_FOR( 10.9.0, time_utc )
ZPL_DEF_INLINE f64 utc_time_now( void );


#ifndef ZPL__UNIX_TO_WIN32_EPOCH
#	define ZPL__UNIX_TO_WIN32_EPOCH 11644473600000ull
#endif

ZPL_IMPL_INLINE u64 time_win32_to_unix( u64 ms )
{
	return ms - ZPL__UNIX_TO_WIN32_EPOCH;
}

ZPL_IMPL_INLINE u64 time_unix_to_win32( u64 ms )
{
	return ms + ZPL__UNIX_TO_WIN32_EPOCH;
}

ZPL_IMPL_INLINE void sleep( f32 s )
{
	sleep_ms( ( u32 )( s * 1000 ) );
}

ZPL_IMPL_INLINE f64 time_now()
{
	return time_rel();
}

ZPL_IMPL_INLINE f64 utc_time_now()
{
	return time_utc();
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
