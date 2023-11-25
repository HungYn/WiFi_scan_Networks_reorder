#include <SPI.h>
#include <JPEGDecoder.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <SPIFFS.h>

// WPA/WPA2 SSID and password
String ssid = "";
String password = "";
const char* filepath = "/config.txt";
unsigned long WifiConnectingTimeout; //Wifi連線逾時時間
bool portalRunning = 1;     // 是否執行中，預設「是」。

#define FRAME_LENGTH 7  //7字節
#define SerialBT Serial
#define HMI_DATA_BUFF_SIZE  64
char    szHMIData[HMI_DATA_BUFF_SIZE];
#define LED_PIN     2   // 定義LED腳位


///-----------按鈕--------------------
#include <Button.h> //https://github.com/madleech/Button
Button buttonA(0);  //按鈕按下開始運作


//----------------------------- openweathermap----------------------------------
//open weather map api key
String apiKey = "5133bb2e0a6cbb34d2f9b9304e65cfad";
//the city you want the weather for
String location = "Taipei,TW";
char server[] = "api.openweathermap.org";
String weatherFlag;
unsigned long weatherTimeoutStart;
int weatherTimeoutCounter = 0;
String weatherStr, weatherStr1, weatherStr2, weatherPic , weatherData , nowTemp, maxTemp, minTemp , humidity , cityStr;
int getDataFlag;
int weatherKind;
char dayType = '1', dayTypeR = '1';

String locationStr[20] = { "Taipei,TW", "Keelung,TW", "Tainan,TW", "Kaohsiung,TW", "Taoyuan,TW", "Hsinchu,TW", "Taichung,TW", "Miaoli,TW", "Chang-hua,TW", "Nantou,TW", "Chiayi,TW", "HengChun,TW", "Taitung,TW", "Hualien,TW", "Yilan,TW", "Magong,TW" , "Jincheng,TW", "Nangan,TW", "Zhongliao,TW", "Yuren,TW"};
String locationTemp[20] = { "Taipei", "Keelung", "Tainan", "Kaohsiung", "Taoyuan", "Hsinchu", "Taichung" , "Miaoli", "Chang-hua", "Nantou", "Chiayi", "Hengchun", "Taitung", "Hualien", "Yilan" , "Magong", "Jincheng", "Nangan", "Zhongliao", "Yuren"};
String locationCTemp[20] = {"台北", "基隆", "台南", "高雄", "桃園", "新竹", "台中", "苗栗", "彰化", "南投", "嘉義", "屏東", "台東", "花蓮", "宜蘭" , "澎湖", "金門", "馬祖", "綠島", "蘭嶼"};
int locNum = 0 ;
int thermometer_coordinate_X = 20;
int thermometer_coordinate_Y = 100;

char str[100];


WiFiClient wifiClient;

void setup() {
  Serial.begin(115200);
  SerialBT.begin(115200);
  buttonA.begin();
  SPIFFS.begin(true);
  pinMode(2, OUTPUT);  // Initialize the BUILTIN_LED pin as an output

  WifiConn();

  //  清空串口緩存
  Serial.flush();
  while (Serial.read() >= 0) {}
}

