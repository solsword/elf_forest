#ifndef TICK_H
#define TICK_H

// tick.h
// Rate control and updates.

/*************
 * Constants *
 *************/

extern const float TICKS_PER_SECOND;
extern const int TICKS_PER_SECOND_I;

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
