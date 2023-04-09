// file: source/threading/atomic.c


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

////////////////////////////////////////////////////////////////
//
// Concurrency
//
//
// IMPORTANT TODO: Use compiler intrinsics for the atomics

#if defined( ZPL_COMPILER_MSVC ) && ! defined( ZPL_COMPILER_CLANG )
s32 atomic32_load( atomic32 const* a )
{
	return a->value;
}

void atomic32_store( atomic32* a, atomicarg( s32 ) value )
{
	a->value = value;
}

s32 atomic32_compare_exchange( atomic32* a, atomicarg( s32 ) expected, atomicarg( s32 ) desired )
{
	return _InterlockedCompareExchange( zpl_cast( long* ) a, desired, expected );
}

s32 atomic32_exchange( atomic32* a, atomicarg( s32 ) desired )
{
	return _InterlockedExchange( zpl_cast( long* ) a, desired );
}

s32 atomic32_fetch_add( atomic32* a, atomicarg( s32 ) operand )
{
	return _InterlockedExchangeAdd( zpl_cast( long* ) a, operand );
}

s32 atomic32_fetch_and( atomic32* a, atomicarg( s32 ) operand )
{
	return _InterlockedAnd( zpl_cast( long* ) a, operand );
}

s32 atomic32_fetch_or( atomic32* a, atomicarg( s32 ) operand )
{
	return _InterlockedOr( zpl_cast( long* ) a, operand );
}

s64 atomic64_load( atomic64 const* a )
{
#	if defined( ZPL_ARCH_64_BIT )
	return a->value;
#	elif ZPL_CPU_X86
	// NOTE: The most compatible way to get an atomic 64-bit load on x86 is with cmpxchg8b
	atomicarg( s64 ) result;
	__asm {
                mov esi, a;
                mov ebx, eax;
                mov ecx, edx;
                lock cmpxchg8b [esi];
                mov dword ptr result, eax;
                mov dword ptr result[4], edx;
	}
	return result;
#	else
#		error TODO: atomics for this CPU
#	endif
}

void atomic64_store( atomic64* a, atomicarg( s64 ) value )
{
#	if defined( ZPL_ARCH_64_BIT )
	a->value = value;
#	elif ZPL_CPU_X86
	// NOTE: The most compatible way to get an atomic 64-bit store on x86 is with cmpxchg8b
	__asm {
                mov esi, a;
                mov ebx, dword ptr value;
                mov ecx, dword ptr value[4];
                retry:
                cmpxchg8b [esi];
                jne retry;
	}
#	else
#		error TODO: atomics for this CPU
#	endif
}

s64 atomic64_compare_exchange( atomic64* a, atomicarg( s64 ) expected, atomicarg( s64 ) desired )
{
	return _InterlockedCompareExchange64( zpl_cast( atomicarg( s64 )* ) a, desired, expected );
}

s64 atomic64_exchange( atomic64* a, atomicarg( s64 ) desired )
{
#	if defined( ZPL_ARCH_64_BIT )
	return _InterlockedExchange64( zpl_cast( atomicarg( s64 )* ) a, desired );
#	elif ZPL_CPU_X86
	atomicarg( s64 ) expected = a->value;
	for ( ;; )
	{
		atomicarg( s64 ) original = _InterlockedCompareExchange64( zpl_cast( atomicarg( s64 )* ) a, desired, expected );
		if ( original == expected )
			return original;
		expected = original;
	}
#	else
#		error TODO: atomics for this CPU
#	endif
}

s64 atomic64_fetch_add( atomic64* a, atomicarg( s64 ) operand )
{
#	if defined( ZPL_ARCH_64_BIT )
	return _InterlockedExchangeAdd64( zpl_cast( atomicarg( s64 )* ) a, operand );
#	elif ZPL_CPU_X86
	atomicarg( s64 ) expected = a->value;
	for ( ;; )
	{
		atomicarg( s64 ) original = _InterlockedCompareExchange64( zpl_cast( atomicarg( s64 )* ) a, expected + operand, expected );
		if ( original == expected )
			return original;
		expected = original;
	}
#	else
#		error TODO: atomics for this CPU
#	endif
}

