#include "debug_serial.h"

#include "config.h"

#include <Arduino.h>

namespace {

void waitForUsbHost() {
    const uint32_t deadline = millis() + USB_SERIAL_HOST_WAIT_MS;
    while (!Serial && millis() < deadline) {
        delay(10);
    }
}

void printlnAndFlush(const char* line) {
    Serial.println(line);
    Serial.flush();
}

}  // namespace

namespace debug_serial {

void begin() {
    Serial.begin(USB_SERIAL_BAUD);
}

void bootOk() {
    waitForUsbHost();
    printlnAndFlush("BOOT OK");
}

void error(const char* code) {
    waitForUsbHost();
    printlnAndFlush(code);
}

}  // namespace debug_serial
