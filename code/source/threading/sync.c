// file: source/threading/sync.c

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

void sync_init( sync* s )
{
	zero_item( s );
	mutex_init( &s->mutex );
	mutex_init( &s->start );
	semaphore_init( &s->release );
}

void sync_destroy( sync* s )
{
	if ( s->waiting )
	{
		ZPL_PANIC( "Cannot destroy while threads are waiting!" );
	}

	mutex_destroy( &s->mutex );
	mutex_destroy( &s->start );
	semaphore_destroy( &s->release );
}

void sync_set_target( sync* s, s32 count )
{
	mutex_lock( &s->start );

	mutex_lock( &s->mutex );
	ZPL_ASSERT( s->target == 0 );
	s->target  = count;
	s->current = 0;
	s->waiting = 0;
	mutex_unlock( &s->mutex );
}

void sync_release( sync* s )
{
	if ( s->waiting )
	{
		semaphore_release( &s->release );
	}
	else
	{
		s->target = 0;
		mutex_unlock( &s->start );
	}
}

s32 sync_reach( sync* s )
{
	s32 n;
	mutex_lock( &s->mutex );
	ZPL_ASSERT( s->current < s->target );
	n = ++s->current; // NOTE: Record this value to avoid possible race if `return s->current` was done
	if ( s->current == s->target )
		sync_release( s );
	mutex_unlock( &s->mutex );
	return n;
}

void sync_reach_and_wait( sync* s )
{
	mutex_lock( &s->mutex );
	ZPL_ASSERT( s->current < s->target );
	s->current++;
	if ( s->current == s->target )
	{
		sync_release( s );
		mutex_unlock( &s->mutex );
	}
	else
	{
		s->waiting++; // NOTE: Waiting, so one more waiter
		mutex_unlock( &s->mutex ); // NOTE: Release the mutex to other threads
		semaphore_wait( &s->release ); // NOTE: Wait for merge completion
		mutex_lock( &s->mutex ); // NOTE: On merge completion, lock mutex
		s->waiting--; // NOTE: Done waiting
		sync_release( s ); // NOTE: Restart the next waiter
		mutex_unlock( &s->mutex );
	}
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
