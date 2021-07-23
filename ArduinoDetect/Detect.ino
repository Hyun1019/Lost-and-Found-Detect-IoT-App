#include "ESP8266.h"
#include <SoftwareSerial.h>

#define SSID        "AndroidHotspot" 
#define PASSWORD    "password"

#define TRIG_r1 8
#define ECHO_r1 9 
#define TRIG_r2 10
#define ECHO_r2 11

SoftwareSerial mySerial(5, 4);
ESP8266 wifi(mySerial);


void setup(void)
{  
    Serial.begin(9600);
    Serial.print("setup begin\r\n");
    
    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());
      
    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP: ");
        Serial.println(wifi.getLocalIP().c_str());    
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.enableMUX()) {
        Serial.print("multiple ok\r\n");
    } else {
        Serial.print("multiple err\r\n");
    }
    
    if (wifi.startTCPServer(8090)) {
        Serial.print("start tcp server ok\r\n");
    } else {
        Serial.print("start tcp server err\r\n");
    }
    
    if (wifi.setTCPServerTimeout(360)) { 
        Serial.print("set tcp server timout 360 seconds\r\n");
    } else {
        Serial.print("set tcp server timout err\r\n");
    }
    
    Serial.print("setup end\r\n");

    pinMode(A0, INPUT);   //fsr
    pinMode(A1, INPUT);   //cds

    pinMode(6, INPUT);    //infrared 1
    pinMode(7, INPUT);    //infrared 2
    
    pinMode(TRIG_r1, OUTPUT);  //ultrasonic 1
    pinMode(ECHO_r1, INPUT);
    pinMode(TRIG_r2, OUTPUT);  //ultrasonic 2
    pinMode(ECHO_r2, INPUT);
    
    pinMode(LED_BUILTIN, OUTPUT);
}

 
void loop(void)
{
    uint8_t buffer[128] = {0};
    uint8_t mux_id;
    uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer), 100);

    if (len > 0){
      Serial.print("\nStatus:[");
      Serial.print(wifi.getIPStatus().c_str());
      Serial.println("]");
        
      Serial.print("Received from :");
      Serial.print(mux_id);
      Serial.print(" [");

      Serial.print("Command:");
      char command = (char)buffer[0];
      Serial.print(command);
      Serial.println("]");


      //ultrasonic sensor
      if (command == '0'){
        Serial.println("\n*Ultrasonic Start*");
        long duration_r1, distance_r1;
        long duration_r2, distance_r2;

        digitalWrite(TRIG_r1, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_r1, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_r1, LOW);
  
        duration_r1 = pulseIn (ECHO_r1, HIGH); 
        distance_r1 = duration_r1 * 17 / 1000;
  
        digitalWrite(TRIG_r2, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_r2, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_r2, LOW);

        duration_r2 = pulseIn (ECHO_r2, HIGH); 
        distance_r2 = duration_r2 * 17 / 1000;
  
        Serial.print("D1 : ");
        Serial.print(distance_r1);
        Serial.print(" Cm, ");

        Serial.print("D2 : ");
        Serial.print(distance_r2);
        Serial.println(" Cm");

        if (distance_r1 < 7 or distance_r2 < 7) {
          Serial.println("[Ultra] : Lost Detected!");
          digitalWrite(LED_BUILTIN, HIGH);
          sprintf(buffer, "Detected!\n");
          wifi.send(mux_id, buffer, strlen(buffer));
        }
        else {
          digitalWrite(LED_BUILTIN, LOW);
          sprintf(buffer, "Not Detected.\n");
          wifi.send(mux_id, buffer, strlen(buffer));
        }
      }


      //cds sensor   
      else if (command == '1'){
        Serial.println("\n*CDS Start*");
      
        int cds = analogRead(A1);
        Serial.println("cds : " + String(cds));
  
        if (cds < 500){   
          Serial.println("[CDS] : Lost Detected!");
          digitalWrite(LED_BUILTIN, HIGH);
          sprintf(buffer, "Detected!\n");
          wifi.send(mux_id, buffer, strlen(buffer));
        }
        else{
          digitalWrite(LED_BUILTIN, LOW);  
          sprintf(buffer, "Not Detected.\n");
          wifi.send(mux_id, buffer, strlen(buffer));    
        }
      }


      //fsr sensor
      else if (command == '2'){
        Serial.println("\n*FSR Start*");
      
        int fsrADC = analogRead(A0);
        Serial.println("fsrADC : " + String(fsrADC));
  
        if (fsrADC > 15){   
          Serial.println("[FSR] : Lost Detected!");
          digitalWrite(LED_BUILTIN, HIGH);
          sprintf(buffer, "Detected!\n");
          wifi.send(mux_id, buffer, strlen(buffer));
        }
        else{
          digitalWrite(LED_BUILTIN, LOW);  
          sprintf(buffer, "Not Detected.\n");
          wifi.send(mux_id, buffer, strlen(buffer));     
        }
      }


      //infrared sensor
      else if (command == '3'){
        Serial.println("\n*Infrared Start*");
        int s1 = digitalRead(6);
        int s2 = digitalRead(7);

        Serial.print("s1 = ");
        Serial.print(s1);
        Serial.print(", s2 = ");
        Serial.println(s2);
 
        if(s1 == 0 or s2 == 0){
          Serial.println("[Infrared] Lost Detected!");
          digitalWrite(LED_BUILTIN, HIGH);
          sprintf(buffer, "Detected!\n");
          wifi.send(mux_id, buffer, strlen(buffer));
        }
        else{
          digitalWrite(LED_BUILTIN, LOW);
          sprintf(buffer, "Not Detected.\n");
          wifi.send(mux_id, buffer, strlen(buffer));
        }
      }
  
      else{
        Serial.println("Disabled Command");
      }
   }                    
}
