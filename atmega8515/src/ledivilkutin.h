#ifndef LEDIVILKUTIN_H
#define LEDIVILKUTIN_H

/// One-time hardware and module initialisation.
void ledivilkutin_init(void);

/// Run one iteration of the state machine. Call forever from main().
void ledivilkutin_run_once(void);


#endif // LEDIVILKUTIN_H
