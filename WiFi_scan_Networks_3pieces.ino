#include "WiFi.h"

void setup()
{
    Serial.begin(115200);

    // 設置WiFi為station模式，如果之前已連接到AP，則斷開連接
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("設置完成");
}

void loop()
{
    Serial.println("開始掃描");

    // WiFi.scanNetworks會返回找到的網絡數量
    int n = WiFi.scanNetworks();
    Serial.println("掃描完成");
    if (n == 0) {
        Serial.println("沒有找到任何網絡");
    } else {
        Serial.print(n);
        Serial.println(" 個網絡已找到");
        // 將掃描到的網絡按信號強度由高到低排序，並選擇信號最強的前三個網絡
        for (int i = 0; i < 3; ++i) {
            // 對於每個找到的網絡，打印其SSID和RSSI
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));//Wifi 的SSID名稱
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));//Wifi 的訊號為多少dBm
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" 開放":"加密");//Wifi 的加密模式
            delay(10);
        }
    }
    Serial.println("");

    // 等待一段時間再進行下一次掃描
    delay(5000);
}
