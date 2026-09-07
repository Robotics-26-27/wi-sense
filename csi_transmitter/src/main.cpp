#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

namespace {
constexpr uint8_t wifiChannel = 11;
constexpr uint32_t sendFrequencyHz = 100;
const uint8_t deviceMac[] = {0x1a, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t broadcastMac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
}

void setup() {
    Serial.begin(921600);
    WiFi.mode(WIFI_STA);

    if (esp_wifi_set_mac(WIFI_IF_STA, deviceMac) != ESP_OK ||
        esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40) != ESP_OK ||
        esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_BELOW) != ESP_OK) {
        Serial.println("Wi-Fi setup failed");
        while (true) {
            delay(1000);
        }
    }

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed");
        while (true) {
            delay(1000);
        }
    }

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, broadcastMac, sizeof(broadcastMac));
    peer.channel = wifiChannel;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    if (esp_now_add_peer(&peer) != ESP_OK) {
        Serial.println("ESP-NOW peer setup failed");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("CSI transmitter ready on channel 11");
}

void loop() {
    static uint32_t sequence = 0;
    esp_now_send(broadcastMac, reinterpret_cast<const uint8_t *>(&sequence), sizeof(sequence));
    ++sequence;
    delay(1000 / sendFrequencyHz);
}
