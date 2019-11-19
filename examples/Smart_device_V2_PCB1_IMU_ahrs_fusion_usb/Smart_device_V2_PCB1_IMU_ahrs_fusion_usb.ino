
/***********************ICM20948 9-axis motion sensor*****************************************
  Smart device v2 Main PCB code for ESP32 wrover and ICM20948 9 DOF motion sensor, Max Linear acc meter
 ***************************************************************/

//Version 2.2, added FFAT
//Version 2.1, added yaw_offsets
//Version 2.0,added Calc_tilt function using acc_X+Y+Z data, 2019.11.18
//modified to fit for smart device v2 by ling zhou  2019.11.14
//original code from  DPEng_ICM20948
/*
  "C:\\Users\\ling\\AppData\\Local\\Arduino15\\packages\\esp32\\hardware\\esp32\\1.0.4/tools/xtensa-esp32-elf/bin/xtensa-esp32-elf-size" -A "C:\\Users\\ling\\AppData\\Local\\Temp\\arduino_build_594389/Smart_device_V2_PCB1_IMU_ahrs_fusion_usb.ino.elf"
  Sketch uses 327486 bytes (10%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 16604 bytes (5%) of dynamic memory, leaving 311076 bytes for local variables. Maximum is 327680 bytes.
*/
//I've found in my experience FatFs (plus wearleveling) to be faster than SPIFFS.
#include "FS.h"
#include "FFat.h"
#define FORMAT_FFAT false // You only need to format FFat the first time you run a test
// This file should be compiled with 'Partition Scheme' (in Tools menu)
// set to '16M Flash (3MB APP/9MB FATFS)'  for smart device 16M wrover module

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Mahony_DPEng.h>
#include <Madgwick_DPEng.h>
#include <DPEng_ICM20948_AK09916.h>
#include <M5Stack.h>

char tmp_string[64];
const float Gravity_HZ = 9.7936 ;
const float Gravity_0 = 9.80665 ;
float Linear_ACC = 0 ;
float Linear_ACC_max = 0;
float Linear_ACC_max_last = 0; // for file writting
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

  M5.begin();
  //Serial.begin(115200);

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

  Serial.setDebugOutput(true);
  if (FORMAT_FFAT) FFat.format();
  if (!FFat.begin()) {
    Serial.println("Smart device FFat Mount Failed");
    return;
  }
  unsigned mem_space = FFat.totalBytes();
  Serial.printf("Total space: %10u bytes (%2u MB)\n", mem_space, mem_space / 1024 / 1024);
  mem_space = FFat.freeBytes();
  Serial.printf("Free space: %10u bytes (%2u MB)\n", mem_space, mem_space / 1024 / 1024);
  listDir(FFat, "/", 0);
  Serial.println("Now read last time Max Linear Acc value:");
  readFile(FFat, "/Linear_ACC_max.txt");
  //  writeFile(FFat, "/hello.txt", "Hello ");
  //  appendFile(FFat, "/hello.txt", "World!\r\n");
  //  readFile(FFat, "/hello.txt");
  //  renameFile(FFat, "/hello.txt", "/foo.txt");
  //  readFile(FFat, "/foo.txt");
  //  deleteFile(FFat, "/foo.txt");

}

void loop(void)
{


  blink_status = 1 - blink_status;
  digitalWrite(ledPin, blink_status);


  M5.update(); // This function reads The State of Button A and B and C.

  //toggle gas reading
  if (M5.BtnA.wasPressed() ) {
    buttonA_wasPressed();
    //Serial.printf("BtnA was Pressed\r\n");
  }








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
  Serial.print("--> Orientation(Yaw,Pitch,Row): ");
  Serial.print(heading);
  Serial.print(" ");
  Serial.print(pitch);
  Serial.print(" ");
  Serial.println(roll);
  Tilt_A = Calc_tilt(dpEng.accel_raw.x, dpEng.accel_raw.y, dpEng.accel_raw.z);
  if (pitch < 0)
    Tilt_A = -Tilt_A;
  Serial.printf("--> PCB Tilt in steady state: %.1f degree\r\n", Tilt_A);
  //Show_raw_values();
  show_sci_values();
  Linear_ACC = Calc_linearACC (ACC_X, ACC_Y, ACC_Z) ; // m/s2
  Linear_ACC = Linear_ACC / Gravity_0; // to G
  Linear_ACC_max = max(Linear_ACC_max, Linear_ACC);
  Serial.printf("--> current Linear Acc: %.2f G; Max value: %.2f ; Recorded Max: %.2f G\r\n", Linear_ACC, Linear_ACC_max, Linear_ACC_max_last);
  Serial.println("");
  Serial.printf("------- System run count: %d  loop time cost: %d ms  -------\r\n", run_cnt, now);
  Serial.println("");
  run_cnt++;
  if (run_cnt % 1000 == 0) {
    Record_max_G();
    //    Serial.println("Recording max linear ACC value now:");
    //    if (Linear_ACC_max > Linear_ACC_max_last) {
    //      Serial.println("Current Linear_ACC_max is larger than previous one, writting files now...");
    //      dtostrf(Linear_ACC_max, 2, 2, tmp_string);
    //      //toFloat()
    //      //sprintf(tmp_string, "%d", run_cnt);
    //      writeFile(FFat, "/Linear_ACC_max.txt", tmp_string);
    //      readFile(FFat, "/Linear_ACC_max.txt");
    //    }

  }
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
  if (R > Gravity_HZ)
    Linear_ACC = sqrt(R - sq(Gravity_HZ)); // <0 bugs found @ 2019.11.19
  else Linear_ACC = 0;

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

void Record_max_G(void) {

  if (Linear_ACC_max > Linear_ACC_max_last) {
    Serial.println("Current Linear_ACC_max is larger than previous one, writting files now...");
    dtostrf(Linear_ACC_max, 2, 2, tmp_string);
    //toFloat()
    //sprintf(tmp_string, "%d", run_cnt);
    writeFile(FFat, "/Linear_ACC_max.txt", tmp_string);
    readFile(FFat, "/Linear_ACC_max.txt");
  }
}

void readFile(fs::FS &fs, const char * path) {

  char buf[32];
  unsigned int i = 0 ;
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while (file.available()) {
    buf[i] = file.read();
    Serial.write(buf[i]);
    i++;
  }
  Serial.println("");
  Serial.println("File reading ends here!");
  String tmp_string = String(buf);
  Linear_ACC_max_last = tmp_string.toFloat();
  Serial.printf("Apply %.2f to Linear_ACC_max_last!", Linear_ACC_max_last);
  Serial.println("");
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}


void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message: 'World!' appended");
  } else {
    Serial.println("- append failed");
  }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

//reset Recorded Linear Acc Max to Zero
void buttonA_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  Serial.println(" !--> buttonA/S1 was Pressed");
  Serial.println("##reset Recorded Linear Acc Max to Zero！");
  Linear_ACC_max_last = 0;
  Serial.println("##Recording new max Linear Acc value...");
  Record_max_G();

}
