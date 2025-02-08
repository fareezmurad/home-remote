#include "ESPNOW.h"

#define STATUS_INDICATOR 2
#define SELECT_BUTTON 32

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xCC, 0xDB, 0xA7, 0x2E, 0x0E, 0x14};

// Structure to send data
typedef struct struct_message {
  bool toggleSwitch[4];  // Control 4 LEDs
} struct_message;

struct_message switchData;

esp_now_peer_info_t peerInfo;

// Callback function when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // Copy the data into the struct message
  memcpy(&switchData, incomingData, sizeof(switchData));
}

// Callback function when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void printWiFiState() {
  if (WiFi.getMode() == WIFI_OFF) {
    digitalWrite(STATUS_INDICATOR, LOW);
    Serial.println("Wi-Fi is OFF");
  } else {
    digitalWrite(STATUS_INDICATOR, HIGH);
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

  esp_now_register_recv_cb(OnDataRecv);  // Register data received callback
  esp_now_register_send_cb(OnDataSent);  // Register data send callback

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

// Disable Wifi/ESPNOW to save power
void deInitESPNow() {
  if (esp_now_deinit() == ESP_OK) {
    Serial.println("ESP-NOW de-initialized");
    WiFi.mode(WIFI_OFF);
    printWiFiState();
  } else {
    Serial.println("Error de-initializing ESP-NOW");
  }
}

void dataUpdateOnStartup() {
  initESPNow();
  Serial.println("Pull switch state data from receiver");
  delay(500);
  deInitESPNow();
}

// Send data
void sendSwitchData(int switchIndex) {
  int buttonState = digitalRead(SELECT_BUTTON);

  if (buttonState == LOW) {
    // Toggle the corresponding switch state
    switchData.toggleSwitch[switchIndex] = !switchData.toggleSwitch[switchIndex];

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&switchData, sizeof(switchData));

    if (result == ESP_OK) {
      Serial.println("Sent with success");
      deInitESPNow();  // Turn off WiFi after successfully sending the data
    } else {
      Serial.println("Error sending the data");
    }
    delay(50);
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