#include "ESPNOW.h"

#include <Preferences.h>

#define SELECT_BUTTON 32

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xCC, 0xDB, 0xA7, 0x2E, 0x0E, 0x14};

// Structure to send data
typedef struct struct_message {
  bool toggleSwitch[4];  // Control 4 LEDs
} struct_message;

struct_message switchData;

// Preferences for NVS storage
Preferences preferences;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void saveToggleStates() {
  preferences.begin("toggle-states", false);  // Open NVS
  for (int i = 0; i < 4; i++) {
    preferences.putBool((String("switch") + i).c_str(), switchData.toggleSwitch[i]);  // Convert String to const char*
  }
  preferences.end();  // Close NVS
}

void loadToggleStates() {
  preferences.begin("toggle-states", true);  // Open NVS in read-only mode
  for (int i = 0; i < 4; i++) {
    switchData.toggleSwitch[i] = preferences.getBool((String("switch") + i).c_str(), false);  // Convert String to const char*
  }
  preferences.end();  // Close NVS
}

void printWiFiState() {
  if (WiFi.getMode() == WIFI_OFF) {
    Serial.println("Wi-Fi is OFF");
  } else {
    Serial.println("Wi-Fi is ON");
  }
}

void initESPNow() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  printWiFiState();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void deInitESPNow() {
  if (esp_now_deinit() == ESP_OK) {
    Serial.println("ESP-NOW de-initialized");
    WiFi.mode(WIFI_OFF);
    printWiFiState();
  } else {
    Serial.println("Error de-initializing ESP-NOW");
  }
}

void sendSwitchData(int switchIndex) {
  int buttonState = digitalRead(SELECT_BUTTON);

  if (buttonState == LOW) {
    // Toggle the corresponding switch state
    switchData.toggleSwitch[switchIndex] = !switchData.toggleSwitch[switchIndex];

    // Save states to NVS
    saveToggleStates();

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&switchData, sizeof(switchData));

    if (result == ESP_OK) {
      Serial.println("Sent with success");
      deInitESPNow();
    } else {
      Serial.println("Error sending the data");
    }
    delay(100);
  }
}

void sendDataSwitch1() {
  initESPNow();
  sendSwitchData(0);
}

void sendDataSwitch2() {
  initESPNow();
  sendSwitchData(1);
}

void sendDataSwitch3() {
  initESPNow();
  sendSwitchData(2);
}

void sendDataSwitch4() {
  initESPNow();
  sendSwitchData(3);
}