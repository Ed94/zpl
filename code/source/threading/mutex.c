// file: source/threading/mutex.c


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

void mutex_init( mutex* m )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	InitializeCriticalSection( ( CRITICAL_SECTION* )m->win32_critical_section );
#else
	pthread_mutex_init( &m->pthread_mutex, NULL );
#endif
}

void mutex_destroy( mutex* m )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	DeleteCriticalSection( ( CRITICAL_SECTION* )m->win32_critical_section );
#else
	pthread_mutex_destroy( &m->pthread_mutex );
#endif
}

void mutex_lock( mutex* m )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	EnterCriticalSection( ( CRITICAL_SECTION* )m->win32_critical_section );
#else
	pthread_mutex_lock( &m->pthread_mutex );
#endif
}

b32 mutex_try_lock( mutex* m )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	return TryEnterCriticalSection( ( CRITICAL_SECTION* )m->win32_critical_section );
#else
	return pthread_mutex_trylock( &m->pthread_mutex );
#endif
}

void mutex_unlock( mutex* m )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	LeaveCriticalSection( ( CRITICAL_SECTION* )m->win32_critical_section );
#else
	pthread_mutex_unlock( &m->pthread_mutex );
#endif
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
