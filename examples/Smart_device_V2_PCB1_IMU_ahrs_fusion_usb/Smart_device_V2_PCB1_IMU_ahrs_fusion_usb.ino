//Version 2.1, added yaw_offsets
//Version 2.0,added Calc_tilt function using acc_X+Y+Z data, 2019.11.18
//modified to fit for smart device v2 by ling zhou  2019.11.14
//original code from  DPEng_ICM20948
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Mahony_DPEng.h>
#include <Madgwick_DPEng.h>
#include <DPEng_ICM20948_AK09916.h>

const float Gravity_HZ = 9.7936 ;
const float Gravity_0 = 9.80665 ;
float Linear_ACC = 0 ;
float Linear_ACC_max = 0;
float Tilt_A = 0;
float ACC_X, ACC_Y, ACC_Z;
// Create sensor instance.
DPEng_ICM20948 dpEng = DPEng_ICM20948(0x948A, 0x948B, 0x948C);

bool yaw_compensation_EN = true ;
float yaw_offsets = 24.7 ; //added by zl ,offsets between ecompass heading and physical compass north
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
  if (yaw_compensation_EN)
    heading = heading - yaw_offsets ;//!!!! 2019.11.18 only for PCB #1 !!!!
  now = millis() - now;
  //Serial.print(millis());
  Serial.print(" - Orientation(Yaw,Pitch,Row): ");
  Serial.print(heading);
  Serial.print(" ");
  Serial.print(pitch);
  Serial.print(" ");
  Serial.println(roll);
  Tilt_A = Calc_tilt(dpEng.accel_raw.x, dpEng.accel_raw.y, dpEng.accel_raw.z);
  if (pitch < 0)
    Tilt_A = -Tilt_A;
  Serial.printf("-->PCB Tilt in steady state: %.1f degree\r\n", Tilt_A);
  //Show_raw_values();
  show_sci_values();
  Linear_ACC = Calc_linearACC (ACC_X, ACC_Y, ACC_Z) ;
  Linear_ACC = Linear_ACC / Gravity_0;
  Linear_ACC_max = max(Linear_ACC_max,Linear_ACC);
  Serial.printf("-->Device Linear Acc: %.2f G; Max value: %.2f G\r\n", Linear_ACC,Linear_ACC_max);
  Serial.println("");
  Serial.printf("------- System benchmark run count: %d  loop time cost: %d ms  -------\r\n", run_cnt, now);
  Serial.println("");
  run_cnt++;
  delay(20);
}


void show_sci_values (void) {


  sensors_event_t aevent, gevent, mevent;

  /* Get a new sensor event */
  dpEng.getEvent(&aevent, &gevent, &mevent);
  ACC_X = aevent.acceleration.x;
  ACC_Y = aevent.acceleration.y;
  ACC_Z = aevent.acceleration.z;

  /* Display the accel results (acceleration is measured in m/s^2) */
  Serial.print("A ");
  Serial.print("X: "); Serial.print(aevent.acceleration.x, 4); Serial.print("  ");
  Serial.print("Y: "); Serial.print(aevent.acceleration.y, 4); Serial.print("  ");
  Serial.print("Z: "); Serial.print(aevent.acceleration.z, 4); Serial.print("  ");
  Serial.println("m/s^2");

  /* Display the gyro results (gyro data is in g) */
  Serial.print("G ");
  Serial.print("X: "); Serial.print(gevent.gyro.x, 1); Serial.print("  ");
  Serial.print("Y: "); Serial.print(gevent.gyro.y, 1); Serial.print("  ");
  Serial.print("Z: "); Serial.print(gevent.gyro.z, 1); Serial.print("  ");
  Serial.println("g");

  /* Display the mag results (mag data is in uTesla) */
  Serial.print("M ");
  Serial.print("X: "); Serial.print(mevent.magnetic.x, 1); Serial.print("  ");
  Serial.print("Y: "); Serial.print(mevent.magnetic.y, 1); Serial.print("  ");
  Serial.print("Z: "); Serial.print(mevent.magnetic.z, 1); Serial.print("  ");
  Serial.println("uT");

  Serial.println("");

}
void Show_raw_values( void) {

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

float Calc_linearACC ( float acc_x, float acc_y, float acc_z) {

  float Linear_ACC = 0 ;
  float R =  sq(acc_x ) + sq(acc_y) + sq(acc_z)  ;
  Linear_ACC = sqrt(R - sq(Gravity_0));

  return Linear_ACC;
}

float Calc_tilt ( float acc_x, float acc_y, float acc_z) {

  float tilt_angle = 0 ;
  float unit_g =  sq(acc_x ) + sq(acc_y) + sq(acc_z)  ;
  unit_g = sqrt(unit_g);
  //cos(tilt_angle)= acc_z /unit_g ;
  tilt_angle = acos(acc_z / unit_g); //radians
  tilt_angle = tilt_angle * RAD_TO_DEG ; //convert to degree,#define RAD_TO_DEG 57.295779513082320876798154814105
  return tilt_angle;
}
