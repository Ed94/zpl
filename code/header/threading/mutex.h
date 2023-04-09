// file: header/threading/mutex.h


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct mutex
{
#if defined( ZPL_SYSTEM_WINDOWS )
	u64 win32_critical_section[ sizeof( uw ) / 2 + 1 ];
#else
	pthread_mutex_t pthread_mutex;
#endif
} mutex;

ZPL_DEF void mutex_init( mutex* m );
ZPL_DEF void mutex_destroy( mutex* m );
ZPL_DEF void mutex_lock( mutex* m );
ZPL_DEF b32  mutex_try_lock( mutex* m );
ZPL_DEF void mutex_unlock( mutex* m );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
