
#include "WiFi.h"

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");
}

void loop()
{
  Serial.println("scan start");

  int n = WiFi.scanNetworks(); //取得WiFi基地台
  Serial.println("掃描完成");
  if (n == 0) {
    Serial.println("沒有發現任何網路");
  } else {
    Serial.print(n);
    Serial.println(" 個網路被發現");

    // 將網路依 RSSI 排序
    int indices[n];
    for (int i = 0; i < n; ++i) {
      indices[i] = i;
    }
    for (int i = 0; i < n - 1; ++i) {
      for (int j = i + 1; j < n; ++j) {
        if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
          // 交換位置
          int tmp = indices[i];
          indices[i] = indices[j];
          indices[j] = tmp;
        }
      }
    }

    // 依序列印排序後的網路資訊
    for (int i = 0; i < n; ++i) {
      int j = indices[i];
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(j));//Wifi 的SSID名稱
      Serial.print(" (");
      Serial.print(WiFi.RSSI(j));//Wifi 的訊號為多少dBm
      Serial.print(")");
      Serial.println((WiFi.encryptionType(j) == WIFI_AUTH_OPEN) ? "開放" : "加密");//Wifi 的加密模式
      delay(10);
    }
  }
  Serial.println("");
  // Wait a bit before scanning again
  delay(5000);
}
