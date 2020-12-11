#include <M5EPD.h>

//Author Zell, 
//Version 1.1， 09.dez.2020, improvedGUI, added M5EPDUpdater, but failed, need improve,
//for use as bin app lovyan03, need a lot of modification  work
//#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5EPDUpdater.h>  // based on version 0.2.2 M5StackUpdater.h, tested not working, still original app

#include <M5paperUpdater.h>  // based on version 1.0.4 M5StackUpdater.h, tested not working, still original app
//SPI flash system, need run format code first!
#include "FS.h"
#include "FFat.h"
#define FORMAT_FFAT true // You only need to format FFat the first time you run a test

M5EPD_Canvas canvas(&M5.EPD);

int point[2][2];
char tmp_msg[64] ;

void setup()
{
    M5.begin();
    M5.EPD.SetRotation(90);
    M5.TP.SetRotation(90);
    M5.EPD.Clear(true);
    Serial.println( "E-ink Touch Demo v1.2"); //real system compiled time
    Serial.printf( "Author: Ling Zhou, Application was Compiled on %s %s\n", __DATE__, __TIME__ ); //real system compiled time
    canvas.createCanvas(540, 960);
    canvas.setTextSize(4);
    canvas.drawString("E-ink Touch Demo v1.2", 20, 100);
    canvas.setTextSize(5);
    canvas.drawString("Touch The Screen!", 20, 400);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);

      //for app flash back
      M5.update();
    //if (digitalRead(BUTTON_A_PIN) == 0) {
    if( M5.BtnP.wasPressed()){
    canvas.setTextSize(3);
    canvas.drawString(">:Will Load menu binary", 20, 20);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
    Serial.println("Will Load default binary");
    updateFromFS(SD);
    ESP.restart();
  }

    canvas.setTextSize(2);
}

void loop()
{   

    if(M5.TP.avaliable()){
        if(!M5.TP.isFingerUp()){
            M5.TP.update();
            canvas.fillCanvas(0);
            bool is_upadte = false;
            for(int i=0;i<2; i++){
                tp_finger_t FingerItem = M5.TP.readFinger(i);
                if((point[i][0]!=FingerItem.x)||(point[i][1]!=FingerItem.y)){
                    is_upadte = true;
                    point[i][0] = FingerItem.x;
                    point[i][1] = FingerItem.y;
                    //canvas.fillRect(FingerItem.x-50, FingerItem.y-50, 100, 100, 15);
                    int rect_wd = FingerItem.size;
                    canvas.fillRect(FingerItem.x-rect_wd, FingerItem.y-rect_wd, 2*rect_wd, 2*rect_wd, 15);
                   // canvas.fillRect(FingerItem.x-25, FingerItem.y-25, 50, 50, 15);
                    Serial.printf("Finger ID:%d-->X: %d  Y: %d  Size: %d\r\n", FingerItem.id, FingerItem.x, FingerItem.y , FingerItem.size);
                    sprintf(tmp_msg,"Finger ID:%d-->X: %d  Y: %d  Size: %d", FingerItem.id, FingerItem.x, FingerItem.y , FingerItem.size);
                    canvas.drawString(tmp_msg, 20, 900+i*20);
                    //canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
                    
                }
            }
            if(is_upadte)
            {   
                canvas.setTextSize(3);
                canvas.drawString("E-ink Touch Demo v1.1", 20, 20);
                canvas.setTextSize(2);
                canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
            }
        }
    }
    M5.update();
    if( M5.BtnP.wasPressed()){
    //canvas1.createRender(32, 256);
    canvas.setTextDatum(TC_DATUM);
    canvas.setTextSize(4);
    canvas.drawString(">:Will Load menu binary", 20, 210);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
    Serial.println("Will Load default binary");
    //updateFromFS(SD);
    updateFromFS(SD,"/Calculator-v0.0.1.bin");
    ESP.restart();
  }
  
  //load bin
  if( M5.BtnL.wasPressed()){
    //canvas1.createRender(32, 256);
    canvas.setTextDatum(TC_DATUM);
    canvas.setTextSize(4);
    canvas.drawString(">:Will Load menu binary", 20, 210);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
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
    canvas.setTextDatum(TC_DATUM);
    canvas.setTextSize(3);
    canvas.drawString(">:Formatting FFat finished!", 20, 210);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
  }
  else {
    Serial.println("-->:Internal flash FFat file system was initilized!");
  }
  unsigned mem_space = FFat.totalBytes();
  Serial.printf("Total space: %10u bytes (%2u MB)\n", mem_space, mem_space/1024/1024);
  mem_space = FFat.freeBytes();
  Serial.printf("Free space: %10u bytes (%2u MB)\n", mem_space, mem_space/1024/1024);
  //listDir(FFat, "/", 0);
  }
}
