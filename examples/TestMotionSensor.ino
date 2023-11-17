#include <ESP8266WiFi.h>
#include <MotionSensor.h>
#include <string.h>
#include <ezOutput.h> 
#include <SSVLongTime.h>
#include <TimeLib.h>

const int PIRSigPin     = D5; // GPIO14 input
const int BUZZER        = D3; // GPIO0  OK output avec pullup externe et active on LOW
const int BOARD_LED     = D4; // GPIO2 OK output mais avec pullup externe et active on LOW


ezOutput Buzzer(BUZZER);  // create ezOutput object that attach to pin BUZZER;
ezOutput Board_Led(BOARD_LED);  // create ezOutput object that attach to pin BOARD_LED;

bool Logger(char *text, bool SerialPrint);

MotionSensor PIR1((char *)"Bedroom", PIRSigPin, false, NULL);
MotionSensor PIR2((char *)"Garage", PIRSigPin, false, Logger);                  

MotionSensor *PIR;

// yyyy-MM-ddTHH:mm:ss
// 2018-11-14T11:20:37
void printDateTime(time_t t, char *buf)
{
    sprintf(buf, "%d-%.2d-%.2dT%.2d:%.2d:%.2d",
        year(t), month(t), day(t), hour(t), minute(t), second(t));
}

bool Logger(char *text, bool SerialPrint)
{
  char time_buf[50];
  char buf[300];
  bool ret = true;
  
  printDateTime(now(),time_buf);
  sprintf(buf,"%s %s ",time_buf, text);
  
  if(SerialPrint)
      Serial.println(buf);
  
  return ret;
}

void setup() {

   Serial.begin(115200);  
   while (!Serial)
   { /*wait*/
   }
    
   delay(500); // For serial to flush and connect

   PIR1.Setup(
            300,                   // sensor refresh rate in miliseconds
            4500,                  // sensor motion time period in miliseconds
            14);                   // sensor minimal triggering number

   PIR2.Setup(
            100,                   // sensor refresh rate in miliseconds
            500,                  // sensor motion time period in miliseconds
            5);                    // sensor minimal triggering number
            
    
   Board_Led.high(); 
   Buzzer.high(); 

   PIR = &PIR1;
   
   sprintf(buff, "Beginning for sensor: %s", PIR->GetName());
   Serial.println(buff);  

   Serial.print("UpTime: "); Serial.println( Sec_To_Age_Str( SSVLongTime.getUpTimeSec() ) );
}

void loop() {
  char buff[250];
  
  PIR->loop();
  Buzzer.loop();   
  Board_Led.loop();   
  
  if(PIR->ValidDetection())
  {
    sprintf(buff, "Detection happened, sensor: %s at %s", PIR->GetName(), Sec_To_Age_Str( SSVLongTime.getUpTimeSec()).c_str()); 
    Logger(buff, true); 
    // Logger((char *)PIR->GetLastJsonValidDetectionData().c_str(), true); 
    
    // Buzzer.blink(70, 70, 0, 4); // 70 milliseconds ON, 70 milliseconds OFF, start immidiately, blink 4 times (2 times OFF->ON, 2 times ON->OFF, interleavedly)  
    Board_Led.blink(70, 70, 0, 4); // 70 milliseconds ON, 70 milliseconds OFF, start immidiately, blink 4 times (2 times OFF->ON, 2 times ON->OFF, interleavedly) 
    
  }
  
}
