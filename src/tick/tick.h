#ifndef TICK_H
#define TICK_H

// tick.h
// Rate control and updates.

/**************
 * Structures *
 **************/

struct rate_data_s;
typedef struct rate_data_s rate_data;

struct count_data_s;
typedef struct count_data_s count_data;

/*************
 * Constants *
 *************/

// The desired number of ticks per second. The number of ticks per frame may
// vary, as may the number of frames per second, but the number of ticks per
// second shouldn't rise above this value (and will only fall below it if the
// machine can't keep up).
#define TICKS_PER_SECOND 60.0
#define TICKS_PER_SECOND_I 60

// Inverse of TICKS_PER_SECOND:
#define SECONDS_PER_TICK (1.0 / TICKS_PER_SECOND)

// The size of a count_data buffer;
#define COUNT_DATA_BUFFER_SIZE 1024

// How often rate trackers recompute their rates.
extern double const DEFAULT_TRACKING_INTERVAL;

/***********
 * Globals *
 ***********/

// The number of ticks since the start of this second:
extern int TICK_COUNT;

// Whether or not automatic data loading should be performed every tick:
extern int TICK_AUTOLOAD;

// Rate trackers:
extern rate_data TICKRATE;
extern rate_data FRAMERATE;

// Count trackers:
extern count_data CHUNK_LAYERS_RENDERED;
extern count_data CHUNKS_LOADED;
extern count_data CHUNKS_COMPILED;

/*************************
 * Structure Definitions *
 *************************/

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

// Sets up the tick system, in particular initializing the tick rate tracker
// and setting the TICK_AUTOLOAD variable.
void init_tick(int autoload);

// Computes how many ticks should happen based on how much time has elapsed
// since the last call to ticks_expected().
int ticks_expected(void);

// Ticks forward the given number of steps.
void tick(int steps);

// Initializes up the given rate/count data structure with the given tracking
// interval.
void setup_rate_data(rate_data *rd, double interval);
void setup_count_data(count_data *cd, double interval);

// Call this every time the thing that you want to track occurs. Make sure to
// call setup_rate_data first.
void update_rate(rate_data *rd);

// Call this every time the thing that you want to track occurs, giving it a
// data point. Make sure to call setup_count_data first.
void update_count(count_data *cd, int data_point);

#endif //ifndef TICK_H
