// file: header/threading/atomic.h

// Atomics

// TODO: Be specific with memory order?
// e.g. relaxed, acquire, release, acquire_release

#if ! defined( __STDC_NO_ATOMICS__ ) && ! defined( __cplusplus ) && ! defined( ZPL_COMPILER_MSVC ) && ! defined( ZPL_COMPILER_TINYC )
#define atomic( X ) volatile _Atomic( X )
#else
// TODO: Fix once C++ guys bother to add C atomics to std.
// # include <atomic>
#define atomic( X ) volatile X /*std::atomic<X>*/
#endif

#if defined( __STDC_NO_ATOMICS__ ) || defined( __cplusplus ) || defined( ZPL_COMPILER_MSVC )
#define atomicarg( X ) volatile X
#else
#define atomicarg( X ) X
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#if defined( ZPL_COMPILER_MSVC )
typedef struct atomic32
{
	atomic( s32 ) value;
} atomic32;
typedef struct atomic64
{
	atomic( s64 ) value;
} atomic64;
typedef struct atomic_ptr
{
	atomic( void* ) value;
} atomic_ptr;
#else
#if defined( ZPL_ARCH_32_BIT )
#define ZPL_ATOMIC_PTR_ALIGNMENT 4
#elif defined( ZPL_ARCH_64_BIT )
#define ZPL_ATOMIC_PTR_ALIGNMENT 8
#else
#error Unknown architecture
#endif

typedef struct atomic32
{
	atomic( s32 ) value;
} __attribute__( ( aligned( 4 ) ) ) atomic32;
typedef struct atomic64
{
	atomic( s64 ) value;
} __attribute__( ( aligned( 8 ) ) ) atomic64;
typedef struct atomic_ptr
{
	atomic( void* ) value;
} __attribute__( ( aligned( ZPL_ATOMIC_PTR_ALIGNMENT ) ) ) atomic_ptr;
#endif

ZPL_DEF s32  atomic32_load( atomic32 const * a );
ZPL_DEF void atomic32_store( atomic32* a, atomicarg( s32 ) value );
ZPL_DEF s32  atomic32_compare_exchange( atomic32* a, atomicarg( s32 ) expected, atomicarg( s32 ) desired );
ZPL_DEF s32  atomic32_exchange( atomic32* a, atomicarg( s32 ) desired );
ZPL_DEF s32  atomic32_fetch_add( atomic32* a, atomicarg( s32 ) operand );
ZPL_DEF s32  atomic32_fetch_and( atomic32* a, atomicarg( s32 ) operand );
ZPL_DEF s32  atomic32_fetch_or( atomic32* a, atomicarg( s32 ) operand );
ZPL_DEF b32  atomic32_spin_lock( atomic32* a, sw time_out ); // NOTE: time_out = -1 as default
ZPL_DEF void atomic32_spin_unlock( atomic32* a );
ZPL_DEF b32  atomic32_try_acquire_lock( atomic32* a );

ZPL_DEF s64  atomic64_load( atomic64 const * a );
ZPL_DEF void atomic64_store( atomic64* a, atomicarg( s64 ) value );
ZPL_DEF s64  atomic64_compare_exchange( atomic64* a, atomicarg( s64 ) expected, atomicarg( s64 ) desired );
ZPL_DEF s64  atomic64_exchange( atomic64* a, atomicarg( s64 ) desired );
ZPL_DEF s64  atomic64_fetch_add( atomic64* a, atomicarg( s64 ) operand );
ZPL_DEF s64  atomic64_fetch_and( atomic64* a, atomicarg( s64 ) operand );
ZPL_DEF s64  atomic64_fetch_or( atomic64* a, atomicarg( s64 ) operand );
ZPL_DEF b32  atomic64_spin_lock( atomic64* a, sw time_out ); // NOTE: time_out = -1 as default
ZPL_DEF void atomic64_spin_unlock( atomic64* a );
ZPL_DEF b32  atomic64_try_acquire_lock( atomic64* a );

ZPL_DEF void* atomic_ptr_load( atomic_ptr const * a );
ZPL_DEF void  atomic_ptr_store( atomic_ptr* a, atomicarg( void* ) value );
ZPL_DEF void* atomic_ptr_compare_exchange( atomic_ptr* a, atomicarg( void* ) expected, atomicarg( void* ) desired );
ZPL_DEF void* atomic_ptr_exchange( atomic_ptr* a, atomicarg( void* ) desired );
ZPL_DEF void* atomic_ptr_fetch_add( atomic_ptr* a, atomicarg( void* ) operand );
ZPL_DEF void* atomic_ptr_fetch_and( atomic_ptr* a, atomicarg( void* ) operand );
ZPL_DEF void* atomic_ptr_fetch_or( atomic_ptr* a, atomicarg( void* ) operand );
ZPL_DEF b32   atomic_ptr_spin_lock( atomic_ptr* a, sw time_out ); // NOTE: time_out = -1 as default
ZPL_DEF void  atomic_ptr_spin_unlock( atomic_ptr* a );
ZPL_DEF b32   atomic_ptr_try_acquire_lock( atomic_ptr* a );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