void loop() {
  // put your main code here, to run repeatedly:


  if (buttonA.pressed()) { //設定選單

    getWeather();
    Serial.print("發送天氣預報訊息");
    Serial.println(locationCTemp[locNum]);

    locNum = locNum + 1;

    if (locNum > 19) {
      locNum = 0;
    }
  }

  int nHMIDataLen = handleHmiSerial(&SerialBT);

  if (nHMIDataLen > 0)
  {
    //Serial.println("nHMIDataLen=="+String(nHMIDataLen));
    if (szHMIData[0] == 0x65 && szHMIData[4] == 0xff && szHMIData[5] == 0xff && szHMIData[6] == 0xff)
    {
      if (szHMIData[1] == 1)
      {
        digitalWrite(LED_PIN, HIGH);
        locNum = szHMIData[2];
        getWeather();
      }
      else if (szHMIData[1] == 0)
      {
        digitalWrite(LED_PIN, LOW);
        locNum = szHMIData[2];
        getWeather();
      }
    }

    //設定帳密及重新連網
    if (szHMIData[0] == 0x66 && szHMIData[4] == 0xff && szHMIData[5] == 0xff && szHMIData[6] == 0xff)
    {
      //設定帳密
      if (szHMIData[2] == 1)
        modifyCredentials();
      //重新連網
      if (szHMIData[2] == 2)
        WifiConn();
      //重新下拉SSID清單
      if (szHMIData[2] == 3)
        HMI_ScanNetwork() ;
    }
    //查詢網路是否斷線
    if (WiFi.status() != WL_CONNECTED) {
      fnSetHmisysVal("sys1", 1);
    } else {
      fnSetHmisysVal("sys1", 0);
    }
  }

  //  while (Serial.available() >= FRAME_LENGTH) {
  //    unsigned char ubuffer[FRAME_LENGTH];
  //    //從串口緩沖讀取1個字節但不刪除
  //    unsigned char frame_header = Serial.peek();
  //    //当读取到的是0x55个字节时
  //    if (frame_header == 0x65) {
  //      //從串口緩沖區讀取7字節
  //      Serial.readBytes(ubuffer, FRAME_LENGTH);
  //      if (ubuffer[4] == 0xff && ubuffer[5] == 0xff && ubuffer[6] == 0xff) {
  //
  //        //        if (ubuffer[1] == 0x00 && ubuffer[2] == 0x07 && ubuffer[3] == 0x01) {
  //        dayType = ubuffer[2];
  //        sprintf(str, "t0.txt=\"%d\"" , dayType);
  //        Serial.print(str);
  //        SendEnd();
  //        Led_No_Off();
  //
  //        locNum = ubuffer[2];
  //        getWeather();
  //
  //        //        }
  //        //        if (ubuffer[1] == 0x00 && ubuffer[2] == 0x08 && ubuffer[3] == 0x01) {
  //        //          Serial2.print("t0.txt=\"停止\"\xff\xff\xff");
  //        //        }
  //      }
  //    } else {
  //      //  清空串口緩存
  //      Serial.flush();
  //      while (Serial.read() >= 0) {}
  //    }
  //  }

}

