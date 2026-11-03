
/// @returns true if the button remained pressed after the debounce interval;
///          false if the interrupt was caused by contact bounce.
bool debounce_button(void)
{
    // Wait for mechanical contact bouncing to settle
    _delay_ms(DEBOUNCE_DELAY_MS);

    // Check whether the button is still pressed (PD2 LOW)
    bool valid_press = !(PIND & (1 << PD2));

    if (valid_press)
    {
        // Wait until button is released
        while (!(PIND & (1 << PD2)))
        {
        }

        // Wait for release bouncing to settle
        _delay_ms(DEBOUNCE_DELAY_MS);
    }

    // Clear button flag and pending interrupt
    cli();

    button_pressed = false;
    GIFR = (1 << INTF0);

    // Re-enable INT0
    GICR |= (1 << INT0);

    sei();

    return valid_press;
}


ISR(TIMER1_COMPA_vect)
{
    simulate_step = true;
}

void init_sim_timer(void)
{

    // Stop Timer1 while configuring
    TCCR1B = 0;
    TCCR1A = 0;

    // CTC mode (TOP = OCR1A)
    TCCR1B |= (1 << WGM12);

    // Compare value
    OCR1A = OCR1A_PS1024_19_HZ;

    // Enable Timer1 Compare Match A interrupt
    TIMSK |= (1 << OCIE1A);

    // Prescaler = 1024, start timer
    TCCR1B |= (1 << CS12) | (1 << CS10);
}


int main()
{
    init_button_interrupt();

    // Prolly be redundant with the wait_for_button
    sei();

    // Wait for button press before starting I2C over pins PB5 & PB6
    do
    {
        wait_for_button();
    }
    while (!debounce_button());

    // Init I2C only after this to allow removing the programming device.
    i2c_init();
    while (!mpu6050_init())
    {
        // Blink something to indicate
        _delay_ms(250);
    }

    cli();
    simulation_init();
    init_sim_timer();

    // Set to prevent infinite loop in LED interrupt.
    update_brightness();
    init_led_pins();
    init_led_timer();
    sei();

    while (1)
    {
        if (button_pressed)
        {
            if (debounce_button())
            {
                // Toggle running
                running = !running;
            }
        }

        if (!mpu6050_is_connected())
        {
            running = false;

            while (!mpu6050_init())
            {
                _delay_ms(250);
            }
        }

        if (running)
        {
            // Choose acceleration
            // This is moved out of simulation_step for performance.
            if (--sim_accel_timer == 0)
            {
                sim_accel_timer = SIM_ACCELERATION_INTERVAL;

                // On failure set acceleration to zero and if this was
                // caused by MPU6050 disconnecting the code above will
                // handle it.
                if (!mpu6050_read_accel_xy_i8(&a.x, &a.y))
                {
                    a.x = 0u;
                    a.y = 0u;
                }

                // Divide by 4 to map acceleration to [-32, 31]
                // https://ww1.microchip.com/downloads/en/DeviceDoc/AVR-Instruction-Set-Manual-DS40002198A.pdf#_OPENTOPIC_TOC_PROCESSING_d138e17156
                a.x >>= 2;
                a.y >>= 2;
            }
            else
            {
                a.x = 0u;
                a.y = 0u;
            }

            // Idle while waiting
            while (!simulate_step)
            {
                set_sleep_mode(SLEEP_MODE_IDLE);
                sleep_mode();
            }

            simulate_step = false;
            simulation_step();
            update_brightness();
        }
        else
        {
            // Failure is acceptable: the sensor may already be disconnected.
            mpu6050_set_sleep(true);

            // Ensure I2C pins are released for possible programming.
            i2c_release_pins();

            // Sleep until interrupt
            wait_for_button();

            // If it is disconnected, the next connectivity check handles recovery.
            mpu6050_set_sleep(false);
        }
    }
}

