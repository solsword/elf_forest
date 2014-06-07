// jobs.c
// Asynchronous jobs.

#include "datatypes/queue.h"

#include "jobs.h"

/***********
 * Globals *
 ***********/

queue* ACTIVE_JOBS = NULL;

/******************************
 * Constructors & Destructors *
 ******************************/

work_state* create_work_state(job j, void *s, job_cleanup cl) {
  work_state *result = (work_state *) malloc(sizeof(work_state));
  result->j = j;
  result->s = s;
  result->cl = cl;
  return result;
}

void cleanup_work_state(work_state *ws) {
  // Call the cleanup code if it exists:
  if (ws->cl != NULL) {
    (*(ws->cl))(ws->s);
  }
  free(ws->s);
  free(ws);
}

void cleanup_work_state_in_queue(void *wsp) {
  cleanup_work_state((work_state *) wsp);
}

/*************
 * Functions *
 *************/

void setup_jobs() {
  ACTIVE_JOBS = create_queue();
}

void cleanup_jobs() {
  q_foreach(ACTIVE_JOBS, &cleanup_work_state_in_queue);
  cleanup_queue(ACTIVE_JOBS);
}

void start_job(job j, void *s, job_cleanup cl) {
  work_state *ws = create_work_state(j, s, cl);
  q_push_element(ACTIVE_JOBS, (void *) ws);
}

void do_step() {
  work_state *nws = (work_state*) q_pop_element(ACTIVE_JOBS);
  if (nws == NULL) {
    return; // no work to do at this time
  }
  do_step_for(nws);
  if (nws->j == NULL) {
    cleanup_work_state(nws);
  } else {
    q_push_element(ACTIVE_JOBS, (void *) nws);
  }
}

void do_step_for(work_state *ws) {
  ws->j = (job) ((*(ws->j))(ws->s));
}

void run_to_completion(work_state *ws) {
  while (ws->j != NULL) {
    ws->j = (job) ((*(ws->j))(ws->s));
  }
}
