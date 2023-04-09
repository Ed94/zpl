// file: header/threading/sem.h


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#if defined( ZPL_SYSTEM_MACOS )
#	include <mach/thread_act.h>
#elif defined( ZPL_SYSTEM_UNIX )
#	include <semaphore.h>
#endif

#if defined( ZPL_SYSTEM_WINDOWS )
typedef struct semaphore
{
	void* win32_handle;
} semaphore;
#elif defined( ZPL_SYSTEM_MACOS )
typedef struct semaphore
{
	semaphore_t osx_handle;
} semaphore;
#elif defined( ZPL_SYSTEM_UNIX )
typedef struct semaphore
{
	sem_t unix_handle;
} semaphore;
#else
#	error
#endif

ZPL_DEF void semaphore_init( semaphore* s );
ZPL_DEF void semaphore_destroy( semaphore* s );
ZPL_DEF void semaphore_post( semaphore* s, s32 count );
ZPL_DEF void semaphore_release( semaphore* s );    // NOTE: zpl_semaphore_post(s, 1)
ZPL_DEF void semaphore_wait( semaphore* s );
ZPL_DEF s32  semaphore_trywait( semaphore* s );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
