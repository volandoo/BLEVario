#!/usr/bin/env python3
"""Host verification: python3 tests/verify_checksum.py [path/to/VarioSentenceParser.swift].

Compiles the actual sendMessage method with minimal String/BLE stand-ins; no ESP32
is required. Supply the iPhone parser source to also verify Swift interoperability.
"""

from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
CASES = [
    ("LK8EX1,101325,1234,2,20,90", "06"),
    ("LK8EX1,101325,1234,169,20,90", "0A"),
    ("LK8EX1,101325,1234,17,20,90", "32"),
]

# Extract the complete production method, including XOR and sentence framing.
source = (ROOT / "src/BLEController.cpp").read_text()
start = source.index("void BLEController::sendMessage(String &message)")
end = source.index("\n}", start) + 2
method = source[start:end]
harness = r'''
#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>
using String = std::string;
struct Characteristic {
    std::string value;
    int notifications = 0;
    void setValue(const char *text) { value = text; }
    void notify() { ++notifications; }
};
void delay(int) {}
struct BLEController {
    Characteristic *txCharacteristic;
    void sendMessage(String &message);
};
''' + method + r'''
int main(int argc, char **argv) {
    Characteristic characteristic;
    BLEController controller{&characteristic};
    for (int i = 1; i < argc; ++i) {
        String message = argv[i];
        controller.sendMessage(message);
        assert(characteristic.value == message);
        assert(characteristic.notifications == i);
        std::cout << characteristic.value;
    }
}
'''

with tempfile.TemporaryDirectory(prefix="ble-checksum-") as directory:
    temp = Path(directory)
    (temp / "verify.cpp").write_text(harness)
    subprocess.run(["c++", "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(temp / "verify.cpp"), "-o", str(temp / "verify")], check=True)
    emitted = subprocess.check_output([str(temp / "verify"), *[p for p, _ in CASES]])
    expected = "".join(f"${payload}*{checksum}\r\n" for payload, checksum in CASES).encode()
    assert emitted == expected, (emitted, expected)
    print("PASS: emitted sentences preserve payload and CRLF, with checksums 06, 0A, 32")

    if len(sys.argv) > 1:
        (temp / "sentences").write_bytes(emitted)
        (temp / "main.swift").write_text(r'''
import Foundation
let data = try Data(contentsOf: URL(fileURLWithPath: CommandLine.arguments[1]))
// Exercise every BLE notification split, including between checksum digits.
for split in 0...data.count {
    let parser = VarioSentenceParser()
    let readings = parser.accept(Data(data.prefix(split)))
        + parser.accept(Data(data.dropFirst(split)))
    assert(readings.count == 3)
    for (reading, rate) in zip(readings, [0.02, 1.69, 0.17]) {
        assert(reading.barometricAltitude == 1234)
        assert(abs(reading.climbRate - rate) < 1e-9)
    }
}
print("PASS: iPhone Swift parser accepts emitted sentences at every notification split")
''')
        subprocess.run(["swiftc", str(Path(sys.argv[1]).resolve()),
                        str(temp / "main.swift"), "-o", str(temp / "parse")], check=True)
        subprocess.run([str(temp / "parse"), str(temp / "sentences")], check=True)
