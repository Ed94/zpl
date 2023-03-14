// file: header/threading/thread.h

#ifdef ZPL_EDITOR
#include <zpl.h>
#else
struct thread;
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef sw ( *thread_proc )( struct thread* thread );

typedef struct thread
{
#if defined( ZPL_SYSTEM_WINDOWS )
	void* win32_handle;
#else
	pthread_t posix_handle;
#endif

	thread_proc proc;
	void*       user_data;
	sw          user_index;
	sw          return_value;

	semaphore semaphore;
	sw        stack_size;
	b32       is_running;
	b32       nowait;
} thread;

ZPL_DEF void thread_init( thread* t );
ZPL_DEF void thread_init_nowait( thread* t );
ZPL_DEF void thread_destroy( thread* t );
ZPL_DEF void thread_start( thread* t, thread_proc proc, void* data );
ZPL_DEF void thread_start_with_stack( thread* t, thread_proc proc, void* data, sw stack_size );
ZPL_DEF void thread_join( thread* t );
ZPL_DEF b32  thread_is_running( thread const * t );
ZPL_DEF u32  thread_current_id( void );
ZPL_DEF void thread_set_name( thread* t, char const * name );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
