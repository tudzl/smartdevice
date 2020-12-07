#include <M5EPD.h>

//for use as bin app lovyan03, need a lot of modification  work
//#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
#include <M5EPDUpdater.h> 


M5EPD_Canvas canvas(&M5.EPD);

int point[2][2];
char tmp_msg[64] ;

void setup()
{
    M5.begin();
    M5.EPD.SetRotation(90);
    M5.TP.SetRotation(90);
    M5.EPD.Clear(true);
    canvas.createCanvas(540, 960);
    canvas.setTextSize(4);
    canvas.drawString("E-ink Touch Demo v1.0", 20, 100);
    canvas.setTextSize(5);
    canvas.drawString("Touch The Screen!", 60, 400);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);

      //for app flash back
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
                    canvas.fillRect(FingerItem.x-25, FingerItem.y-25, 50, 50, 15);
                    Serial.printf("Finger ID:%d-->X: %d  Y: %d  Size: %d\r\n", FingerItem.id, FingerItem.x, FingerItem.y , FingerItem.size);
                    sprintf(tmp_msg,"Finger ID:%d-->X: %d  Y: %d  Size: %d", FingerItem.id, FingerItem.x, FingerItem.y , FingerItem.size);
                    canvas.drawString(tmp_msg, 20, 900);
                    //canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
                    
                }
            }
            if(is_upadte)
            {
                canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
            }
        }
    }
        M5.update();
    if( M5.BtnP.wasPressed()){
    //canvas1.createRender(32, 256);
    canvas1.setTextDatum(TC_DATUM);
    canvas1.setTextSize(32);
    canvas1.drawString(">:Will Load menu binary", 20, 210);
    canvas1.pushCanvas(0,0,UPDATE_MODE_DU4);
    Serial.println("Will Load default binary");
    updateFromFS(SD,"/Calculator-v0.0.1.bin");
    //updateFromFS(SD,"/Calculator-v0.0.1.bin");
    ESP.restart();
  }
}
