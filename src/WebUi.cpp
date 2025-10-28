#include "WebUi.h"
#include "Config.h" // Include for DEBUG_PRINTLN
#include "Logger.h"
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

WebUi::WebUi() {}

void WebUi::setup() {
    server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!request->authenticate(CONFIG_PORTAL_SSID, CONFIG_PORTAL_PASSWORD)) {
            // Manually send the 401 Unauthorized response
            AsyncWebServerResponse *response = request->beginResponse(401);
            response->addHeader("WWW-Authenticate", "Basic realm=\"ESP32 Logs\"");
            request->send(response);
            return;
        }

        AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
            static const char* header = "<!DOCTYPE html><html><head><title>ESP32 Logs</title><meta http-equiv=\"refresh\" content=\"5\"></head><body><h1>ESP32 Logs</h1><pre>";
            static const char* footer = "</pre></body></html>";
            static int log_index = -1; // -1 for header, 0 to count-1 for logs, count for footer

            if (index == 0) {
                log_index = -1; // Reset for a new request
            }

            if (log_index == -1) {
                size_t len = strlen(header);
                if (len > maxLen) return 0; // Should not happen
                memcpy(buffer, header, len);
                log_index++;
                return len;
            }

            const Logger& logger = Logger::getInstance();
            const String* log_lines = logger.getLogLines();
            int current_line = logger.getCurrentLine();
            bool buffer_full = logger.isBufferFull();
            int count = buffer_full ? Logger::LOG_BUFFER_SIZE : current_line;

            if (log_index < count) {
                int line_to_send_index = current_line - 1 - log_index;
                if (line_to_send_index < 0) {
                    line_to_send_index += Logger::LOG_BUFFER_SIZE;
                }
                String line = log_lines[line_to_send_index] + "\n";
                size_t len = line.length();
                if (len > maxLen) len = maxLen; // Truncate if necessary
                memcpy(buffer, line.c_str(), len);
                log_index++;
                return len;
            }

            if (log_index == count) {
                size_t len = strlen(footer);
                if (len > maxLen) return 0; // Should not happen
                memcpy(buffer, footer, len);
                log_index++;
                return len;
            }

            return 0; // End of stream
        });

        request->send(response);
    });

    server.begin();
    DEBUG_PRINTLN("[WebUI] Web server started. Access logs at /logs");
}
