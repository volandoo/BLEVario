// Run from the repository root:
// c++ -std=c++11 -Isrc -Ilib/Altimeter tests/verify_sample_timing.cpp lib/Altimeter/Altimeter.cpp lib/Altimeter/KalmanFilter.cpp -o /tmp/verify_sample_timing
// /tmp/verify_sample_timing
#include "SampleTimer.h"
#include "Altimeter.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <initializer_list>

int main()
{
    SampleTimer timer;
    assert(timer.elapsedSeconds(0) == 1.0);
    assert(timer.elapsedSeconds(20) == 0.020);
    assert(timer.elapsedSeconds(120) == 0.100);
    assert(timer.elapsedSeconds(120) == 0.0);
    assert(timer.elapsedSeconds(140) == 0.020);
    timer.reset();
    assert(timer.elapsedSeconds(UINT32_MAX - 9) == 1.0);
    assert(timer.elapsedSeconds(10) == 0.020);
    timer.reset();
    assert(timer.elapsedSeconds(5000) == 1.0);

    // Feed real pressure/altitude/filter code a known climb or sink at two
    // sample rates. The reciprocal-milliseconds bug gives incorrect speeds.
    for (uint32_t interval : {20u, 100u})
    {
        for (double rate : {-2.0, 0.0, 3.0})
        {
            SampleTimer samples;
            Altimeter altimeter;
            for (uint32_t ms = 0; ms <= 30000; ms += interval)
            {
                const double altitude = 1000.0 + rate * ms / 1000.0;
                const double pressure = 101325.0 * std::pow(
                    1.0 - altitude / 44330.0, 1.0 / 0.190295);
                altimeter.addPressure(pressure, samples.elapsedSeconds(ms));
            }
            assert(std::abs(altimeter.varioValue() - rate) < 0.01);
        }
    }
    std::puts("PASS: sample timing, rollover, and climb/sink estimates");
}
