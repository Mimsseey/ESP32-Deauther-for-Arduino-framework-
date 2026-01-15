// ESP32_Deauther.ino - Main sketch file
#include <WiFi.h>
#include <esp_wifi.h>
#include "types.h"
#include "web_interface.h"
#include "deauth.h"
#include "definitions.h"

int curr_channel = 1;

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);
  delay(1000);  // Wait for serial to stabilize
  Serial.println("\n\n=================================");
  Serial.println("ESP32 Deauther Starting...");
  Serial.println("=================================");

#ifdef LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.println("LED initialized on pin 13");
#endif

  // CRITICAL - Complete WiFi reset before starting
  Serial.println("\n[1/5] Resetting WiFi...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(600);  // Increased delay for proper reset
  Serial.println("      WiFi reset complete");
  
  // Configure IP addresses for webUI before starting AP
  Serial.println("\n[2/5] Configuring IP addresses...");
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  if (WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("      IP configuration successful");
  } else {
    Serial.println("      IP configuration failed!");
  }
  
  // Start WiFi in AP mode
  Serial.println("\n[3/5] Starting AP mode...");
  WiFi.mode(WIFI_AP);
  delay(300);
  Serial.println("      Mode set to AP");
  
  // Start the Access Point
  Serial.println("\n[4/5] Creating Access Point...");
  Serial.print("      SSID: ");
  Serial.println(AP_SSID);
  Serial.print("      Password: ");
  Serial.println(AP_PASS);
  Serial.println("      Channel: 1");
  Serial.println("      Max Connections: 4");
  
  // Parameters: SSID, password, channel, hidden, max_connections
  bool ap_started = WiFi.softAP(AP_SSID, AP_PASS, 1, false, 4);
  
  delay(600);  // Give AP time to fully start TODO: Change timing logic to millis() instead delay
  
  if (ap_started) {
    Serial.println("\n      ✓ Access Point started successfully!");
    Serial.print("      IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("      MAC Address: ");
    Serial.println(WiFi.softAPmacAddress());
  } else {
    Serial.println("\n      ✗ Access Point FAILED to start!");
    Serial.println("      Attempting recovery...");
    
    // Try again with default settings
    delay(1000);
    WiFi.softAP(AP_SSID, AP_PASS);
    delay(500);
    
    Serial.print("      Retry IP: ");
    Serial.println(WiFi.softAPIP());
  }
  
  // Start web server
  Serial.println("\n[5/5] Starting web interface...");
  start_web_interface();
  Serial.println("      Web server started");
  
  Serial.println("\n=================================");
  Serial.println("SETUP COMPLETE!");
  Serial.println("=================================");
  Serial.println("Connect to WiFi network: " + String(AP_SSID));
  Serial.println("Then open: http://192.168.4.1");
  Serial.println("=================================\n");
}

void loop() {
  static unsigned long last_status = 0;
  static int last_client_count = -1;
  
  // Print connection status every 2 seconds
  if (millis() - last_status > 2000) {
    last_status = millis();
    
    int client_count = WiFi.softAPgetStationNum();
    
    // Only print if count changed
    if (client_count != last_client_count) {
      Serial.print("[STATUS] Connected clients: ");
      Serial.println(client_count);
      
      if (client_count > 0 && last_client_count == 0) {
        Serial.println("[STATUS] Client connected! Web interface should be accessible.");
      } else if (client_count == 0 && last_client_count > 0) {
        Serial.println("[STATUS] All clients disconnected.");
      }
      
      last_client_count = client_count;
    }
  }
  
  // Main operation logic
  if (deauth_type == DEAUTH_TYPE_ALL) {
    // Channel hopping mode for deauth-all attack
    if (curr_channel > CHANNEL_MAX) curr_channel = 1;
    esp_wifi_set_channel(curr_channel, WIFI_SECOND_CHAN_NONE);
    curr_channel++;
    delay(10);
  } else {
    // Normal mode - handle web requests
    web_interface_handle_client();
  }
}
