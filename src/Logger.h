#pragma once

#include <Arduino.h>

class Logger {
public:
    static const int LOG_BUFFER_SIZE = 500;

    static Logger& getInstance();

    void add(const char* fmt, ...);
    String getHtml();

    // New methods for chunked transfer
    const String* getLogLines() const { return _log_lines; }
    int getCurrentLine() const { return _current_line; }
    bool isBufferFull() const { return _buffer_full; }

private:
    Logger(); // Private constructor
    String _log_lines[LOG_BUFFER_SIZE];
    int _current_line = 0;
    bool _buffer_full = false;

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;
};
