// file: source/threading/thread.c

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

b32 thread_is_running( thread const * t )
{
	return t->is_running != 0;
}

void thread_init_nowait( thread* t )
{
	zero_item( t );

#if defined( ZPL_SYSTEM_WINDOWS )
	t->win32_handle = INVALID_HANDLE_VALUE;
#endif

	t->nowait = true;
}

void thread_init( thread* t )
{
	thread_init_nowait( t );

	t->nowait = false;
	semaphore_init( &t->semaphore );
}

void thread_destroy( thread* t )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	if ( t->win32_handle != INVALID_HANDLE_VALUE )
		thread_join( t );
#else
	if ( t->posix_handle )
		thread_join( t );
#endif
	if ( ! t->nowait )
		semaphore_destroy( &t->semaphore );
}

static void zpl__thread_run( thread* t )
{
	if ( ! t->nowait )
		semaphore_release( &t->semaphore );
	t->return_value = t->proc( t );
}

#if defined( ZPL_SYSTEM_WINDOWS )
static DWORD __stdcall zpl__thread_proc( void* arg )
{
	thread* t     = zpl_cast( thread* ) arg;
	t->is_running = true;
	zpl__thread_run( t );
	t->is_running = false;
	return 0;
}
#else
static void* zpl__thread_proc( void* arg )
{
	thread* t = zpl_cast( thread* ) arg;
	t->is_running = true;
	zpl__thread_run( t );
	t->is_running = false;
	return NULL;
}
#endif

void thread_start( thread* t, thread_proc proc, void* user_data )
{
	thread_start_with_stack( t, proc, user_data, 0 );
}

void thread_start_with_stack( thread* t, thread_proc proc, void* user_data, sw stack_size )
{
	ZPL_ASSERT( ! t->is_running );
	ZPL_ASSERT( proc != NULL );
	t->proc       = proc;
	t->user_data  = user_data;
	t->stack_size = stack_size;

#if defined( ZPL_SYSTEM_WINDOWS )
	t->win32_handle = CreateThread( NULL, stack_size, zpl__thread_proc, t, 0, NULL );
	ZPL_ASSERT_MSG( t->win32_handle != NULL, "CreateThread: GetLastError" );
#else
	{
		pthread_attr_t attr;
		pthread_attr_init( &attr );
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
		if ( stack_size != 0 )
			pthread_attr_setstacksize( &attr, stack_size );
		pthread_create( &t->posix_handle, &attr, zpl__thread_proc, t );
		pthread_attr_destroy( &attr );
	}
#endif
	if ( ! t->nowait )
		semaphore_wait( &t->semaphore );
}

void thread_join( thread* t )
{
#if defined( ZPL_SYSTEM_WINDOWS )
	WaitForSingleObject( t->win32_handle, INFINITE );
	CloseHandle( t->win32_handle );
	t->win32_handle = INVALID_HANDLE_VALUE;
#else
	pthread_join( t->posix_handle, NULL );
	t->posix_handle = 0;
#endif
}

u32 thread_current_id( void )
{
	u32 thread_id;
#if defined( ZPL_SYSTEM_WINDOWS )
#if defined( ZPL_ARCH_32_BIT ) && defined( ZPL_CPU_X86 )
	thread_id = ( zpl_cast( u32* ) __readfsdword( 24 ) )[ 9 ];
#elif defined( ZPL_ARCH_64_BIT ) && defined( ZPL_CPU_X86 )
	thread_id = ( zpl_cast( u32* ) __readgsqword( 48 ) )[ 18 ];
#else
	thread_id = GetCurrentThreadId();
#endif

#elif defined( ZPL_SYSTEM_OSX ) && defined( ZPL_ARCH_64_BIT )
	thread_id = pthread_mach_thread_np( pthread_self() );
#elif defined( ZPL_ARCH_32_BIT ) && defined( ZPL_CPU_X86 )
	__asm__( "mov %%gs:0x08,%0" : "=r"( thread_id ) );
#elif defined( ZPL_ARCH_64_BIT ) && defined( ZPL_CPU_X86 )
	__asm__( "mov %%fs:0x10,%0" : "=r"( thread_id ) );
#elif defined( __ARM_ARCH )
	thread_id = pthread_self();
#else
#error Unsupported architecture for thread_current_id()
#endif

	return thread_id;
}

void thread_set_name( thread* t, char const * name )
{
#if defined( ZPL_COMPILER_MSVC )
#pragma pack( push, 8 )
	typedef struct
	{
		DWORD        type;
		char const * name;
		DWORD        id;
		DWORD        flags;
	} zplprivThreadName;
#pragma pack( pop )

	zplprivThreadName tn;
	tn.type  = 0x1000;
	tn.name  = name;
	tn.id    = GetThreadId( zpl_cast( HANDLE ) t->win32_handle );
	tn.flags = 0;

	__try
	{
		RaiseException( 0x406d1388, 0, size_of( tn ) / 4, zpl_cast( ULONG_PTR* ) & tn );
	}
	__except ( 1 /*EXCEPTION_EXECUTE_HANDLER*/ )
	{
	}

#elif defined( ZPL_SYSTEM_WINDOWS ) && ! defined( ZPL_COMPILER_MSVC )
	unused( t );
	unused( name );
	// IMPORTANT TODO: Set thread name for GCC/Clang on windows
	return;
#elif defined( ZPL_SYSTEM_OSX )
	// TODO: Test if this works
	pthread_setname_np( name );
#else
	unused( t );
	unused( name );
	// TODO: Test if this works
	//        pthread_set_name_np(t->posix_handle, name);
#endif
}

ZPL_END_C_DECLS
ZPL_BEGIN_NAMESPACE
