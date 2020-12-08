//The Fonts file (M5EPD_TTF) used in the example is in the same path as the program, please install Fonts file in TF card or SPIFFS before compiling.

//Author Zell, 07.12.2020
//Version 2.0 added FAT file system, added SD updater support, but failed after reb0ot!
//Version 1.0 basic demo
#include "M5EPD.h"
#include "FS.h"
#include "SPIFFS.h"
#include "binaryttf.h" // no chinese support
#include <M5EPDUpdater.h> 

//SPI flash system, need run format code first!
#include "FS.h"
#include "FFat.h"
#define FORMAT_FFAT true // You only need to format FFat the first time you run a test
//#define FORMAT_FFAT false // You only need to format FFat the first time you run a test
// This file should be compiled with 'Partition Scheme' (in Tools menu)
// set to '16M Flash (3MB APP/9MB FATFS)'  for smart device 16M wrover module

//Example for TTF font
//#define USE_SPIFFS //test not working the new m5paper 
#define USE_SD 1

M5EPD_Canvas canvas1(&M5.EPD);

void setup()
{
    M5.begin();
    M5.TP.SetRotation(90);
    M5.EPD.SetRotation(90);
    M5.EPD.Clear(true);

    if (!SPIFFS.begin(true))
    {
        log_e("SPIFFS Mount Failed");
        while(1);
    }

    Serial.println("<M5paper TrueTypeFont demo v2.0>");

#ifdef USE_SPIFFS
    Serial.println("Will Load GenSenRounded-R.ttf from SPIFFS!");
    canvas1.loadFont("/GenSenRounded-R.ttf", SPIFFS); // Load font files from SPIFFS
#elif USE_SD
    //Serial.println("Will load simkai.ttf from SD!");
    Serial.println("Will load GenSenRounded-R from SD!");
    canvas1.loadFont("/GenSenRounded-R.ttf", SD); //GenSenRounded-R.ttf
    //canvas1.loadFont("/simkai.ttf", SD); // Load font files from SD Card
#else
    Serial.println("Will load build-in binaryttf font...");
    canvas1.loadFont(binaryttf, sizeof(binaryttf)); // Load font files from binary data
#endif

    canvas1.createCanvas(540, 960);
    canvas1.createRender(48, 256);
    canvas1.setTextSize(48);
    canvas1.setTextColor(0);
    canvas1.setTextDatum(TC_DATUM);// plotting alignment
    canvas1.fillRect(0, 0, 540, 200, 15);
    canvas1.drawString("TTF font rendering", 270, 20);
    canvas1.drawString("TTF レンダリング", 270, 80);
    canvas1.drawString("TTF 字体渲染", 270, 140);

    canvas1.setTextColor(15);
    canvas1.setTextDatum(BL_DATUM);
    uint16_t x = 15;
    for(int i = 8; i < 72; i += 8)
    {
        if(i != 48)
        {
            canvas1.createRender(i);
        }
        
        canvas1.setTextSize(i);
        canvas1.drawString("※", x, 300);
        x += i;
    }

    for(int i = 72 - 16; i >= 8; i -= 8)
    {
        canvas1.setTextSize(i);
        canvas1.drawString("※", x, 300);
        x += i;

        if(i != 48)
        {
            canvas1.destoryRender(i);
        }
    }

    canvas1.createRender(32, 256);//? size, cache size?
    canvas1.setTextSize(32);
    canvas1.setTextDatum(TC_DATUM);
    canvas1.drawString("Flexible font size", 270, 310);
    canvas1.drawString("柔軟なフォントサイズ", 270, 355);
    canvas1.drawString("灵活的字体大小", 270, 400);
    
    canvas1.setTextSize(48);
    canvas1.setTextColor(15);
    canvas1.setTextDatum(TC_DATUM);
    canvas1.drawString("❖★∑❀Ⅻゑǜψ※Ж≌╋", 10, 480);
    canvas1.setTextSize(32);
    canvas1.setTextDatum(TC_DATUM);
    canvas1.drawString("Perfect Unicode support", 270, 550);
    canvas1.drawString("完善的Unicode支持", 270, 595);
    canvas1.drawString("完全なUnicodeサポート", 270, 640);

    const uint16_t kDatumTy = 720;
    const uint16_t kDatumBy = 960;
    const uint16_t kDatumCy = kDatumTy + ((kDatumBy - kDatumTy) / 2);
    canvas1.setTextSize(48);
    canvas1.setTextColor(15);
    canvas1.setTextDatum(TL_DATUM);
    canvas1.drawString("TL←", 0, kDatumTy);
    canvas1.setTextDatum(TC_DATUM);
    canvas1.drawString("→TC←", 270, kDatumTy);
    canvas1.setTextDatum(TR_DATUM);
    canvas1.drawString("→TR", 540, kDatumTy);
    canvas1.setTextDatum(CL_DATUM);
    canvas1.drawString("CL←", 0, kDatumCy);
    canvas1.setTextDatum(CC_DATUM);
    canvas1.drawString("→CC←", 270, kDatumCy);
    canvas1.setTextDatum(CR_DATUM);
    canvas1.drawString("→CR", 540, kDatumCy);
    canvas1.setTextDatum(BL_DATUM);
    canvas1.drawString("BL←", 0, kDatumBy);
    canvas1.setTextDatum(BC_DATUM);
    canvas1.drawString("→BC←", 270, kDatumBy);
    canvas1.setTextDatum(BR_DATUM);
    canvas1.drawString("→BR", 540, kDatumBy);

    canvas1.pushCanvas(0, 0, UPDATE_MODE_GC16);
}

void loop()
{
    M5.update();
    if( M5.BtnP.wasPressed()){
    //canvas1.createRender(32, 256);
    canvas1.setTextDatum(TC_DATUM);
    canvas1.setTextSize(32);
    canvas1.drawString(">:Will Load Calculator binary", 20, 210);
    canvas1.pushCanvas(0,0,UPDATE_MODE_DU4);
    Serial.println("Will Load default binary");
    updateFromFS(SD,"/Calculator-v0.0.1.bin");
    ESP.restart();
  }
  //load bin

      if( M5.BtnL.wasPressed()){
    //canvas1.createRender(32, 256);
    canvas1.setTextDatum(TC_DATUM);
    canvas1.setTextSize(32);
    canvas1.drawString(">:Will Load menu binary", 20, 210);
    canvas1.pushCanvas(0,0,UPDATE_MODE_DU4);
    Serial.println("Will Load default binary");
    updateFromFS(SD,"/menu.bin");
    ESP.restart();
  }
  // FFat
  if( M5.BtnR.wasPressed()){

    //--------mount spi flash filesystem
  if (FORMAT_FFAT) FFat.format();//run only once
  if (!FFat.begin()) {
    Serial.println("!!!-->FFat Mount Failed!");
    FFat.format();//run only once
    Serial.println("--> Try formatting FFat finish!");
  }
  else {
    Serial.println("-->:Internal flash FFat file system is initilized!");
  }
  unsigned mem_space = FFat.totalBytes();
  Serial.printf("Total space: %10u bytes (%2u MB)\n", mem_space, mem_space/1024/1024);
  mem_space = FFat.freeBytes();
  Serial.printf("Free space: %10u bytes (%2u MB)\n", mem_space, mem_space/1024/1024);
  listDir(FFat, "/", 0);
  }
}



void listDir(fs::FS & fs, const char * dirname, uint8_t levels) {
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


void writeFile(fs::FS & fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- fwrite failed");
  }
}

void appendFile(fs::FS & fs, const char * path, const char * message) {
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

void renameFile(fs::FS & fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS & fs, const char * path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}
