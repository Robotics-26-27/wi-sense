#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

namespace {
constexpr uint8_t wifiChannel = 11;
const uint8_t receiverMac[] = {0x1b, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t transmitterMac[] = {0x1a, 0x00, 0x00, 0x00, 0x00, 0x00};
}

void onCsi(void *, wifi_csi_info_t *info) {
    if (info == nullptr || info->buf == nullptr || info->payload == nullptr) {
        return;
    }

    if (memcmp(info->mac, transmitterMac, sizeof(transmitterMac)) != 0) {
        return;
    }

    uint32_t sequence = 0;
    memcpy(&sequence, info->payload + 15, sizeof(sequence));
    Serial.printf("CSI_DATA,%lu,%02x:%02x:%02x:%02x:%02x:%02x,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\"[",
                  static_cast<unsigned long>(sequence), info->mac[0], info->mac[1],
                  info->mac[2], info->mac[3], info->mac[4], info->mac[5], info->rx_ctrl.rssi,
                  info->rx_ctrl.rate, info->rx_ctrl.sig_mode, info->rx_ctrl.mcs,
                  info->rx_ctrl.cwb, info->rx_ctrl.smoothing, info->rx_ctrl.not_sounding,
                  info->rx_ctrl.aggregation, info->rx_ctrl.stbc, info->rx_ctrl.fec_coding,
                  info->rx_ctrl.sgi, info->rx_ctrl.noise_floor, info->rx_ctrl.ampdu_cnt,
                  info->rx_ctrl.channel, info->rx_ctrl.secondary_channel);

    for (int i = 0; i < info->len; ++i) {
        if (i != 0) {
            Serial.print(',');
        }
        Serial.print(info->buf[i]);
    }
    Serial.println("]\"");
}

void setup() {
    Serial.begin(921600);
    WiFi.mode(WIFI_STA);

    if (esp_wifi_set_mac(WIFI_IF_STA, receiverMac) != ESP_OK ||
        esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40) != ESP_OK ||
        esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_BELOW) != ESP_OK ||
        esp_wifi_set_promiscuous(true) != ESP_OK) {
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

    wifi_csi_config_t csiConfig = {
        .lltf_en = true,
        .htltf_en = true,
        .stbc_htltf2_en = true,
        .ltf_merge_en = true,
        .channel_filter_en = true,
        .manu_scale = false,
        .shift = false,
    };
    if (esp_wifi_set_csi_config(&csiConfig) != ESP_OK ||
        esp_wifi_set_csi_rx_cb(onCsi, nullptr) != ESP_OK ||
        esp_wifi_set_csi(true) != ESP_OK) {
        Serial.println("CSI setup failed");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("type,id,mac,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_format,len,first_word,data");
    Serial.println("CSI receiver ready on channel 11");
}

void loop() {
    delay(1000);
}
