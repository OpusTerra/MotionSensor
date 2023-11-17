/**
 * MotionSensor is a simple and easy to use Arduino class for the implementation
 * of non-blocking timers, as it is far better to use non-blocking timers
 * with a micro-controller since they allow you to trigger code at defined
 * durations of time, without stopping the execution of your main loop.
 *
 * Written by - Denis Lambert
 * Full documentation can be found at:
 *
 * See LICENSE file for acceptable use conditions - this is open source
 * and there are no restrictions on its usage, I simply ask for some acknowledgment
 * if it is used in your project.
 */
 
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "MotionSensor.h"
#include <string.h>


MotionSensor::MotionSensor(char *sensor_name, unsigned int sensor_pin_number, bool Verbose, bool (*ll) (char *, bool))
{
  last_ValidDetection_time = 0L;
  last_check_time = 0L;
  sensor_trigger_count = 0;
  motion_detected_count = 0;
  is_OK = false;
  SerialVerbose = Verbose;
  JsonDetectionData = "Nothing yet";
  
  Logger = ll;
  
  strcpy(config.name, sensor_name);
  config.pin_number = sensor_pin_number;

  pinMode(config.pin_number, INPUT);
}

void MotionSensor::Set_JsonValidDetectionData(void)
{
    doc["sensor"] = config.name;
    doc["motion_detected_count"] = motion_detected_count;
    doc["sensor_trigger_count"] = sensor_trigger_count;
    doc["last_ValidDetection_time"] = last_ValidDetection_time;

    JsonDetectionData = "";
    
    serializeJson(doc, JsonDetectionData);
}

const String MotionSensor::GetLastJsonValidDetectionData(void)
{
  return JsonDetectionData;
}

bool MotionSensor::Setup(
			unsigned long sensor_refresh_rate_milisecs,
			unsigned long sensor_motion_time_period_milisecs,
			short sensor_minimal_triggering_number) 
{
    char buff[250];
    bool ret = true;
    
    config.refresh_rate_milisecs = sensor_refresh_rate_milisecs;
    config.motion_time_period_milisecs = sensor_motion_time_period_milisecs;
    config.minimal_triggering_number = sensor_minimal_triggering_number;

    is_OK = false; 

    last_ValidDetection_time = 0L;
    last_check_time = 0L;
    sensor_trigger_count = 0;
    motion_detected_count = 0;

    if(sensor_minimal_triggering_number*sensor_refresh_rate_milisecs >sensor_motion_time_period_milisecs)
    {
      sprintf(buff,"\nWrong config.\nImpossible to reach minimal triggering number with these values: %d * %d (%d) > %d ",
          sensor_minimal_triggering_number,
          sensor_refresh_rate_milisecs,
          sensor_minimal_triggering_number*sensor_refresh_rate_milisecs,
          sensor_motion_time_period_milisecs); 
          if (SerialVerbose)
            Serial.println(buff);
		
		  if (Logger) 
           (*Logger)(buff, false);
	   
        ret= false;
    }
    return ret;
}

const char *MotionSensor::GetName()
{
  return (const char *)config.name;
}
bool MotionSensor::try_recv(void)
{
  // Read PIR Signal Pin
  if(config.pin_number != -1)
    return (digitalRead(config.pin_number) == HIGH ? true : false);
  else
    return false;  
}

bool MotionSensor::ValidDetection(void)
{
  if(is_OK)
  {
    is_OK = false;
    return true;
  } 
  else
    return false;   
}

void  MotionSensor::loop(void)
{
 
  char buff[250];

  unsigned long time_difference;

  time_difference = millis() - last_check_time;
  if (time_difference <=   config.refresh_rate_milisecs)
  {
        // Not enough time passed since last check
        sprintf(buff, "Not enough time (%ld) passed since last check at %ld", time_difference, last_check_time ); 
        // Serial.println(buff);
        return ;
  }
  
  // Serial.println("Time to check for new detections");  
  if(try_recv())
  {
      motion_detected_count++; // ssi PIN du PIR est à HIGH à ce moment là
      if(motion_detected_count == 1)
      {
        sprintf(buff, "**New motion sequence** Resetting sensor_trigger_count: %d", sensor_trigger_count);                      
        if (SerialVerbose) 
          Serial.println(buff);
        sensor_trigger_count = 0;  // on reset le sensor checker si on vient juste de commencer a avoir un mouvement
      }
  }
    
  sensor_trigger_count += 1;

  // because we use Instant::now, the real time difference needs to be multiply by counts to
  // reflect real motion time period time
  time_difference = config.refresh_rate_milisecs * sensor_trigger_count;
  sprintf(buff, "motion_detected_count: %d sensor_trigger_count: %d time_difference: %ld", motion_detected_count, sensor_trigger_count, time_difference );
  if (SerialVerbose)   
   Serial.println(buff);
   
  if (time_difference > config.motion_time_period_milisecs)
  {
      // This is a new detection period - reset the counters
      sprintf(buff, "Motion Time elapsed. Resetting - sensor_trigger_count %d time_difference: %ld at %ld", sensor_trigger_count, time_difference, millis() );  
      if (SerialVerbose) 
        Serial.println(buff);  
        
      sensor_trigger_count = 1;
      motion_detected_count = 0;
  }

  // If minimal triggering number within motion time period is reached for motion_detected_count, then we got valid detection.
  if (sensor_trigger_count >= config.minimal_triggering_number)
  {
        // Minimal triggering number is reached for sensor checking. Let's check if we also had a  minimal_triggering_number for motion_detected_count 
        if(motion_detected_count >= config.minimal_triggering_number)
        {
          // Minimal triggering number is reached for  motion_detected_count - WE HAVE a VALID detection within minimal_triggering_number and motion_time_period_milisecs
          last_ValidDetection_time = millis();
          
          sprintf(buff, "Detection happened: sensor: %s, sensor_trigger_count: %d", config.name, sensor_trigger_count);
          // Serial.println(buff);  

          Set_JsonValidDetectionData();
          
          if (SerialVerbose) 
          {
            Serial.print("Detection at ");
            Serial.println(last_ValidDetection_time);
          }
          is_OK = true; // sera remis à false par la fonction qui va utiliser le résultat
        }
        else
        {
          if(motion_detected_count != 0)
          {
            sprintf(buff, "**Rejected motion**: sensor: %s, motion_detected_count: %d sensor_trigger_count: %d during %d msec", config.name, motion_detected_count, sensor_trigger_count, config.motion_time_period_milisecs );                      
            if (Logger) 
              (*Logger)(buff, true);
          }
        }
        // Reset the counters
        sensor_trigger_count = 0;
        motion_detected_count = 0;
  }
  
  // Update the last check time
  last_check_time = millis();
}
