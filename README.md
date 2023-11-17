## PIR Motion Sensor 

Arduino library to interact mainly with PIR motion sensors. This lib was tested mainly on cheap HC-SR501 motion sensor and ESP8266 based device.

&nbsp;

## Why this library?

HC-SR501 PIR is very cheap and widely available an infrared motion sensor and it's very capable to various project based on arduino/raspberry/stm32, etc. 

However, because it's very cheap, it can detects "noise" from time to time, which means detection happens when there are no real motion within it's detection range - this is a false positive detection.

I was searching for a library that does the job and found nothing except something very interesting that was written in RUST language. (refer to https://github.com/mateusz-szczyrzyca/pir-motion-sensor/). So I try my best to port this RUST code to Arduino C++ and here it is. As the operation is similar and the configuration the same, I have duplicated here most of the Readme.md file for your convenience. 


&nbsp;

## Prerequisites

Use this reference manual for HC-SR501 PIR: https://lastminuteengineers.com/pir-sensor-arduino-tutorial/

Based on this manual, you should:

- set Sensitivity Adjustment yellow screw to longest range possible
- set Time-Delay Adjustment yellow screw to shortest time possible
- set Trigger Selection Jumper to Multiple Trigger Mode, by default it' probably set to Single Trigger

All of above settings can be programmatically changed by this library, so you won't need to touch jumper and screws again after this operation.

&nbsp;

## How it works

In this instruction, there is a term `valid detection` - this is a detection which is classified as *valid* by this library - which may be not the same as OUT pin state of the sensor - everything depends on your configuration.

In other words: depends on sensor configuration, there can be many detections made by sensor (here defined as setting it's OUT pin at high state), but it does not mean, there will be single `valid detection` classified.

&nbsp;

## Configuration

To init your sensor, consider the following parameters:

- GPIO PIN number (obvious staff)

- `sensor refresh rate`
  This isn't a refresh rate for a sensor itself, but the refresh rate for loop reading sensor PIN state. Shorter time allows
  to read data from sensor more often, thus it leads to better "refresh rate" of sensors itself, but may impose sligtly higher system load.

  Higher values of this parameter may lead to "miss" some fast detections.

- `motion time period`
  It's a time limit which app classifies `valid detection`. If this period is shorter, it means it will be "harder" to detect motion
  within specified time period. 

- `minimal triggering number`
  When a sensor detects motion (or something similar) it sets it's OUT pin to the high state. If you make initial adjustment correctly, this can happen couple times per second and that's fine. This setting is a number of such high state sets which is required within `motion time period` to classify single `valid detection`. This option is especially useful for excluding "noise" detections if >1

To conclude these parameters shortly: based on `sensor refresh rate` time, the library periodically reads state of sensor OUT pin. If there is a detection (here defined as the high state on sensor signal line), the library will try to count up these high states up to `minimal triggering number` within `motion time period` time. If `minimal triggering number` within `motion time period` is reached, then we got `valid detection`.

Setting these parameters allows you to decide how sensitive and accurate is your sensor. Because "noise" detections are usually very short hence using this library you can effectively get rid of them if your settings are not too sensitive (good tested values: `sensor_refresh_rate > 100`, `motion_time_period < 1000`, `minimal_triggering_number > 2`). Feel free to experiment with your own
values.

Keep in mind that these settings can affect each other, for instance: a very short `sensor_refresh_rate` can be reduced by higher values of `motion_time_period` and `minimal_triggering_number`

&nbsp;
## Using in your project

Please see example in `examples/` directory

&nbsp;

## Installation 

Simply download ZIP file and extract it in the Arduino library folder.

&nbsp;
## TODO

Not yet ...