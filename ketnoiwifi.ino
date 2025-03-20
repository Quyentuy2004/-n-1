#include <EEPROM.h> //Tên wifi và mật khẩu lưu vào ô nhớ 0->96
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h> //Thêm thư viện web server
WebServer webServer(80); //Khởi tạo đối tượng webServer port 80
#include <Ticker.h>
Ticker blinker;

String ssid;
String password;
#define ledPin 2
#define btnPin 0
unsigned long lastTimePress = millis();
#define PUSHTIME 5000
int wifiMode; // 0: Chế độ cấu hình, 1: Chế độ kết nối, 2: Mất wifi
unsigned long blinkTime = millis();

//Tạo biến chứa mã nguồn trang web HTML để hiển thị trên trình duyệt
const char html[] PROGMEM = R"html( 
  <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>SETTING WIFI INFORMATION</title>
        <style type="text/css">
          body{display: flex;justify-content: center;align-items: center;}
          button{width: 135px;height: 40px;margin-top: 10px;border-radius: 5px}
          label, span{font-size: 25px;}
          input{margin-bottom: 10px;width:275px;height: 30px;font-size: 17px;}
          select{margin-bottom: 10px;width: 280px;height: 30px;font-size: 17px;}
        </style>
    </head>
    <body>
      <div>
        <h3 style="text-align: center;">SETTING WIFI INFORMATION</h3>
        <p id="info" style="text-align: center;">Scanning wifi network...!</p>
        <label>Wifi name:</label><br>
        <select id="ssid">
          <option>Choose wifi name!</option>
        </select><br>
        <label>Password:</label><br>
        <input id="password" type="text"><br>

        <button onclick="saveWifi()" style="background-color: cyan;margin-right: 10px">SAVE</button>
        <button onclick="reStart()" style="background-color: pink;">RESTART</button>
      </div>
        <script type="text/javascript">
          window.onload=function(){
            scanWifi();
          }
          var xhttp = new XMLHttpRequest();
          
          function scanWifi(){
            xhttp.onreadystatechange = function(){
              if (xhttp.readyState == 4) {
                console.log ("qua buoc 1");
                if (xhttp.status == 200) {
                  console.log("Thành công buoc 2:", xhttp.responseText);
                  data = xhttp.responseText;
                  document.getElementById("info").innerHTML = "WiFi scan completed!";
                  var obj = JSON.parse(data);
                  var select = document.getElementById("ssid");
                  for(var i=0; i<obj.length;++i){
                   select[select.length] = new Option(obj[i],obj[i]);
            
                  }
                }
                else if (xhttp.status == 404) {
                  console.error("Lỗi 404: Tài nguyên không tìm thấy!");
                  alert("Lỗi 404: Trang không tồn tại. Vui lòng kiểm tra lại URL.");
                }
                else {
                  console.error("Lỗi khác: " + xhttp.status);
                }
              }
            }
            xhttp.open("GET","/scanWifi",true);
            xhttp.send();
          }
          function saveWifi(){
            ssid = document.getElementById("ssid").value;
            pass = document.getElementById("password").value;
            xhttp.onreadystatechange = function(){
              if(xhttp.readyState==4&&xhttp.status==200){
                data = xhttp.responseText;
                alert(data);
              }
            }
            xhttp.open("GET","/saveWifi?ssid="+ssid+"&pass="+pass,true);
            xhttp.send();
          }
          function reStart(){
            xhttp.onreadystatechange = function(){
              if(xhttp.readyState==4&&xhttp.status==200){
                data = xhttp.responseText;
                alert(data);
              }
            }
            xhttp.open("GET","/reStart",true);
            xhttp.send();
          }
        </script>
    </body>
  </html>
)html";

void scanWiFiNetworks() {
  Serial.println("Scanning WiFi...");
  int numNetworks = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < numNetworks; i++) {
    if (i) json += ",";
    json += "\"" + WiFi.SSID(i) + "\"";
  }
  json += "]";
  Serial.println(json);
  webServer.send(200, "application/json", json);
}
void blinkLed(uint32_t t){
  if(millis()-blinkTime>t){
    digitalWrite(ledPin,!digitalRead(ledPin));
    blinkTime=millis();
  }
}

