const int LED_PIN = 13;

const uint32_t N_LED = 164;

float weights[3] = {0.27, 0.13, 0.6};
uint8_t duration[3] = {64, 99, 199}; 
uint32_t average_duration = 0;


void preciseDelay(uint32_t us)
{
    delay(us / 1000);
    delayMicroseconds(us % 1000);
}


void setup()
{
    pinMode(LED_PIN, OUTPUT);

    // Calculate weighted average from the distribution of LED brightnesses.
    float sum = 0;
    for (int i = 0; i < 3; i++)
    {
        sum += duration[i] * weights[i];
    }

    average_duration = sum;
}


void loop()
{
    static uint32_t lastChange = micros();
    static uint8_t phase = 0;

    if (micros() - lastChange >= 500000)
    {
        lastChange += 500000;
        phase = (phase + 1) % 3;
    }

    uint32_t T_OFF_US = average_duration * (N_LED - 1);
    uint32_t T_ON_US = duration[2 - phase]; // Reverse order looks like a more natural progression

    digitalWrite(LED_PIN, HIGH);
    preciseDelay(T_ON_US);

    digitalWrite(LED_PIN, LOW);
    preciseDelay(T_OFF_US);
}