#include "Logger.h"

Logger::Logger() {}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::add(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // Prepend timestamp
    String timestamp = "[" + String(millis()) + "] ";
    _log_lines[_current_line] = timestamp + buf;

    _current_line++;
    if (_current_line >= LOG_BUFFER_SIZE) {
        _current_line = 0;
        _buffer_full = true;
    }
}

String Logger::getHtml() {
    String html = "<!DOCTYPE html><html><head><title>ESP32 Logs</title><meta http-equiv=\"refresh\" content=\"5\"></head><body><h1>ESP32 Logs</h1><pre>";
    
    int index;
    int count = _buffer_full ? LOG_BUFFER_SIZE : _current_line;

    for (int i = 0; i < count; i++) {
        index = _current_line - 1 - i;
        if (index < 0) {
            index += LOG_BUFFER_SIZE;
        }
        html += _log_lines[index] + "\n";
    }

    html += "</pre></body></html>";
    return html;
}