void ledControl(){
  if(digitalRead(btnPin)==LOW){
    if(millis()-lastTimePress<PUSHTIME){
      blinkLed(1000);
    }else{
      blinkLed(50);
    }
  }else{
    if(wifiMode==0){
      blinkLed(50);
    }else if(wifiMode==1){
      blinkLed(3000);
    }else if(wifiMode==2){
      blinkLed(300);
    }
  }
}
void WiFiEvent(WiFiEvent_t event) {
            Serial.print("Event:");
             Serial.println(event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("WiFi connected!");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("WiFi lost connection.");
            wifiMode=2;
            delay(5000);
      WiFi.begin(ssid, password);
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            break;
        default:
        Serial.print("No ");
            break;
    }
}

void setupWifi(){
  EEPROM.get(0, ssid);
  EEPROM.get(50, password);
  Serial.print("SSID từ EEPROM: ");
  Serial.println(ssid);
  Serial.print("Mật khẩu từ EEPROM: ");
  Serial.println(password);
  WiFi.onEvent(WiFiEvent);//chỉ cần gọi một lần trước khi kết nối WiFi, vì nó đăng ký lắng nghe sự kiện WiFi suốt quá trình chạy.
  if(ssid.length()>0){
    Serial.println("Connecting to wifi...!");
    WiFi.mode(WIFI_STA);
   
    WiFi.begin(ssid, password);
    Serial.print("Đang kết nối tới WiFi");
    while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    }
    Serial.println("\nĐa Hoan thanh ket noi WiFi!");
    WiFi.onEvent(WiFiEvent);
  
    
  }
  else{
    Serial.println("ESP32 wifi network created!");
    WiFi.mode(WIFI_AP);
    uint8_t macAddr[6];
    WiFi.softAPmacAddress(macAddr);
    String ssid_ap="ESP32-"+String(macAddr[4],HEX)+String(macAddr[5],HEX);
    ssid_ap.toUpperCase();
    WiFi.softAP(ssid_ap.c_str());
    Serial.println("Access point name:"+ssid_ap);
    Serial.println("Web server access address:"+WiFi.softAPIP().toString());
    wifiMode=0;
  }
}

void setupWebServer(){
  webServer.on("/",[]{
    webServer.send(200, "text/html", html);
  });
  webServer.on("/scanWifi", scanWiFiNetworks);
  
  webServer.on("/saveWifi", []() { 
   if (webServer.hasArg("ssid") && webServer.hasArg("pass")) {
    ssid = webServer.arg("ssid");
    password = webServer.arg("pass");
    
    EEPROM.put(0, ssid);
    EEPROM.put(50, password);
    EEPROM.commit();  // Lưu vào bộ nhớ thực tế

    webServer.send(200, "text/plain", "WiFi information saved! Restarting...");
    delay(2000);
    ESP.restart();
   } else {
    webServer.send(400, "text/plain", "Missing parameters");
   }
  });
  webServer.begin();
}

void checkButton(){
  if(digitalRead(btnPin)==LOW){
    Serial.println("Press and hold for 5 seconds to reset to default!");
    if(millis()-lastTimePress>PUSHTIME){
      for(int i=0; i<100;i++){
        EEPROM.write(i,0);
      }
      EEPROM.commit();
      Serial.println("EEPROM memory erased!");
      delay(2000);
      ESP.restart();
    }
    delay(1000);
  }else{
    lastTimePress=millis();
  }
}

class Config{
public:
  void begin(){
    pinMode(ledPin,OUTPUT);
    pinMode(btnPin,INPUT_PULLUP);
    blinker.attach_ms(50, ledControl);
    EEPROM.begin(100);
    setupWifi();
    if(wifiMode==0) setupWebServer();
  }
  void run(){
    checkButton();
    if(wifiMode==0)webServer.handleClient();
  }
} wifiConfig;
void setup() {
  Serial.begin(115200);
  wifiConfig.begin();
 
}
void loop() {
  wifiConfig.run();
  
}