time_t getWeather() {

  String jsonTmp ;
  wifiClient.stop();
  //  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (wifiClient.connect(server, 80)) {
    //    Serial.println("connected to server");
    // Make a HTTP request:
    wifiClient.print("GET /data/2.5/weather?");
    wifiClient.print("q=" + locationStr[locNum]); //
    wifiClient.print("&appid=" + apiKey);
    wifiClient.print("&lang=zh_tw&cnt=3");
    wifiClient.println("&units=metric");
    wifiClient.println("Host: api.openweathermap.org");
    wifiClient.println("Connection: close");
    wifiClient.println();
    wifiClient.flush();
  } else {
    Serial.println("unable to connect");
  }

  delay(500);
  while (wifiClient.available())
  {
    DynamicJsonDocument doc(5000);

    String jsonData = wifiClient.readStringUntil('\r');  //逐列讀取
    //    Serial.println(jsonData);
    DeserializationError error = deserializeJson(doc, jsonData);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      //return;
    }
    //JsonArray array = doc.as<JsonArray>();

    String Str1 = doc["main"]["temp"];
    String Str2 = doc["main"]["temp_min"];
    String Str3 = doc["main"]["temp_max"];
    String Str4 = doc["main"]["humidity"];
    String Str5 = doc["weather"][0]["main"];
    String Str51 = doc["weather"][0]["description"];
    String Str52 = doc["weather"][0]["icon"];
    String Str6 = doc["name"];

    nowTemp = Str1;
    minTemp = Str2;
    maxTemp = Str3;
    humidity = Str4;
    weatherStr = Str5;
    weatherStr1 = Str51;
    weatherStr2 = Str52;
    cityStr = Str6;

    //    fnSetHmiTxt("t0", locationCTemp[locNum].c_str());
    fnSetHmiTxt("cityStr", cityStr.c_str());//城市
    fnSetHmiVal("z0", nowTemp.toInt() * 3);//目前溫度
    fnSetHmiTxt("t0", nowTemp.c_str());
    fnSetHmiVal("z1", minTemp.toInt() * 3);//最低溫度
    fnSetHmiTxt("t1", minTemp.c_str());
    fnSetHmiVal("z2", maxTemp.toInt() * 3);//最高溫度
    fnSetHmiTxt("t2", maxTemp.c_str());
    fnSetHmiVal("j0", humidity.toInt());//濕度
    fnSetHmiTxt("t3", humidity.c_str());
    fnSetHmiTxt("weatherStr1", weatherStr1.c_str());//天氣描述
    //    Serial.print("目前溫度: ");
    //    Serial.println(nowTemp);
    //    Serial.print("最低溫度: ");
    //    Serial.println(minTemp);  //顯示溫度值
    //    Serial.print("最高溫度: ");
    //    Serial.println(maxTemp);  //顯示溫度值
    //    Serial.print("濕度: ");
    //    Serial.println(humidity);
    //    Serial.print("天氣狀況: ");
    //    Serial.println(weatherStr);
    //    Serial.print("天氣描述: ");
    //    Serial.println(weatherStr1);
    //    Serial.print("天氣圖: ");
    //    Serial.println(weatherStr2);
    //    Serial.print("城市: ");
    //    Serial.println(cityStr);
    //    weatherData = "\n城市:" + cityStr + "\n目前溫度:" + nowTemp + "°C" + "\n最高溫度:" + maxTemp + "°C" + "\n最低溫度:" + minTemp +  "°C" + "\n濕度:" + humidity + "%";
    //    Serial.println(weatherData);

  }
}

int WifiConn()
{

  if (!SPIFFS.exists(filepath)) {
    Serial.println("配置文件不存在。創造...");
    writeFile(filepath, "");
  }

  readConfig(ssid, password);

  WiFi.disconnect();//断开连接
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
  //  listDir(SPIFFS, "/", 0); //列出檔案
}


// 接收來自 HMI 的資料
//
int handleHmiSerial(Stream* ser)
{
  int   nHMIDataLen = 0;

  memset(szHMIData, 0, sizeof(char)*HMI_DATA_BUFF_SIZE);
  if (ser->available()) {
    while (ser->available()) {
      char cc = ser->read();

      //Serial.println(cc, HEX);
      if (nHMIDataLen >= HMI_DATA_BUFF_SIZE) {
        Serial.println("Error : out of range");
        nHMIDataLen = 0;
        return 0;
      }
      szHMIData[nHMIDataLen++] = cc;
      //Serial.println(nHMIDataLen);
      if (checkEnd(szHMIData, nHMIDataLen)) {
        return nHMIDataLen;
      }
      delay(15);
    }
  }
  return 0;
}

bool checkEnd(char data[], int len)
{
  //Serial.printf("len=%d\n", len);
  if (len < 3)
    return false;

  int ei = len - 1;
  //Serial.printf("data[%d]=%02x\n", ei, data[ei]);
  if (data[ei--] != 0xFF)
    return false;

  //Serial.printf("data[%d]=%02x\n", ei, data[ei]);
  if (data[ei--] != 0xFF)
    return false;

  //Serial.printf("data[%d]=%02x\n", ei, data[ei]);
  if (data[ei] != 0xFF)
    return false;

  return true;
}
void fnSetHmiPage(int nPageID)
{
  char buff[64];
  sprintf(buff, "page %d", nPageID);
  SerialBT.print(buff);
  SerialBT.write(0xff);
  SerialBT.write(0xff);
  SerialBT.write(0xff);
  Led_No_Off();
}

