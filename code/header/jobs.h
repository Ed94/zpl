// file: header/jobs.h

/** @file threadpool.c
@brief Job system
@defgroup jobs Job system

 This job system follows thread pool pattern to minimize the costs of thread initialization.
 It reuses fixed number of threads to process variable number of jobs.

 @{
 */

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef void ( *jobs_proc )( void* data );

#define ZPL_INVALID_JOB ZPL_U32_MAX

#ifndef ZPL_JOBS_MAX_QUEUE
#	define ZPL_JOBS_MAX_QUEUE 100
#endif

#ifdef ZPL_JOBS_ENABLE_DEBUG
#	define ZPL_JOBS_DEBUG
#endif

typedef enum
{
	ZPL_JOBS_STATUS_READY,
	ZPL_JOBS_STATUS_BUSY,
	ZPL_JOBS_STATUS_WAITING,
	ZPL_JOBS_STATUS_TERM,
} jobs_status;

typedef enum
{
	ZPL_JOBS_PRIORITY_REALTIME,
	ZPL_JOBS_PRIORITY_HIGH,
	ZPL_JOBS_PRIORITY_NORMAL,
	ZPL_JOBS_PRIORITY_LOW,
	ZPL_JOBS_PRIORITY_IDLE,
	ZPL_JOBS_MAX_PRIORITIES,
} jobs_priority;

typedef struct
{
	jobs_proc proc;
	void*     data;
} thread_job;

ZPL_RING_DECLARE( extern, _jobs_ring_, thread_job );

typedef struct
{
	thread     thread;
	atomic32   status;
	thread_job job;
#ifdef ZPL_JOBS_DEBUG
	u32 hits;
	u32 idle;
#endif
} thread_worker;

typedef struct
{
	_jobs_ring_zpl_thread_job jobs;    ///< zpl_ring
	u32                       chance;
#ifdef ZPL_JOBS_DEBUG
	u32 hits;
#endif
} thread_queue;

typedef struct
{
	AllocatorInfo  alloc;
	u32            max_threads, max_jobs, counter;
	thread_worker* workers;    ///< zpl_buffer
	thread_queue   queues[ ZPL_JOBS_MAX_PRIORITIES ];
} jobs_system;

//! Initialize thread pool with specified amount of fixed threads.
ZPL_DEF void jobs_init( jobs_system* pool, AllocatorInfo a, u32 max_threads );

//! Initialize thread pool with specified amount of fixed threads and custom job limit.
ZPL_DEF void jobs_init_with_limit( jobs_system* pool, AllocatorInfo a, u32 max_threads, u32 max_jobs );

//! Release the resources use by thread pool.
ZPL_DEF void jobs_free( jobs_system* pool );

//! Enqueue a job with specified data and custom priority.
ZPL_DEF b32 jobs_enqueue_with_priority( jobs_system* pool, jobs_proc proc, void* data, jobs_priority priority );

//! Enqueue a job with specified data.
ZPL_DEF b32 jobs_enqueue( jobs_system* pool, jobs_proc proc, void* data );

//! Check if the work queue is empty.
ZPL_DEF b32 jobs_empty( jobs_system* pool, jobs_priority priority );

ZPL_DEF b32 jobs_empty_all( jobs_system* pool );
ZPL_DEF b32 jobs_full_all( jobs_system* pool );

//! Check if the work queue is full.
ZPL_DEF b32 jobs_full( jobs_system* pool, jobs_priority priority );

//! Check if all workers are done.
ZPL_DEF b32 jobs_done( jobs_system* pool );

//! Process all jobs and check all threads. Should be called by Main Thread in a tight loop.
ZPL_DEF b32 jobs_process( jobs_system* pool );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
