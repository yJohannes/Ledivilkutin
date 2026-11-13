/**
 * @file ledivilkutin.h
 * @author Johannes Ylinen
 * @brief Initialization and execution of the ledivilkutin.
 * @date 2026-11-14
 */

#ifndef LEDIVILKUTIN_H
#define LEDIVILKUTIN_H


/// @brief Initialize hardware and wait for the first button press
// to ensure the programmer is removed safely.
void ledivilkutin_init(void);

/// @brief Run one iteration of the state machine.
void ledivilkutin_run_once(void);


#endif // LEDIVILKUTIN_H
