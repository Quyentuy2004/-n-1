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
const char html1[] PROGMEM = R"html(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 IoT - Giám sát Nhiệt độ và Độ ẩm</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
            color: #333;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #2c3e50;
            text-align: center;
            margin-bottom: 30px;
        }
        .dashboard {
            display: flex;
            flex-wrap: wrap;
            gap: 20px;
            justify-content: center;
        }
        .card {
            background-color: #fff;
            border-radius: 8px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            transition: transform 0.3s ease;
            flex: 1;
            min-width: 250px;
        }
        .card:hover {
            transform: translateY(-5px);
        }
        .card-title {
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 15px;
            color: #3498db;
            text-align: center;
        }
        .reading {
            font-size: 36px;
            font-weight: bold;
            text-align: center;
            margin: 20px 0;
        }
        .temperature {
            color: #e74c3c;
        }
        .humidity {
            color: #3498db;
        }
        .unit {
            font-size: 20px;
        }
        .simulated {
            text-align: center;
            margin-top: 10px;
            color: #7f8c8d;
            font-style: italic;
        }
        .sensor-icon {
            text-align: center;
            font-size: 24px;
            margin-bottom: 10px;
        }
        .section {
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #eee;
        }
        .description {
            line-height: 1.6;
            margin-bottom: 20px;
        }
        .code-section {
            background-color: #f9f9f9;
            border: 1px solid #ddd;
            border-radius: 5px;
            padding: 15px;
            margin: 20px 0;
            overflow-x: auto;
        }
        pre {
            margin: 0;
            white-space: pre-wrap;
        }
        code {
            font-family: monospace;
            color: #333;
        }
        .footer {
            text-align: center;
            margin-top: 30px;
            color: #7f8c8d;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 IoT - Giám sát Nhiệt độ và Độ ẩm</h1>
        
        <div class="dashboard">
            <div class="card">
                <div class="sensor-icon">🌡️</div>
                <div class="card-title">Nhiệt độ</div>
                <div class="reading temperature" id="temperature">26.5<span class="unit">°C</span></div>
            </div>
            
            <div class="card">
                <div class="sensor-icon">💧</div>
                <div class="card-title">Độ ẩm</div>
                <div class="reading humidity" id="humidity">68.2<span class="unit">%</span></div>
            </div>
        </div>
        
        <div class="simulated">
            Đây là mô phỏng giao diện - dữ liệu thật từ ESP32 sẽ được hiển thị khi kết nối với thiết bị.
        </div>
        
        <div class="section">
            <h2>Mô tả dự án</h2>
            <div class="description">
                <p>Dự án này sử dụng ESP32 để tạo một thiết bị IoT đo nhiệt độ và độ ẩm. Thiết bị có các tính năng:</p>
                <ul>
                    <li>Cấu hình WiFi qua giao diện web</li>
                    <li>Đo nhiệt độ và độ ẩm sử dụng cảm biến DHT22/DHT11</li>
                    <li>Hiển thị dữ liệu theo thời gian thực trên trang web</li>
                    <li>Lưu thông tin WiFi vào EEPROM để duy trì kết nối sau khi khởi động lại</li>
                    <li>LED trạng thái cho biết chế độ hoạt động của thiết bị</li>
                </ul>
            </div>
        </div>
        
        <div class="section">
            <h2>Chức năng chính</h2>
            <div class="code-section">
                <pre><code>
// Chế độ Access Point
- Khi khởi động lần đầu, ESP32 sẽ tạo một AP có tên "ESP32-xx"
- Kết nối đến AP này để cấu hình Wi-Fi nhà bạn
- Truy cập 192.168.4.1 để nhập SSID và mật khẩu Wi-Fi

// Chế độ đo và hiển thị dữ liệu
- Sau khi kết nối Wi-Fi thành công, ESP32 bắt đầu đọc cảm biến
- Dữ liệu nhiệt độ và độ ẩm được cập nhật mỗi 2 giây
- Truy cập IP của ESP32 trên cổng 81 để xem dữ liệu

// Khôi phục cài đặt
- Nhấn giữ nút RESET trong 5 giây để xóa thông tin Wi-Fi
- Thiết bị sẽ khởi động lại ở chế độ Access Point
                </code></pre>
            </div>
        </div>
        
        <div class="footer">
            &copy; 2025 ESP32 IoT - Thiết bị giám sát nhiệt độ và độ ẩm
        </div>
    </div>
    
    <script>
        // Mô phỏng dữ liệu cho demo
        function simulateData() {
            const tempBase = 25 + (Math.random() * 6) - 3;  // 22-28°C
            const humBase = 65 + (Math.random() * 15) - 5;  // 60-75%
            
            document.getElementById('temperature').innerHTML = 
                tempBase.toFixed(1) + '<span class="unit">°C</span>';
            document.getElementById('humidity').innerHTML = 
                humBase.toFixed(1) + '<span class="unit">%</span>';
        }
        
        // Cập nhật dữ liệu mỗi 5 giây
        setInterval(simulateData, 5000);
        
        // Khởi tạo dữ liệu ban đầu sau khi trang tải xong
        document.addEventListener('DOMContentLoaded', simulateData);
    </script>
</body>
</html>


)html";
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
void handleRoot() {
    webServer.send(200, "text/html", "<h1>Xin chào! Đây là ESP32 Web Server</h1>");
}
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
  if (WiFi.status() == WL_CONNECTED) {  
        webServer.on("/",[]{
    webServer.send(200, "text/html", html1);
  });
        delay(2000);
        Serial.println("Web server started");
    }
    else {
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
    
   } else {
    webServer.send(400, "text/plain", "Missing parameters");
   }
  });}
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
