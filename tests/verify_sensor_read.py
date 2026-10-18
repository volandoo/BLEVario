#!/usr/bin/env python3
"""Run with python3 tests/verify_sensor_read.py; requires a host C++ compiler."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / "lib/MS5611/MS5611.cpp").read_text()
start = source.index("uint32_t MS5611::readnBytes(uint8_t nBytes)")
method = source[start:source.index("\n}", start) + 2]
harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#define ADD_MS5611 0x77
struct FakeWire {
    bool writeLocked = false;
    int count = 0, index = 0, requests = 0;
    bool shortRead = false;
    void beginTransmission(int) { writeLocked = true; }
    int endTransmission() { assert(writeLocked); writeLocked = false; return 0; }
    int requestFrom(uint8_t address, uint8_t size) {
        // ESP32 Wire would block here while holding an unfinished write lock.
        assert(!writeLocked);
        assert(address == ADD_MS5611);
        ++requests;
        index = 0;
        count = shortRead ? size - 1 : size;
        return count;
    }
    int available() { return count - index; }
    int read() { return 0x12 + index++; }
} Wire;
struct MS5611 { uint32_t readnBytes(uint8_t); };
''' + method + r'''
int main() {
    MS5611 sensor;
    // Six calibration words, then pressure/temperature ADC reads.
    for (int i = 0; i < 6; ++i) assert(sensor.readnBytes(2) == 0x1213);
    assert(sensor.readnBytes(3) == 0x121314);
    assert(sensor.readnBytes(3) == 0x121314);
    Wire.shortRead = true;
    assert(sensor.readnBytes(2) == 0);
    Wire.shortRead = false;
    assert(sensor.readnBytes(2) == 0x1213);
    const int requests = Wire.requests;
    assert(sensor.readnBytes(0) == 0);
    assert(sensor.readnBytes(5) == 0);
    assert(Wire.requests == requests);
    puts("PASS: sensor reads avoid nested Wire locks; decoding and short reads verified");
}
'''
with tempfile.TemporaryDirectory(prefix="sensor-read-") as directory:
    temp = Path(directory)
    (temp / "test.cpp").write_text(harness)
    subprocess.run(["c++", "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(temp / "test.cpp"), "-o", str(temp / "test")], check=True)
    subprocess.run([str(temp / "test")], check=True)
