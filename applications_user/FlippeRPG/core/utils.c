#include "utils.h"

void popup_message(const char* message) {
    printf("[Popup] %s\n", message);
}

// Simple wrapper so callers can pass string literals without confusion
void popup_message_str(const char* message) {
    popup_message(message);
}