#ifndef MOTIONSENSOR_H
#define MOTIONSENSOR_H
#include <ArduinoJson.h>
typedef struct {
    char name[50];
    unsigned int pin_number;
    unsigned long refresh_rate_milisecs;
    unsigned long motion_time_period_milisecs;
    short minimal_triggering_number;
} t_SensorConfig;

typedef struct { // not used
  short last_sensor_trigger_count;  
  unsigned long last_check_time;
} t_DetectionData;

class MotionSensor {
    private:
      unsigned long last_ValidDetection_time;
      unsigned long last_check_time;
      unsigned int sensor_trigger_count;
      unsigned int motion_detected_count;
      bool is_OK;
      t_SensorConfig config; 
      bool SerialVerbose;
      DynamicJsonDocument doc = DynamicJsonDocument(1024);
      String JsonDetectionData;
      void Set_JsonValidDetectionData(void); 
      bool (*Logger) (char *text, bool SerialPrint);
      
    public:
      
      MotionSensor(char * sensor_name, unsigned int sensor_pin_number,  bool Verbose, bool (*ll) (char *, bool));
  		bool Setup(
  			  unsigned long sensor_refresh_rate_milisecs,
  			  unsigned long sensor_motion_time_period_milisecs,
  			  short sensor_minimal_triggering_number);
  
      bool try_recv(void);
      const char *GetName(void);
      bool ValidDetection(void); // Checking if we have a Valid Detection that occured within motion_time_period_milisecs
      void loop(void); // must be called OFTEN !
      const String GetLastJsonValidDetectionData(void);

};
#endif
