#include "wifiConfic.h"


void setup() {
  Serial.begin(115200);
  wifiConfig.begin();
   // Cấu hình Web Server web 1
   
    
    
   
}
void loop() {
  wifiConfig.run();
  
}