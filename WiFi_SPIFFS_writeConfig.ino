#include <SPIFFS.h>
#include <WiFi.h>
#include <Button.h>  //https://github.com/madleech/Button
#define LED_PIN     2   // 定義LED腳位
Button buttonA(0);  //按鈕按下開始運作
byte meniu = 0; //選單

String ssid = "";
String password = "";
const char* filepath = "/config.txt";
unsigned long WifiConnectingTimeout; //Wifi連線逾時時間
bool portalRunning = 1;     // 是否執行中，預設「是」。

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(true);
  buttonA.begin();
  pinMode(LED_PIN, OUTPUT);

  if (!SPIFFS.exists(filepath)) {
    Serial.println("配置文件不存在。創造...");
    writeFile(filepath, "");
  }

  readConfig(ssid, password);


  WiFi.begin(ssid.c_str(), password.c_str());

  WifiConnectingTimeout = millis();
  Serial.print("嘗試連接到 SSID:");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if ((millis() - WifiConnectingTimeout) > 5000) { //等待 10秒
      Serial.println("連線失敗");
      portalRunning = 0;
      break;
    }
  }
  Serial.println((portalRunning == 1) ? "已連接到 WiFi！" : "");

}

void loop() {

  if (buttonA.pressed()) { //B按鈕按下後執行
    meniu = meniu + 1;

    if (meniu > 3) {
      meniu = 0;
    }

    if  (meniu == 0) {
      Led_No_Off() ;
      modifyCredentials();
    }
    if (meniu == 1) {
      Led_No_Off() ;
      listDir(SPIFFS, "/", 0);
    }
    if (meniu == 2) {
      readConfig(ssid, password);
    }
  }
}

void readConfig(String& ssid, String& password) {
  File configFile = SPIFFS.open(filepath, "r");
  if (configFile) {
    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      //startsWith檢查String是否以特定的子字符串開頭
      if (line.startsWith("ssid=")) {
        //第 5 個字符開始到該字符串(ssid=)共5字符
        ssid = line.substring(5);
      } else if (line.startsWith("password=")) {
        //第 9 個字符開始到該字符串(password=)共9字符
        password = line.substring(9);
      }
      Serial.println("存儲的憑據:");
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(password);
    }
    configFile.close();
  }
}

void writeConfig(String ssid, String password) {
  File configFile = SPIFFS.open(filepath, "w");
  if (configFile) {
    configFile.printf("ssid=%s\n", ssid.c_str());
    configFile.printf("password=%s\n", password.c_str());
    configFile.close();
  }
}

void writeFile(const char* path, const char* message) {
  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("無法打開文件進行寫入");
    return;
  }

  if (file.print(message)) {
    Serial.println("文件寫入");
  } else {
    Serial.println("寫入失敗");
  }

  file.close();
}

void modifyCredentials() {
  Serial.println("輸入新憑據：");

  String newSsid = "";
  String newPassword = "";

  Serial.print("SSID: ");
  while (Serial.available() == 0) {
    delay(100);
  }
  newSsid = Serial.readStringUntil('\n');
  Serial.println(newSsid);

  Serial.print("Password: ");
  while (Serial.available() == 0) {
    delay(100);
  }
  newPassword = Serial.readStringUntil('\n');
  Serial.println(newPassword);

  writeConfig(newSsid, newPassword);
  Serial.println("憑據已更新!");

  ESP.restart();// 檢查是否需要重啟
}

void Led_No_Off() {
  digitalWrite(LED_PIN, HIGH);  // sets the LED on
  delay(200);             // waits for 200mS
  digitalWrite(LED_PIN, LOW);   // sets the LED off
  delay(200);
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("目錄列表: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- 打開目錄失敗");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - 不是目錄");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