void fnSetHmiTxt(char* ctl, const char* txt)
{
  char buff[32];
  sprintf(buff, "%s.txt=\"%s\"", ctl, txt);
  SerialBT.print(buff);
  USART_Serialcommunication();
  Led_No_Off();
}

void fnSetHmi_Page_Txt(char* page, char* ctl, const char* txt)
{
  char buff[32];
  sprintf(buff, "%s.%s.txt=\"%s\"", page, ctl, txt);
  SerialBT.print(buff);
  USART_Serialcommunication();
  Led_No_Off();
}

void fnSetHmiVal(const char* ctl, int val)
{
  char buff[32];
  sprintf(buff, "%s.val=%d", ctl, val);
  SerialBT.print(buff);
  USART_Serialcommunication();
  Led_No_Off();
}
//傳送SYS0變數
void fnSetHmisysVal(const char* ctl, int val)
{
  char buff[32];
  sprintf(buff, "%s=%d", ctl, val);
  SerialBT.print(buff);
  USART_Serialcommunication();
  Led_No_Off();
}
//傳送下拉PATH變數
void fnSetHmiPath(const char* ctl, const char* Path)
{
  char buff[32];
  sprintf(buff, "%s.path=\"%s\"", ctl, Path);
  SerialBT.print(buff);
  USART_Serialcommunication();
  Led_No_Off();
}
//按鍵顯示與否
void fnSetHmiVis(const char* ctl, int val)
{
  char buff[32];
  sprintf(buff, "vis %s,%d", ctl, val);
  SerialBT.print(buff);
  USART_Serialcommunication();
  Led_No_Off();
}

void USART_Serialcommunication() {
  SerialBT.write(0xff);
  SerialBT.write(0xff);
  SerialBT.write(0xff);
}

void Led_No_Off() {
  digitalWrite(LED_PIN, HIGH);  // sets the LED on
  delay(200);             // waits for 200mS
  digitalWrite(LED_PIN, LOW);   // sets the LED off
  delay(200);
}
// SPIFFS Wifi 帳密設定

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
void modifyCredentials() {
  Serial.println("輸入新憑據：");

  String newSsid = "";
  String newPassword = "";

  Serial.print("SSID: ");
  while (SerialBT.available() == 0) {
    delay(100);
  }
  newSsid = SerialBT.readStringUntil('\n');
  Serial.println(newSsid);

  Serial.print("Password: ");
  while (SerialBT.available() == 0) {
    delay(100);
  }
  newPassword = SerialBT.readStringUntil('\n');
  Serial.println(newPassword);

  writeConfig(newSsid, newPassword);
  Serial.println("憑據已更新!");

  ESP.restart();// 檢查是否需要重啟
}

// HMI 掃描網路的數量 & 列印出 SSID and RSSI
void HMI_ScanNetwork() {
  String SSID_Name = "";
  WiFi.disconnect();//断开连接
  
  // WiFi.scanNetworks將返回找到的網路數量
  int n = WiFi.scanNetworks();  // 掃描網路的數量
  if (n == 0) {
    //    HMI_SerialInstructionTxt("WiFiSet", "cb0", "txt", "No Networks Found", 4294967295);  // 沒有找到任何網路
    fnSetHmi_Page_Txt("page1", "cb0", "找不到SSID");
  } else {
    //    HMI_SerialInstructionValue("WiFiSet", "n0", "val", n);  //找到n個網路
    for (int i = 0; i < n; ++i) {                           // 找到每個網路，並列印出 SSID and RSSI
      delay(10);
      SSID_Name += WiFi.SSID(i) + "\r\n";
    }
    fnSetHmiPath( "cb0", SSID_Name.c_str()); //掃描網路，將每個網路列在HMI網路設定的"下拉框"
  }
  delay(10);  // 稍等0.1秒再掃描
}