s64 atomic64_fetch_and( atomic64* a, atomicarg( s64 ) operand )
{
#	if defined( ZPL_ARCH_64_BIT )
	return _InterlockedAnd64( zpl_cast( atomicarg( s64 )* ) a, operand );
#	elif ZPL_CPU_X86
	atomicarg( s64 ) expected = a->value;
	for ( ;; )
	{
		atomicarg( s64 ) original = _InterlockedCompareExchange64( zpl_cast( atomicarg( s64 )* ) a, expected & operand, expected );
		if ( original == expected )
			return original;
		expected = original;
	}
#	else
#		error TODO: atomics for this CPU
#	endif
}

s64 atomic64_fetch_or( atomic64* a, atomicarg( s64 ) operand )
{
#	if defined( ZPL_ARCH_64_BIT )
	return _InterlockedOr64( zpl_cast( atomicarg( s64 )* ) a, operand );
#	elif ZPL_CPU_X86
	atomicarg( s64 ) expected = a->value;
	for ( ;; )
	{
		atomicarg( s64 ) original = _InterlockedCompareExchange64( zpl_cast( atomicarg( s64 )* ) a, expected | operand, expected );
		if ( original == expected )
			return original;
		expected = original;
	}
#	else
#		error TODO: atomics for this CPU
#	endif
}

#elif defined( ZPL_CPU_X86 )

s32 atomic32_load( atomic32 const* a )
{
	return a->value;
}

void atomic32_store( atomic32* a, atomicarg( s32 ) value )
{
	a->value = value;
}

s32 atomic32_compare_exchange( atomic32* a, atomicarg( s32 ) expected, atomicarg( s32 ) desired )
{
	atomicarg( s32 ) original;
	__asm__( "lock; cmpxchgl %2, %1" : "=a"( original ), "+m"( a->value ) : "q"( desired ), "0"( expected ) );
	return original;
}

s32 atomic32_exchange( atomic32* a, atomicarg( s32 ) desired )
{
	// NOTE: No lock prefix is necessary for xchgl
	atomicarg( s32 ) original;
	__asm__( "xchgl %0, %1" : "=r"( original ), "+m"( a->value ) : "0"( desired ) );
	return original;
}

s32 atomic32_fetch_add( atomic32* a, atomicarg( s32 ) operand )
{
	atomicarg( s32 ) original;
	__asm__( "lock; xaddl %0, %1" : "=r"( original ), "+m"( a->value ) : "0"( operand ) );
	return original;
}

s32 atomic32_fetch_and( atomic32* a, atomicarg( s32 ) operand )
{
	atomicarg( s32 ) original;
	atomicarg( s32 ) tmp;
	__asm__(
	    "1:     movl    %1, %0\n"
	    "       movl    %0, %2\n"
	    "       andl    %3, %2\n"
	    "       lock; cmpxchgl %2, %1\n"
	    "       jne     1b"
	    : "=&a"( original ), "+m"( a->value ), "=&r"( tmp )
	    : "r"( operand )
	);
	return original;
}

s32 atomic32_fetch_or( atomic32* a, atomicarg( s32 ) operand )
{
	atomicarg( s32 ) original;
	atomicarg( s32 ) temp;
	__asm__(
	    "1:     movl    %1, %0\n"
	    "       movl    %0, %2\n"
	    "       orl     %3, %2\n"
	    "       lock; cmpxchgl %2, %1\n"
	    "       jne     1b"
	    : "=&a"( original ), "+m"( a->value ), "=&r"( temp )
	    : "r"( operand )
	);
	return original;
}

s64 atomic64_load( atomic64 const* a )
{
#	if defined( ZPL_ARCH_64_BIT )
	return a->value;
#	else
	atomicarg( s64 ) original;
	__asm__(
	    "movl %%ebx, %%eax\n"
	    "movl %%ecx, %%edx\n"
	    "lock; cmpxchg8b %1"
	    : "=&A"( original )
	    : "m"( a->value )
	);
	return original;
#	endif
}

void atomic64_store( atomic64* a, atomicarg( s64 ) value )
{
#	if defined( ZPL_ARCH_64_BIT )
	a->value = value;
#	else
	atomicarg( s64 ) expected = a->value;
	__asm__(
	    "1:    cmpxchg8b %0\n"
	    "      jne 1b"
	    : "=m"( a->value )
	    : "b"( ( atomicarg( s32 ) )value ), "c"( ( atomicarg( s32 ) )( value >> 32 ) ), "A"( expected )
	);
#	endif
}

