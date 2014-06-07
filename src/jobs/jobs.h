#ifndef JOBS_H
#define JOBS_H

// jobs.h
// Asynchronous jobs.

#include <datatypes/queue.h>

/************************
 * Types and Structures *
 ************************/

// A job is a pointer to a function that takes a void* (really a pointer to a
// job-specific memory struct) and returns a generic function pointer (really
// another job).
typedef void (*(*job)(void *)) ();

// A job_cleanup function is given a state and is expected to free any pointers
// within that state that need freeing before a job can be shut down. It will
// be called immediately prior to final work state cleanup.
typedef void (*job_cleanup)(void *);

// A work state struct holds a job and its state, as well as a cleanup function
// for freeing any malloc'd pointers that the state might have internally. The
// cleanup function can be NULL if it's not needed.
struct work_state_s;
typedef struct work_state_s work_state;

/***********
 * Globals *
 ***********/

extern queue* active_jobs;

/*************************
 * Structure Definitions *
 *************************/

struct work_state_s {
  job j;
  void * s;
  job_cleanup cl;
};

/********************
 * Inline Functions *
 ********************/

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new work state.
work_state* create_work_state(job j, void *s, job_cleanup cl);

// Cleans up the given work state. The work state's state is assumed to have
// been allocated on the heap, and is freed. If it needs internal pointers
// freed, it should return a cleanup job before returning NULL and that cleanup
// job can free what needs to be freed.
void cleanup_work_state(work_state *ws);

// Just calls cleanup_work_state on the given void*.
void cleanup_work_state_in_queue(void *wsp);

/*************
 * Functions *
 *************/

// Sets up the jobs system.
void setup_jobs();

// Cleans up the jobs system.
void cleanup_jobs();

// Adds the given job to the jobs queue.
void start_job(job j, void *s, job_cleanup cl);

// Runs the next work step on the jobs queue.
void do_step();

// Runs the next step of the given job, hopefully not blocking for very long.
void do_step_for(work_state *ws);

// Runs the given job to completion, possibly blocking for a while.
void run_to_completion(work_state *ws);

#endif // ifndef JOBS_H
