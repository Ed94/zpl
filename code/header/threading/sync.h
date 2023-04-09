// file: header/threading/sync.h

// NOTE: Thread Merge Operation
// Based on Sean Barrett's stb_sync

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct sync
{
	s32 target;     // Target Number of threads
	s32 current;    // Threads to hit
	s32 waiting;    // Threads waiting

	mutex     start;
	mutex     mutex;
	semaphore release;
} sync;

ZPL_DEF void sync_init( sync* s );
ZPL_DEF void sync_destroy( sync* s );
ZPL_DEF void sync_set_target( sync* s, s32 count );
ZPL_DEF void sync_release( sync* s );
ZPL_DEF s32  sync_reach( sync* s );
ZPL_DEF void sync_reach_and_wait( sync* s );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
