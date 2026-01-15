#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// WiFi Access Point Configuration
#define AP_SSID "ESP32-Deauther"
#define AP_PASS "lozinka32"  // Changed from "esp32wroom32" - you can customize this

// LED Configuration
#define LED 22

// Network Configuration
#define CHANNEL_MAX 11  // Channels 1-11 (12-13 not supported by all devices)

// Deauth Attack Configuration
#define NUM_FRAMES_PER_DEAUTH 32        // Number of deauth frames per station
#define DEAUTH_BLINK_TIMES 3            // How many times LED blinks per attack
#define DEAUTH_BLINK_DURATION 85       // Total blink duration in ms (50ms on, 50ms off per blink)

// Attack Types
#define DEAUTH_TYPE_SINGLE 0            // Target single network
#define DEAUTH_TYPE_ALL 1               // Target all networks

// Debug macros (always active now)
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)

// LED blink macro
#ifdef LED
#define BLINK_LED(num_times, blink_duration) blink_led(num_times, blink_duration)
#else
#define BLINK_LED(num_times, blink_duration)
#endif

// Function declarations
void blink_led(int num_times, int blink_duration);

#endif // DEFINITIONS_H