s64 atomic64_compare_exchange( atomic64* a, atomicarg( s64 ) expected, atomicarg( s64 ) desired )
{
#	if defined( ZPL_ARCH_64_BIT )
	atomicarg( s64 ) original;
	__asm__( "lock; cmpxchgq %2, %1" : "=a"( original ), "+m"( a->value ) : "q"( desired ), "0"( expected ) );
	return original;
#	else
	atomicarg( s64 ) original;
	__asm__( "lock; cmpxchg8b %1" : "=A"( original ), "+m"( a->value ) : "b"( ( atomicarg( s32 ) )desired ), "c"( ( atomicarg( s32 ) )( desired >> 32 ) ), "0"( expected ) );
	return original;
#	endif
}

s64 atomic64_exchange( atomic64* a, atomicarg( s64 ) desired )
{
#	if defined( ZPL_ARCH_64_BIT )
	atomicarg( s64 ) original;
	__asm__( "xchgq %0, %1" : "=r"( original ), "+m"( a->value ) : "0"( desired ) );
	return original;
#	else
	atomicarg( s64 ) original = a->value;
	for ( ;; )
	{
		atomicarg( s64 ) previous = atomic64_compare_exchange( a, original, desired );
		if ( original == previous )
			return original;
		original = previous;
	}
#	endif
}

s64 atomic64_fetch_add( atomic64* a, atomicarg( s64 ) operand )
{
#	if defined( ZPL_ARCH_64_BIT )
	atomicarg( s64 ) original;
	__asm__( "lock; xaddq %0, %1" : "=r"( original ), "+m"( a->value ) : "0"( operand ) );
	return original;
#	else
	for ( ;; )
	{
		atomicarg( s64 ) original = a->value;
		if ( atomic64_compare_exchange( a, original, original + operand ) == original )
			return original;
	}
#	endif
}

s64 atomic64_fetch_and( atomic64* a, atomicarg( s64 ) operand )
{
#	if defined( ZPL_ARCH_64_BIT )
	atomicarg( s64 ) original;
	atomicarg( s64 ) tmp;
	__asm__(
	    "1:     movq    %1, %0\n"
	    "       movq    %0, %2\n"
	    "       andq    %3, %2\n"
	    "       lock; cmpxchgq %2, %1\n"
	    "       jne     1b"
	    : "=&a"( original ), "+m"( a->value ), "=&r"( tmp )
	    : "r"( operand )
	);
	return original;
#	else
	for ( ;; )
	{
		atomicarg( s64 ) original = a->value;
		if ( atomic64_compare_exchange( a, original, original & operand ) == original )
			return original;
	}
#	endif
}

s64 atomic64_fetch_or( atomic64* a, atomicarg( s64 ) operand )
{
#	if defined( ZPL_ARCH_64_BIT )
	atomicarg( s64 ) original;
	atomicarg( s64 ) temp;
	__asm__(
	    "1:     movq    %1, %0\n"
	    "       movq    %0, %2\n"
	    "       orq     %3, %2\n"
	    "       lock; cmpxchgq %2, %1\n"
	    "       jne     1b"
	    : "=&a"( original ), "+m"( a->value ), "=&r"( temp )
	    : "r"( operand )
	);
	return original;
#	else
	for ( ;; )
	{
		atomicarg( s64 ) original = a->value;
		if ( atomic64_compare_exchange( a, original, original | operand ) == original )
			return original;
	}
#	endif
}

#elif ! defined( ZPL_COMPILER_MSVC )
s32 atomic32_load( atomic32 const* a )
{
	return __atomic_load_n( ( s32* )&a->value, __ATOMIC_SEQ_CST );
}

void atomic32_store( atomic32* a, atomicarg( s32 ) value )
{
	__atomic_store( ( s32* )&a->value, ( s32* )&value, __ATOMIC_SEQ_CST );
}

