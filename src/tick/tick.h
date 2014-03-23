#ifndef TICK_H
#define TICK_H

// tick.h
// Rate control and updates.

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

/***********
 * Globals *
 ***********/

// The number of ticks since the start of this second:
extern int TICK_COUNT;

/*************
 * Functions *
 *************/

// Computes how many ticks should happen based on how much time has elapsed
// since the last call to ticks_expected().
int ticks_expected(void);

// Ticks forward the given number of steps.
void tick(int steps);

#endif //ifndef TICK_H
