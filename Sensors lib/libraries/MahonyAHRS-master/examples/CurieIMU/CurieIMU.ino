// Basic Inertial Monitoring Unit (IMU) using Mahony filter.
//
// To view this data, use the Arduino Serial Monitor to watch the
// scrolling angles (in radians: 1.57 is 90 degrees).
//
// For graphical display, this Processing sketch works:
// https://www.arduino.cc/en/Tutorial/Genuino101CurieIMUOrientationVisualiser

#include <CurieIMU.h>
#include <MahonyAHRS.h>
#include <Wire.h>

Mahony filter;

void setup() {
  Serial.begin(9600);
  CurieIMU.begin();
}

void loop() {
  int ax, ay, az;
  int gx, gy, gz;
  float roll, pitch, heading;

  // Read the motion sensors
  CurieIMU.readMotionSensor(ax, ay, az, gx, gy, gz);

  // Update the Mahony filter, with scaled gyroscope
  float gyroScale = 0.001;  // TODO: the filter updates too fast
  filter.updateIMU(gx * gyroScale, gy * gyroScale, gz * gyroScale, ax, ay, az);

  if (readyToPrint()) {
    // print the heading, pitch and roll
    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();
    Serial.print(heading);
    Serial.print(",");
    Serial.print(pitch);
    Serial.print(",");
    Serial.println(roll);
  }
}


// Decide when to print
bool readyToPrint() {
  static unsigned long nowMillis;
  static unsigned long thenMillis;

  // If the Processing visualization sketch is sending "s"
  // then send new data each time it wants to redraw
  while (Serial.available()) {
    int val = Serial.read();
    if (val == 's') {
      thenMillis = millis();
      return true;
    }
  }
  // Otherwise, print 8 times per second, for viewing as
  // scrolling numbers in the Arduino Serial Monitor
  nowMillis = millis();
  if (nowMillis - thenMillis > 125) {
    thenMillis = nowMillis;
    return true;
  }
  return false;
}