s32 atomic32_compare_exchange( atomic32* a, atomicarg( s32 ) expected, atomicarg( s32 ) desired )
{
	return __atomic_compare_exchange_n( ( s32* )&a->value, ( s32* )&expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
}

s32 atomic32_exchange( atomic32* a, atomicarg( s32 ) desired )
{
	return __atomic_exchange_n( ( s32* )&a->value, desired, __ATOMIC_SEQ_CST );
}

s32 atomic32_fetch_add( atomic32* a, atomicarg( s32 ) operand )
{
	return __atomic_fetch_add( ( s32* )&a->value, operand, __ATOMIC_SEQ_CST );
}

s32 atomic32_fetch_and( atomic32* a, atomicarg( s32 ) operand )
{
	return __atomic_fetch_and( ( s32* )&a->value, operand, __ATOMIC_SEQ_CST );
}

s32 atomic32_fetch_or( atomic32* a, atomicarg( s32 ) operand )
{
	return __atomic_fetch_or( ( s32* )&a->value, operand, __ATOMIC_SEQ_CST );
}

s64 atomic64_load( atomic64 const* a )
{
	return __atomic_load_n( ( s64* )&a->value, __ATOMIC_SEQ_CST );
}

void atomic64_store( atomic64* a, atomicarg( s64 ) value )
{
	__atomic_store( ( s64* )&a->value, ( s64* )&value, __ATOMIC_SEQ_CST );
}

s64 atomic64_compare_exchange( atomic64* a, atomicarg( s64 ) expected, atomicarg( s64 ) desired )
{
	return __atomic_compare_exchange_n( ( s64* )&a->value, ( s64* )&expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
}

s64 atomic64_exchange( atomic64* a, atomicarg( s64 ) desired )
{
	return __atomic_exchange_n( ( s64* )&a->value, desired, __ATOMIC_SEQ_CST );
}

s64 atomic64_fetch_add( atomic64* a, atomicarg( s64 ) operand )
{
	return __atomic_fetch_add( ( s64* )&a->value, operand, __ATOMIC_SEQ_CST );
}

s64 atomic64_fetch_and( atomic64* a, atomicarg( s64 ) operand )
{
	return __atomic_fetch_and( ( s64* )&a->value, operand, __ATOMIC_SEQ_CST );
}

s64 atomic64_fetch_or( atomic64* a, atomicarg( s64 ) operand )
{
	return __atomic_fetch_or( ( s64* )&a->value, operand, __ATOMIC_SEQ_CST );
}

#else
#	error TODO: Implement Atomics for this CPU
#endif



b32 atomic32_spin_lock( atomic32* a, sw time_out )
{
	atomicarg( s32 ) old_value = atomic32_compare_exchange( a, 1, 0 );
	s32 counter                = 0;
	while ( old_value != 0 && ( time_out < 0 || counter++ < time_out ) )
	{
		yield_thread();
		old_value = atomic32_compare_exchange( a, 1, 0 );
		mfence();
	}
	return old_value == 0;
}

void atomic32_spin_unlock( atomic32* a )
{
	atomic32_store( a, 0 );
	mfence();
}

b32 atomic64_spin_lock( atomic64* a, sw time_out )
{
	atomicarg( s64 ) old_value = atomic64_compare_exchange( a, 1, 0 );
	atomicarg( s64 ) counter   = 0;
	while ( old_value != 0 && ( time_out < 0 || counter++ < time_out ) )
	{
		yield_thread();
		old_value = atomic64_compare_exchange( a, 1, 0 );
		mfence();
	}
	return old_value == 0;
}

void atomic64_spin_unlock( atomic64* a )
{
	atomic64_store( a, 0 );
	mfence();
}

b32 atomic32_try_acquire_lock( atomic32* a )
{
	atomicarg( s32 ) old_value;
	yield_thread();
	old_value = atomic32_compare_exchange( a, 1, 0 );
	mfence();
	return old_value == 0;
}

b32 atomic64_try_acquire_lock( atomic64* a )
{
	atomicarg( s64 ) old_value;
	yield_thread();
	old_value = atomic64_compare_exchange( a, 1, 0 );
	mfence();
	return old_value == 0;
}



#if defined( ZPL_ARCH_32_BIT )

void* atomic_ptr_load( atomic_ptr const* a )
{
	return ( void* )zpl_cast( sptr ) atomic32_load( zpl_cast( atomic32 const* ) a );
}

void atomic_ptr_store( atomic_ptr* a, atomicarg( void* ) value )
{
	atomic32_store( zpl_cast( atomic32* ) a, zpl_cast( atomicarg( s32 ) ) zpl_cast( sptr ) value );
}

void* atomic_ptr_compare_exchange( atomic_ptr* a, atomicarg( void* ) expected, atomicarg( void* ) desired )
{
	return ( void* )zpl_cast( sptr )
	    atomic32_compare_exchange( zpl_cast( atomic32* ) a, zpl_cast( atomicarg( s32 ) ) zpl_cast( sptr ) expected, zpl_cast( atomicarg( s32 ) ) zpl_cast( sptr ) desired );
}

void* atomic_ptr_exchange( atomic_ptr* a, atomicarg( void* ) desired )
{
	return ( void* )zpl_cast( sptr ) atomic32_exchange( zpl_cast( atomic32* ) a, zpl_cast( atomicarg( s32 ) ) zpl_cast( sptr ) desired );
}

void* atomic_ptr_fetch_add( atomic_ptr* a, atomicarg( void* ) operand )
{
	return ( void* )zpl_cast( sptr ) atomic32_fetch_add( zpl_cast( atomic32* ) a, zpl_cast( atomicarg( s32 ) ) zpl_cast( sptr ) operand );
}

void* atomic_ptr_fetch_and( atomic_ptr* a, atomicarg( void* ) operand )
{
	return ( void* )zpl_cast( sptr ) atomic32_fetch_and( zpl_cast( atomic32* ) a, zpl_cast( atomicarg( s32 ) ) zpl_cast( sptr ) operand );
}

void* atomic_ptr_fetch_or( atomic_ptr* a, atomicarg( void* ) operand )
{
	return ( void* )zpl_cast( sptr ) atomic32_fetch_or( zpl_cast( atomic32* ) a, zpl_cast( atomicarg( s32 ) ) zpl_cast( sptr ) operand );
}

b32 atomic_ptr_spin_lock( atomic_ptr* a, sw time_out )
{
	return atomic32_spin_lock( zpl_cast( atomic32* ) a, time_out );
}

void atomic_ptr_spin_unlock( atomic_ptr* a )
{
	atomic32_spin_unlock( zpl_cast( atomic32* ) a );
}

b32 atomic_ptr_try_acquire_lock( atomic_ptr* a )
{
	return atomic32_try_acquire_lock( zpl_cast( atomic32* ) a );
}

#elif defined( ZPL_ARCH_64_BIT )

void* atomic_ptr_load( atomic_ptr const* a )
{
	return ( void* )zpl_cast( sptr ) atomic64_load( zpl_cast( atomic64 const* ) a );
}

void atomic_ptr_store( atomic_ptr* a, atomicarg( void* ) value )
{
	atomic64_store( zpl_cast( atomic64* ) a, zpl_cast( s64 ) zpl_cast( sptr ) value );
}

void* atomic_ptr_compare_exchange( atomic_ptr* a, atomicarg( void* ) expected, atomicarg( void* ) desired )
{
	return ( void* )zpl_cast( sptr ) atomic64_compare_exchange( zpl_cast( atomic64* ) a, zpl_cast( s64 ) zpl_cast( sptr ) expected, zpl_cast( s64 ) zpl_cast( sptr ) desired );
}

void* atomic_ptr_exchange( atomic_ptr* a, atomicarg( void* ) desired )
{
	return ( void* )zpl_cast( sptr ) atomic64_exchange( zpl_cast( atomic64* ) a, zpl_cast( s64 ) zpl_cast( sptr ) desired );
}

void* atomic_ptr_fetch_add( atomic_ptr* a, atomicarg( void* ) operand )
{
	return ( void* )zpl_cast( sptr ) atomic64_fetch_add( zpl_cast( atomic64* ) a, zpl_cast( s64 ) zpl_cast( sptr ) operand );
}

void* atomic_ptr_fetch_and( atomic_ptr* a, atomicarg( void* ) operand )
{
	return ( void* )zpl_cast( sptr ) atomic64_fetch_and( zpl_cast( atomic64* ) a, zpl_cast( s64 ) zpl_cast( sptr ) operand );
}

void* atomic_ptr_fetch_or( atomic_ptr* a, atomicarg( void* ) operand )
{
	return ( void* )zpl_cast( sptr ) atomic64_fetch_or( zpl_cast( atomic64* ) a, zpl_cast( s64 ) zpl_cast( sptr ) operand );
}

b32 atomic_ptr_spin_lock( atomic_ptr* a, sw time_out )
{
	return atomic64_spin_lock( zpl_cast( atomic64* ) a, time_out );
}

void atomic_ptr_spin_unlock( atomic_ptr* a )
{
	atomic64_spin_unlock( zpl_cast( atomic64* ) a );
}

b32 atomic_ptr_try_acquire_lock( atomic_ptr* a )
{
	return atomic64_try_acquire_lock( zpl_cast( atomic64* ) a );
}

#endif

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
