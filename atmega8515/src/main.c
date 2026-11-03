
void init_led_timer(void)
{
    TCCR0 = 0;      // Stop timer while configuring it
    TCNT0 = 0;      // Reset counter

    OCR0 = 48;

    TCCR0 = (1 << WGM01)        // CTC mode
          | (1 << CS02)
          | (1 << CS00);        // Prescaler: 1024

    TIMSK |= (1 << OCIE0);      // Enable Timer0 compare-match interrupt
}


// Quadratic progression
static const uint8_t brightness_ocr[] = {
    0, 48, 96, 192
};

/// @brief Interrupt to update the LEDs.
ISR(TIMER0_COMP_vect)
{
    uint8_t row = current_led >> 4;
    uint8_t col = current_led & 0x0F;

    // Signal   Inactive  Active
    // Rows     LOW       HIGH
    // Columns 	HIGH      LOW

    // Set prev row LOW
    *ROW_PINS[row].port &= ~ROW_PINS[row].mask;

    // Set prev col HIGH
    *COL_PINS[col].port |= COL_PINS[col].mask;


    uint8_t brightness;

    do
    {
        brightness = get_brightness(++current_led);
    }
    while (brightness == 0);

    row = current_led >> 4;
    col = current_led & 0x0F;

    // Set new row HIGH
    *ROW_PINS[row].port |= ROW_PINS[row].mask;

    // Set new col LOW
    *COL_PINS[col].port &= ~COL_PINS[col].mask;

    // Should not cause flicker or weird stuff since
    // There's no way there's enough time for the timer to
    // pass over the OCR0 value causing a wait for overflow + count.
    OCR0 = brightness_ocr[brightness];
}


void wait_for_button(void)
{
    // Disable global interrupts to prevent an ISR from executing
    // while we check the button flag and prepare to sleep.
    cli();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    while (!button_pressed)
    {
        // Enable the sleep mechanism (does not sleep yet).
        sleep_enable();

        // Re-enable interrupts.
        // AVR guarantees that the instruction immediately following
        // sei() executes before any pending interrupt is serviced.
        sei();

        // Enter the selected sleep mode (Power-down).
        // If INT0 is pending, the MCU enters sleep and immediately
        // wakes up to execute the interrupt handler.
        sleep_cpu();

        // Execution resumes here after waking up.
        // Disable the sleep mechanism.
        sleep_disable();

        // Disable interrupts again before checking button_pressed.
        cli();
    }

    // Button was pressed. Re-enable interrupts before returning.
    sei();
}


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

