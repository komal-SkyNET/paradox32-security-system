#include "Log.h"
#include "Logger.h"

void Log(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // Send to serial monitor
    Serial.print(buf);

    // Send to in-memory logger
    Logger::getInstance().add(buf);
}
