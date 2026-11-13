/**
 * @file sim_timer.h
 * @author Johannes Ylinen
 * @brief
 * @date 2026-11-13
 */

#ifndef SIM_TIMER_H
#define SIM_TIMER_H


/**
 * @brief Configure Timer1 for the 1/FPS simulation tick.
 * Timer stays stopped.
 */
void sim_timer_init(void);
void sim_timer_start(void);

/// @brief Stop the tick and discard any pending step.
void sim_timer_stop(void);

/// @brief Sleep (idle mode) until the next simulation tick has elapsed.
void sim_timer_wait_step(void);


#endif // SIM_TIMER_H
