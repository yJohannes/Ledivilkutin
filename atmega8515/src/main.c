/**
 * @file main.c
 * @author Johannes Ylinen
 * @brief Main entry point of the ledivilkutin.
 * @date 2026-11-14
 */

#include "ledivilkutin.h"


int main()
{
    ledivilkutin_init();

    for (;;)
    {
        ledivilkutin_run_once();
    }
}
