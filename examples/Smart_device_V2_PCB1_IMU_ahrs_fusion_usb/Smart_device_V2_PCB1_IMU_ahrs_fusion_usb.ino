//modified to fit for smart device v2 by ling zhou  2019.11.14
//original code from  DPEng_ICM20948
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Mahony_DPEng.h>
#include <Madgwick_DPEng.h>
#include <DPEng_ICM20948_AK09916.h>


// Create sensor instance.
DPEng_ICM20948 dpEng = DPEng_ICM20948(0x948A, 0x948B, 0x948C);

// Mag calibration values are calculated via ahrs_calibration example sketch results（MotionCal.exe）.
// These values must be determined for each baord/environment.
// See the image in this sketch folder for the values used
// below.

// Offsets applied to raw x/y/z mag values， obtained from MotionCal.exe
float mag_offsets[3]            = { -16.8F, -13.68F, 10.33F };  // for 1# PCB smart device v2 ICM-20948  2019.11.18

// Soft iron error compensation matrix ，  obtained from MotionCal.exe
float mag_softiron_matrix[3][3] = { 
  {  1.000,  -0.002,  0.006 },
  {  -0.002,  1.014,  0.002 },
  {  0.006,   0.002,  0.996 }
}; // for 1# PCB smart device v2 ICM-20948  2019.11.18

//mag_field_strength seems not used in th program
float mag_field_strength        = 33.43F;  // for 1# PCB smart device v2 ICM-20948  2019.11.18

// Offsets applied to compensate for gyro zero-drift error for x/y/z
float gyro_zero_offsets[3]      = { 0.0F, 0.0F, 0.0F };

// Mahony is lighter weight as a filter and should be used
// on slower systems
Mahony_DPEng filter;
//Madgwick_DPEng filter;


//PCB related paras
const int ledPin = 2;
unsigned char blink_status = 1;
unsigned long run_cnt = 0;
const unsigned long Benchmark_count = 100 * 100000; //n* 10^6
long now = 0;

void setup()
{
  Serial.begin(115200);

  // Wait for the Serial Monitor to open (comment out to run without Serial Monitor)
  // while(!Serial);

  Serial.println(F("DPEng AHRS Fusion Example")); Serial.println("");

  // Initialize the sensors.
  if (!dpEng.begin(ICM20948_ACCELRANGE_4G, GYRO_RANGE_250DPS, ICM20948_ACCELLOWPASS_50_4_HZ))
  {
    /* There was a problem detecting the ICM20948 ... check your connections */
    Serial.println("Ooops, no ICM20948/AK09916 detected ... Check your wiring!");
    while (1);
  }

  filter.begin();
}

void loop(void)
{


  blink_status = 1 - blink_status;
  digitalWrite(ledPin, blink_status);
  now = millis();
  sensors_event_t accel_event;
  sensors_event_t gyro_event;
  sensors_event_t mag_event;

  // Get new data samples
  dpEng.getEvent(&accel_event, &gyro_event, &mag_event);

  // Apply mag offset compensation (base values in uTesla)
  float x = mag_event.magnetic.x - mag_offsets[0];
  float y = mag_event.magnetic.y - mag_offsets[1];
  float z = mag_event.magnetic.z - mag_offsets[2];

  // Apply mag soft iron error compensation
  float mx = x * mag_softiron_matrix[0][0] + y * mag_softiron_matrix[0][1] + z * mag_softiron_matrix[0][2];
  float my = x * mag_softiron_matrix[1][0] + y * mag_softiron_matrix[1][1] + z * mag_softiron_matrix[1][2];
  float mz = x * mag_softiron_matrix[2][0] + y * mag_softiron_matrix[2][1] + z * mag_softiron_matrix[2][2];

  // Apply gyro zero-rate error compensation
  float gx = gyro_event.gyro.x + gyro_zero_offsets[0];
  float gy = gyro_event.gyro.y + gyro_zero_offsets[1];
  float gz = gyro_event.gyro.z + gyro_zero_offsets[2];

  // Update the filter
  filter.update(gx, gy, gz,
                accel_event.acceleration.x, accel_event.acceleration.y, accel_event.acceleration.z,
                mx, my, mz);

  // Print the orientation filter output
  // Note: To avoid gimbal lock you should read quaternions not Euler
  // angles, but Euler angles are used here since they are easier to
  // understand looking at the raw values. See the ble fusion sketch for
  // and example of working with quaternion data.
  float roll = filter.getRoll();
  float pitch = filter.getPitch();
  float heading = filter.getYaw(); //mag poles 
  now = millis() - now;
  //Serial.print(millis());
  Serial.print(" - Orientation(Yaw,Pitch,Row): ");
  Serial.print(heading);
  Serial.print(" ");
  Serial.print(pitch);
  Serial.print(" ");
  Serial.println(roll);

  Show_raw_values();
  Serial.println("");
  Serial.printf("------- System benchmark run count: %d  loop time cost: %d ms  -------\r\n", run_cnt, now);
  Serial.println("");
  run_cnt++;
  delay(20);
}

void Show_raw_values( void){

  sensors_event_t accel_event;
  sensors_event_t gyro_event;
  sensors_event_t mag_event; // Need to read raw data, which is stored at the same time

  // Get new data samples
  dpEng.getEvent(&accel_event, &gyro_event, &mag_event);
  Serial.print(" - Raw ACC_GRYO_Mag:");

  Serial.print(dpEng.accel_raw.x);
  Serial.print(',');
  Serial.print(dpEng.accel_raw.y);
  Serial.print(',');
  Serial.print(dpEng.accel_raw.z);
  Serial.print(',');

  Serial.print(dpEng.gyro_raw.x);
  Serial.print(',');
  Serial.print(dpEng.gyro_raw.y);
  Serial.print(',');
  Serial.print(dpEng.gyro_raw.z);
  Serial.print(',');

  Serial.print(dpEng.mag_raw.x);
  Serial.print(',');
  Serial.print(dpEng.mag_raw.y);
  Serial.print(',');
  Serial.print(dpEng.mag_raw.z);
  Serial.println();
  
}
