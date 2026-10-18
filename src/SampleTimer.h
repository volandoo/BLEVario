#ifndef SAMPLE_TIMER_H
#define SAMPLE_TIMER_H

#include <stdint.h>

class SampleTimer
{
public:
    double elapsedSeconds(uint32_t now)
    {
        if (!started)
        {
            started = true;
            previous = now;
            return 1.0; // Safe interval for the filter's first measurement.
        }
        const uint32_t elapsed = now - previous; // Handles millis() rollover.
        previous = now;
        return elapsed / 1000.0;
    }

    void reset() { started = false; }

private:
    uint32_t previous = 0;
    bool started = false;
};

#endif
