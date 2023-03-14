// a_file: source/jobs.c

///////////////////////////////////////////////////////////////
//
// Thread Pool
//

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

ZPL_RING_DEFINE( zpl__jobs_ring_, thread_job );

global const u32 zpl__jobs_chances[ ZPL_JOBS_MAX_PRIORITIES ] = { 2, 3, 5, 7, 11 };

sw zpl__jobs_entry( struct thread* thread )
{
	thread_worker* tw = ( thread_worker* )thread->user_data;

	for ( ;; )
	{
		u32 status = atomic32_load( &tw->status );

		switch ( status )
		{
			case ZPL_JOBS_STATUS_READY :
				{
					atomic32_store( &tw->status, ZPL_JOBS_STATUS_BUSY );
					tw->job.proc( tw->job.data );
					atomic32_compare_exchange( &tw->status, ZPL_JOBS_STATUS_BUSY, ZPL_JOBS_STATUS_WAITING );

#ifdef ZPL_JOBS_DEBUG
					++tw->hits;
#endif
				}
				break;

			case ZPL_JOBS_STATUS_WAITING :
				{
#ifdef ZPL_JOBS_DEBUG
					++tw->idle;
#endif
					yield();
				}
				break;

			case ZPL_JOBS_STATUS_TERM :
				{
					return 0;
				}
				break;
		}
	}

	return 0;
}

void jobs_init( jobs_system* a_pool, allocator a, u32 max_threads )
{
	jobs_init_with_limit( a_pool, a, max_threads, ZPL_JOBS_MAX_QUEUE );
}

void jobs_init_with_limit( jobs_system* a_pool, allocator a, u32 max_threads, u32 max_jobs )
{
	jobs_system pool_ = { 0 };
	*a_pool           = pool_;

	a_pool->a_allocator = a;
	a_pool->max_threads = max_threads;
	a_pool->max_jobs    = max_jobs;
	a_pool->counter     = 0;

	buffer_init( a_pool->workers, a, max_threads );

	for ( uw i = 0; i < ZPL_JOBS_MAX_PRIORITIES; ++i )
	{
		thread_queue* q = &a_pool->queues[ i ];
		zpl__jobs_ring_init( &q->jobs, a, max_jobs );
		q->chance = zpl__jobs_chances[ i ];
	}

	for ( uw i = 0; i < max_threads; ++i )
	{
		thread_worker  worker_ = { 0 };
		thread_worker* tw      = a_pool->workers + i;
		*tw                    = worker_;

		thread_init( &tw->thread );
		atomic32_store( &tw->status, ZPL_JOBS_STATUS_WAITING );
		thread_start( &tw->thread, zpl__jobs_entry, ( void* )tw );
	}
}

void jobs_free( jobs_system* a_pool )
{
	for ( uw i = 0; i < a_pool->max_threads; ++i )
	{
		thread_worker* tw = a_pool->workers + i;

		atomic32_store( &tw->status, ZPL_JOBS_STATUS_TERM );
		thread_destroy( &tw->thread );
	}

	buffer_free( a_pool->workers );

	for ( uw i = 0; i < ZPL_JOBS_MAX_PRIORITIES; ++i )
	{
		thread_queue* q = &a_pool->queues[ i ];
		zpl__jobs_ring_free( &q->jobs );
	}
}

b32 jobs_enqueue_with_priority( jobs_system* a_pool, jobs_proc proc, void* data, jobs_priority priority )
{
	ZPL_ASSERT( priority >= 0 && priority < ZPL_JOBS_MAX_PRIORITIES );
	ZPL_ASSERT_NOT_NULL( proc );
	thread_job job = { 0 };
	job.proc       = proc;
	job.data       = data;

	if ( ! jobs_full( a_pool, priority ) )
	{
		zpl__jobs_ring_append( &a_pool->queues[ priority ].jobs, job );
		return true;
	}
	return false;
}

b32 jobs_enqueue( jobs_system* a_pool, jobs_proc proc, void* data )
{
	return jobs_enqueue_with_priority( a_pool, proc, data, ZPL_JOBS_PRIORITY_NORMAL );
}

b32 jobs_empty( jobs_system* a_pool, jobs_priority priority )
{
	ZPL_ASSERT( priority >= 0 && priority < ZPL_JOBS_MAX_PRIORITIES );
	return zpl__jobs_ring_empty( &a_pool->queues[ priority ].jobs );
}

b32 jobs_full( jobs_system* a_pool, jobs_priority priority )
{
	ZPL_ASSERT( priority >= 0 && priority < ZPL_JOBS_MAX_PRIORITIES );
	return zpl__jobs_ring_full( &a_pool->queues[ priority ].jobs );
}

b32 jobs_done( jobs_system* a_pool )
{
	for ( uw i = 0; i < a_pool->max_threads; ++i )
	{
		thread_worker* tw = a_pool->workers + i;
		if ( atomic32_load( &tw->status ) != ZPL_JOBS_STATUS_WAITING )
		{
			return false;
		}
	}

	return jobs_empty_all( a_pool );
}

b32 jobs_empty_all( jobs_system* a_pool )
{
	for ( uw i = 0; i < ZPL_JOBS_MAX_PRIORITIES; ++i )
	{
		if ( ! jobs_empty( a_pool, ( jobs_priority )i ) )
		{
			return false;
		}
	}
	return true;
}

b32 jobs_full_all( jobs_system* a_pool )
{
	for ( uw i = 0; i < ZPL_JOBS_MAX_PRIORITIES; ++i )
	{
		if ( ! jobs_full( a_pool, ( jobs_priority )i ) )
		{
			return false;
		}
	}
	return true;
}

b32 jobs_process( jobs_system* a_pool )
{
	if ( jobs_empty_all( a_pool ) )
	{
		return false;
	}
	// NOTE: Process the jobs
	for ( uw i = 0; i < a_pool->max_threads; ++i )
	{
		thread_worker* tw         = a_pool->workers + i;
		u32            status     = atomic32_load( &tw->status );
		b32            last_empty = false;

		if ( status == ZPL_JOBS_STATUS_WAITING )
		{
			for ( uw j = 0; j < ZPL_JOBS_MAX_PRIORITIES; ++j )
			{
				thread_queue* q = &a_pool->queues[ j ];
				if ( jobs_empty( a_pool, ( jobs_priority )j ) )
				{
					last_empty = ( j + 1 == ZPL_JOBS_MAX_PRIORITIES );
					continue;
				}
				if ( ! last_empty && ( ( a_pool->counter++ % q->chance ) != 0 ) )
				{
					continue;
				}

				last_empty = false;
				tw->job    = *zpl__jobs_ring_get( &q->jobs );
				atomic32_store( &tw->status, ZPL_JOBS_STATUS_READY );
#ifdef ZPL_JOBS_DEBUG
				++q->hits;
#endif
				break;
			}
		}
	}

	return true;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
