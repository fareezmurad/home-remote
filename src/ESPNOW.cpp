#include "ESPNOW.h"

#define SELECT_BUTTON 32

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x10, 0x06, 0x1C, 0xB5, 0x43, 0xE0};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  bool toggleSwitch;
} struct_message;

// Create a struct_message called myData
struct_message switch1;
struct_message switch2;
struct_message switch3;
struct_message switch4;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
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

void sendSwitchData(struct_message &switchObj) {
  int buttonState = digitalRead(SELECT_BUTTON);

  if (buttonState == LOW) {
    // Set the toggleSwitch value
    switchObj.toggleSwitch = buttonState;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&switchObj, sizeof(switchObj));

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
  sendSwitchData(switch1);
}

void sendDataSwitch2() {
  initESPNow();
  sendSwitchData(switch2);
}

void sendDataSwitch3() {
  initESPNow();
  sendSwitchData(switch3);
}

void sendDataSwitch4() {
  initESPNow();
  sendSwitchData(switch3);
}