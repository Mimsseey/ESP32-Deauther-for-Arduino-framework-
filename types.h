#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

// IEEE 802.11 Management Frame - Deauthentication Frame
// Based on IEEE 802.11-2016 specification 

typedef struct __attribute__((packed)) {
  // Frame Control Field (2 bytes)
  struct {
    uint8_t protocol_version : 2;  // Always 0
    uint8_t type : 2;              // Management frame = 00b
    uint8_t subtype : 4;           // Deauth = 1100b (0xC)
    uint8_t to_ds : 1;             // To Distribution System
    uint8_t from_ds : 1;           // From Distribution System
    uint8_t more_frag : 1;         // More fragments
    uint8_t retry : 1;             // Retry flag
    uint8_t pwr_mgmt : 1;          // Power management
    uint8_t more_data : 1;         // More data
    uint8_t protected_frame : 1;   // Protected frame (was WEP in previous code)
    uint8_t order : 1;             // Order flag
  } frame_control;
  
  // Duration/ID (2 bytes)
  uint16_t duration;
  
  // Address 1: Destination Address (Receiver Address) - 6 bytes
  uint8_t destination_addr[6];
  
  // Address 2: Source Address (Transmitter Address) - 6 bytes  
  uint8_t source_addr[6];
  
  // Address 3: BSSID (Basic Service Set Identifier) - 6 bytes
  uint8_t bssid[6];
  
  // Sequence Control (2 bytes)
  struct {
    uint16_t fragment_number : 4;
    uint16_t sequence_number : 12;
  } sequence_control;
  
  // Frame Body: Reason Code (2 bytes)
  uint16_t reason_code;
  
  // Note: FCS (Frame Check Sequence) is added by hardware
} deauth_frame_t;


// Alternative: Simpler version with raw bytes (current implementation style)
typedef struct __attribute__((packed)) {
  uint8_t frame_control[2];     // 0xC0, 0x00 for deauth frame
  uint8_t duration[2];          // Duration/ID field
  uint8_t destination[6];       // Address 1: Destination (station to deauth)
  uint8_t source[6];            // Address 2: Source (AP's BSSID)
  uint8_t bssid[6];             // Address 3: BSSID (AP's BSSID)
  uint8_t sequence[2];          // Sequence control
  uint16_t reason_code;         // Reason for deauthentication
} deauth_frame_simple_t;


// IEEE 802.11 Reason Codes (as per 802.11-2016 Table 9-45)
enum DeauthReasonCode : uint16_t {
  REASON_RESERVED = 0,
  REASON_UNSPECIFIED = 1,
  REASON_PREV_AUTH_NOT_VALID = 2,
  REASON_DEAUTH_LEAVING = 3,
  REASON_DISASSOC_DUE_TO_INACTIVITY = 4,
  REASON_DISASSOC_AP_BUSY = 5,
  REASON_CLASS2_FRAME_FROM_NONAUTH_STA = 6,
  REASON_CLASS3_FRAME_FROM_NONASSOC_STA = 7,
  REASON_DISASSOC_STA_HAS_LEFT = 8,
  REASON_STA_REQ_ASSOC_WITHOUT_AUTH = 9,
  REASON_PWR_CAPABILITY_NOT_VALID = 10,
  REASON_SUPPORTED_CHANNEL_NOT_VALID = 11,
  REASON_BSS_TRANSITION_DISASSOC = 12,
  REASON_INVALID_IE = 13,
  REASON_MIC_FAILURE = 14,
  REASON_4WAY_HANDSHAKE_TIMEOUT = 15,
  REASON_GROUP_KEY_UPDATE_TIMEOUT = 16,
  REASON_IE_IN_4WAY_DIFFERS = 17,
  REASON_GROUP_CIPHER_NOT_VALID = 18,
  REASON_PAIRWISE_CIPHER_NOT_VALID = 19,
  REASON_AKMP_NOT_VALID = 20,
  REASON_UNSUPPORTED_RSNE_VERSION = 21,
  REASON_INVALID_RSNE_CAPABILITIES = 22,
  REASON_IEEE_802_1X_AUTH_FAILED = 23,
  REASON_CIPHER_SUITE_REJECTED = 24
};


// MAC header structure for packet sniffing
typedef struct __attribute__((packed)) {
  uint16_t frame_ctrl;
  uint16_t duration;
  uint8_t dest[6];
  uint8_t src[6];
  uint8_t bssid[6];
  uint16_t sequence_ctrl;
  uint8_t addr4[6];
} mac_hdr_t;


// WiFi packet structure
typedef struct {
  mac_hdr_t hdr;
  uint8_t payload[0];
} wifi_packet_t;


// Promiscuous mode filter
const wifi_promiscuous_filter_t filt = {
  .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA
};


// Helper function to initialize deauth frame
inline void init_deauth_frame(deauth_frame_simple_t* frame, 
                              const uint8_t* dest, 
                              const uint8_t* src, 
                              const uint8_t* bssid_addr,
                              uint16_t reason) {
  // Frame Control: Type = Management (00), Subtype = Deauth (1100)
  // This gives us: 0xC0 in first byte, 0x00 in second byte
  frame->frame_control[0] = 0xC0;
  frame->frame_control[1] = 0x00;
  
  // Duration/ID - typically set to 0 or a small value
  frame->duration[0] = 0x00;
  frame->duration[1] = 0x00;
  
  // Copy addresses
  memcpy(frame->destination, dest, 6);
  memcpy(frame->source, src, 6);
  memcpy(frame->bssid, bssid_addr, 6);
  
  // Sequence control - can be managed by hardware or set manually
  frame->sequence[0] = 0x00;
  frame->sequence[1] = 0x00;
  
  // Reason code (little-endian)
  frame->reason_code = reason;
}


// Explanation of Frame Control Field bits:
// Byte 0 (bits 0-7):
//   Protocol Version: 00 (bits 0-1)
//   Type: 00 (bits 2-3) = Management
//   Subtype: 1100 (bits 4-7) = Deauthentication (0xC)
//   Result: 11000000 = 0xC0
//
// Byte 1 (bits 8-15):
//   To DS: 0 (bit 8)
//   From DS: 0 (bit 9)
//   More Fragments: 0 (bit 10)
//   Retry: 0 (bit 11)
//   Power Management: 0 (bit 12)
//   More Data: 0 (bit 13)
//   Protected Frame: 0 (bit 14)
//   Order: 0 (bit 15)
//   Result: 00000000 = 0x00

#endif // TYPES_H
