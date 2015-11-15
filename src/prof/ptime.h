#ifndef PTIME_H
#define PTIME_H

// ptime.h
// Profiling utilities for tracking time and rates.

/**************
 * Structures *
 **************/

struct duration_data_s;
typedef struct duration_data_s duration_data;

struct rate_data_s;
typedef struct rate_data_s rate_data;

struct count_data_s;
typedef struct count_data_s count_data;

/*************
 * Constants *
 *************/

// The size of a count_data buffer;
#define COUNT_DATA_BUFFER_SIZE 1024

// How often rate trackers recompute their rates.
extern double const DEFAULT_TRACKING_INTERVAL;

// The default weight of each incoming sample vs. the current average for
// duration estimation:
extern double const DEFAULT_AVERAGING_WEIGHT;

/***********
 * Globals *
 ***********/

// Rate trackers:
extern rate_data TICKRATE;
extern rate_data FRAMERATE;

extern duration_data RENDER_TIME;
extern duration_data RENDER_AREA_TIME;
extern duration_data RENDER_UI_TIME;
extern duration_data RENDER_CORE_TIME;
extern duration_data RENDER_INNER_TIME;
extern duration_data COMPILE_TIME;
extern duration_data PHYSICS_TIME;
extern duration_data DATA_TIME;
extern duration_data TGEN_TIME;
extern duration_data DISK_READ_TIME;
extern duration_data DISK_MISS_TIME;
extern duration_data DISK_WRITE_TIME;

// Count trackers:
extern count_data CHUNK_LAYERS_RENDERED;
extern count_data CHUNKS_LOADED;
extern count_data CHUNKS_COMPILED;
extern count_data CHUNKS_BIOGEND;
extern count_data CHUNKS_BIOSKIPPED;

/*************************
 * Structure Definitions *
 *************************/

struct duration_data_s {
  double starttime; // when was star_duration called?
  double duration; // the current duration estimate for one cycle
  double weight; // weight of incoming sample vs. existing average
};

struct rate_data_s {
  int count; // how many times the event has happened in this interval
  double lasttime; // the last time update_rate was called
  double elapsed; // time elapsed since the start of this interval
  double interval; // how often to compute the rate
  double rate; // the rate estimate for the previous interval
};

struct count_data_s {
  int count; // how many times the event has happened in this interval
  int data[COUNT_DATA_BUFFER_SIZE]; // the last few counts for the event
  size_t next_idx; // pointer to the next open entry in the data array
  double lasttime; // the last time update_rate was called
  double elapsed; // time elapsed since the start of this interval
  double interval; // how often to compute the average
  int average; // the count estimate for the previous interval
};

/*************
 * Functions *
 *************/

// Initializes up the given rate/count data structure with the given tracking
// interval.
void setup_duration_data(duration_data *sd, double weight);
void setup_rate_data(rate_data *rd, double interval);
void setup_count_data(count_data *cd, double interval);

// Sets up the time profiling system, in particular initializing the tick rate
// tracker.
void init_ptime(void);

// Call these every time the thing that you want to time happens. Make sure to
// call setup_duration_data first.
void start_duration(duration_data *sd);
void end_duration(duration_data *sd);

// Call this every time the thing that you want to track occurs. Make sure to
// call setup_rate_data first.
void update_rate(rate_data *rd);

// Call this every time the thing that you want to track occurs, giving it a
// data point. Make sure to call setup_count_data first.
void update_count(count_data *cd, int data_point);

#endif //ifndef PTIME_H
