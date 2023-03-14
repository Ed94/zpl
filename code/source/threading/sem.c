// file: source/threading/sem.c

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

void semaphore_release( semaphore* s )
{
	semaphore_post( s, 1 );
}

#if defined( ZPL_SYSTEM_WINDOWS )

void semaphore_init( semaphore* s )
{
	s->win32_handle = CreateSemaphoreA( NULL, 0, ZPL_I32_MAX, NULL );
}
void semaphore_destroy( semaphore* s )
{
	CloseHandle( s->win32_handle );
}
void semaphore_post( semaphore* s, s32 count )
{
	ReleaseSemaphore( s->win32_handle, count, NULL );
}
void semaphore_wait( semaphore* s )
{
	WaitForSingleObject( s->win32_handle, INFINITE );
}
s32 semaphore_trywait( semaphore* s )
{
	int r = WaitForSingleObject( s->win32_handle, 0 );
	return r;
}

#elif defined( ZPL_SYSTEM_OSX )

void semaphore_init( semaphore* s )
{
	semaphore_create( mach_task_self(), &s->osx_handle, SYNC_POLICY_FIFO, 0 );
}
void semaphore_destroy( semaphore* s )
{
	semaphore_destroy( mach_task_self(), s->osx_handle );
}
void semaphore_post( semaphore* s, s32 count )
{
	while ( count-- > 0 )
		semaphore_signal( s->osx_handle );
}
void semaphore_wait( semaphore* s )
{
	semaphore_wait( s->osx_handle );
}
s32 semaphore_trywait( semaphore* s )
{
	mach_timespec_t t;
	t.tv_sec = t.tv_nsec = 0;
	kern_return_t r      = semaphore_timedwait( s->osx_handle, t );
	return r;
}

#elif defined( ZPL_SYSTEM_UNIX )

void semaphore_init( semaphore* s )
{
	sem_init( &s->unix_handle, 0, 0 );
}
void semaphore_destroy( semaphore* s )
{
	sem_destroy( &s->unix_handle );
}
void semaphore_post( semaphore* s, s32 count )
{
	while ( count-- > 0 )
		sem_post( &s->unix_handle );
}
void semaphore_wait( semaphore* s )
{
	int i;
	do
	{
		i = sem_wait( &s->unix_handle );
	} while ( i == -1 && errno == EINTR );
}
s32 semaphore_trywait( semaphore* s )
{
	int r = sem_trywait( &s->unix_handle );
	return r;
}

#else
#error Semaphores for this OS are not implemented
#endif

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
