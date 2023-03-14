// a_file: header/threading/affinity.h


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#if defined( ZPL_SYSTEM_WINDOWS ) || defined( ZPL_SYSTEM_CYGWIN )

typedef struct affinity
{
	b32 is_accurate;
	sw  core_count;
	sw  thread_count;

#	define ZPL_WIN32_MAX_THREADS ( 8 * size_of( ZPL_NS uw ) )
	uw core_masks[ ZPL_WIN32_MAX_THREADS ];
} affinity;

#elif defined( ZPL_SYSTEM_OSX )

typedef struct affinity
{
	b32 is_accurate;
	sw  core_count;
	sw  thread_count;
	sw  threads_per_core;
} affinity;

#elif defined( ZPL_SYSTEM_LINUX ) || defined( ZPL_SYSTEM_FREEBSD ) || defined( ZPL_SYSTEM_EMSCRIPTEN ) || defined( ZPL_SYSTEM_OPENBSD )

typedef struct affinity
{
	b32 is_accurate;
	sw  core_count;
	sw  thread_count;
	sw  threads_per_core;
} affinity;

#else
#	error TODO: Unknown system
#endif

ZPL_DEF void affinity_init( affinity* a );
ZPL_DEF void affinity_destroy( affinity* a );
ZPL_DEF b32  affinity_set( affinity* a, sw core, sw thread );
ZPL_DEF sw   affinity_thread_count_for_core( affinity* a, sw core );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
