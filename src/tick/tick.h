#ifndef TICK_H
#define TICK_H

// tick.h
// Rate control and updates.

/**************
 * Structures *
 **************/

struct rate_data_s;
typedef struct rate_data_s rate_data;

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

// How often rate trackers recompute their rates.
extern double const DEFAULT_TRACKING_INTERVAL;

/***********
 * Globals *
 ***********/

// The number of ticks since the start of this second:
extern int TICK_COUNT;

// Rate trackers:
extern rate_data TICKRATE;
extern rate_data FRAMERATE;

/*************************
 * Structure Definitions *
 *************************/

struct rate_data_s {
  int count; // how many times the event has happened in this interval
  double lasttime; // the last time update_rate was called
  double elapsed; // time elapsed since the start of this interval
  double rate; // the rate estimate for the previous interval
  double interval; // how often to compute the rate
};

/*************
 * Functions *
 *************/

// Sets up the tick system, in particular initializing the tick rate tracker.
void init_tick(void);

// Computes how many ticks should happen based on how much time has elapsed
// since the last call to ticks_expected().
int ticks_expected(void);

// Ticks forward the given number of steps.
void tick(int steps);

// Initializes up the given rate data structure with the given tracking
// interval.
void setup_rate_data(rate_data *rd, double interval);

// Call this every time the thing that you want to track occurs. Make sure to
// call setup_rate_data first.
void update_rate(rate_data *rd);

#endif //ifndef TICK_H
