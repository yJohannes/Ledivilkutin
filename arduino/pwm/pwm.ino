const int LED_PIN = 13;

const uint32_t N = 164;
const float F = 50; // Keep low for performance
const float T = 1.0 / F;
const float T_ON = T / N;
const float T_OFF = T - T_ON;

// const uint32_t T_ON_US = (uint32_t)(T_ON * 1000000UL);
// const uint32_t T_OFF_US = (uint32_t)(T_OFF * 1000000.0f);

uint32_t T_ON_US = 132;
uint32_t T_OFF_US = 132*(N-1); // Average of 66, 132, 198


void preciseDelay(uint32_t us)
{
    delay(us / 1000);
    delayMicroseconds(us % 1000);
}

void setup()
{
    pinMode(LED_PIN, OUTPUT);
}

// void loop()
// {
//     digitalWrite(LED_PIN, HIGH);
//     preciseDelay(T_ON_US);

//     digitalWrite(LED_PIN, LOW);
//     preciseDelay(T_OFF_US);
// }


void loop()
{
    static uint32_t lastChange = micros();
    static uint8_t phase = 0;

    if (micros() - lastChange >= 500000)
    {
        lastChange += 500000;
        phase = (phase + 1) % 3;
    }

    // const uint32_t T_ON_US = (phase == 0) ? 198 :
    //                          (phase == 1) ? 132 :
    //                                         66;
    T_OFF_US = 115*(N-1); // Average of 49, 99, 198

    T_ON_US = (phase == 0) ? 198 :
                (phase == 1) ? 99 :
                            49;

    digitalWrite(LED_PIN, HIGH);
    preciseDelay(T_ON_US);

    digitalWrite(LED_PIN, LOW);
    preciseDelay(T_OFF_US);
}