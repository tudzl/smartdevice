/*
 * File:   WS2812_PIC.c
 * Author: ling
 *
 * Created on April 25, 2018, 11:15 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>        /* For uint8_t definition */
#include <libpic30.h>  //delay us
#include "WS2812_PIC.h"
/*
#define  NOP()  __asm__ volatile ("nop")
 */
#define SYS_FREQ         32000000L
#define FCY    SYS_FREQ/2


#define __delay_ms(d) \
  { __delay32( (unsigned long) (((unsigned long long) d)*(FCY)/1000ULL)); }
#define __delay_us(d) \
  { __delay32( (unsigned long) (((unsigned long long) d)*(FCY)/1000000ULL)); }


#define  NOP()  __asm__ volatile ("nop")
//extern uint8_t RGB_LED_mode;
uint16_t ls_t;
uint8_t LED_data[45][3];
uint8_t led_size = led_size_stripe; //8
//uint8_t led_size = led_ring_size;
uint8_t LED_data_mode1_len = 45;
uint8_t SM_LED_data_mode1_len = 3; //smartdevice
uint16_t LED_data_mode1_idx = 0;
uint8_t LED_data_mode1[45][3];
//smartdevice
uint8_t Smartdevice_LED_status1[3][3];
uint8_t Smartdevice_LED_status2[3][3];
uint8_t Smartdevice_LED_status3[3][3];
uint8_t Smartdevice_LED_status4[3][3];



//ws2811 24bits data GRB, MSB first

void WS2811_pin_Init(void) {
    //PIC pin


}
//for WS2811 code 0 : H 0.5us L 2us;  code 1: 1.2/ 1.3 us
//for WS2812 code 0: H 0.4us L 0.85us;  code 1: 0.85/ 0.4 us

void WS2811_SendByte(uint8_t dat)//??1BIT???
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (dat & 0x80) //send bit 1
        {
            Light_SDA = 1;
            __delay_us(1);
            NOP();
            Light_SDA = 0;
            //NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP(); //62.5 ns
            //NOP();
            //NOP();
        } else //send bit 0
        {
            Light_SDA = 1; //0
            //NOP();
            //NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            Light_SDA = 0;
            __delay_us(1);
        }
        dat <<= 1;
    }
}

//for WS2812 code 0: H 0.4us L 0.85us;  code 1: 0.85/ 0.4 us

void WS2812_SendByte(uint8_t dat)//??1BIT???
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (dat & 0x80) //send bit 1/Hi, MSB first
        {
            Light_SDA = 1;
            //__delay_us(1);
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();

            Light_SDA = 0;
            //NOP();
            NOP();
            //NOP();
            //NOP();
            // NOP();
            //NOP();
            //NOP();
            // NOP(); //62.5 ns
            //NOP();
            //NOP();
        } else //send bit 0/Low
        {
            Light_SDA = 1; //0
            NOP(); // ttt
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            Light_SDA = 0;
            NOP();
            NOP();
            NOP();
            //            NOP();
            //            NOP();
            //            NOP();
            //            NOP();
            //NOP();
            //__delay_us(1);
        }
        dat <<= 1;
    }
}

//for WS2812 code 0: 0.25 / 0.75 us                  H 220 to 380ns L 580 to 1600ns 0.85ns;   
//           code 1: 0.85/ 0.4 us

void WS2813_SendByte(uint8_t dat)//??1BIT???
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (dat & 0x80) //send bit 1
        {
            Light_SDA = 1;
            //__delay_us(1);
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();

            Light_SDA = 0;
            //NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            //NOP();
            //NOP();
            // NOP();
            //NOP();
            //NOP();
            // NOP(); //62.5 ns
            //NOP();
            //NOP();
        } else //send bit 0
        {
            Light_SDA = 1; //0
            NOP(); // ttt
            NOP();
            NOP();
            NOP();
            //NOP();
            Light_SDA = 0;
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            NOP();
            //__delay_us(1);
        }
        dat <<= 1;
    }
}

void RGB_LED_Reset(void) {
    Light_SDA = 0;
    __delay_us(80);
}

void RGB_LED_Idle(void) {
    Light_SDA = 0;
    //__delay_us(500);
}

void WS2811_send_color(uint32_t color) {
    uint8_t i;
    uint8_t RGB_tmp = 0;
    //led_size = RGB_LED_mode;
    for (i = 0; i < led_size; i++) {

        RGB_tmp = (color & 0x00ff00) >> 8;
        WS2811_SendByte(RGB_tmp);
        RGB_tmp = (color & 0xff0000) >> 16;
        WS2811_SendByte(RGB_tmp);
        RGB_tmp = (color & 0x0000ff);
        WS2811_SendByte(RGB_tmp);

        //

        // WS2811_SendByte(G);
        // WS2811_SendByte(R);
        // WS2811_SendByte(B);
    }
    RGB_LED_Reset();
}

//whole led strips

void WS2812_send_color(uint32_t color) {
    uint8_t i;
    uint8_t RGB_tmp = 0;
    //led_size= RGB_LED_mode;
    for (i = 0; i < led_size; i++) {

        RGB_tmp = (color & 0x00ff00) >> 8;
        WS2812_SendByte(RGB_tmp); //G
        RGB_tmp = (color & 0xff0000) >> 16;
        WS2812_SendByte(RGB_tmp); //R
        RGB_tmp = (color & 0x0000ff);
        WS2812_SendByte(RGB_tmp); //B

        //

        // WS2811_SendByte(G);
        // WS2811_SendByte(R);
        // WS2811_SendByte(B);
    }
    RGB_LED_Reset();
}

void WS2811_send_data(uint8_t R, uint8_t G, uint8_t B) {
    uint8_t i;
    for (i = 0; i < led_size; i++) {
        WS2811_SendByte(G);
        WS2811_SendByte(R);
        WS2811_SendByte(B);
    }
    RGB_LED_Reset();
}

void WS2812_send_data(uint8_t R, uint8_t G, uint8_t B) {
    uint8_t i;
    for (i = 0; i < led_size; i++) {
        WS2812_SendByte(G);
        WS2812_SendByte(R);
        WS2812_SendByte(B);
    }
    RGB_LED_Reset();
}

void WS2813_send_data(uint8_t R, uint8_t G, uint8_t B) {
    uint8_t i;
    for (i = 0; i < led_size; i++) {
        WS2811_SendByte(G);
        WS2811_SendByte(R);
        WS2811_SendByte(B);
    }
    RGB_LED_Reset();
}

// 3 ws2813 LEDs, no color flow, fixed patten,status defines color patten

void smartdevice_ws2813_function(uint8_t status, uint8_t idx) {
    uint8_t i;
    uint8_t color_pixel_idx =0;
    
    //uint8_t pixel_color = LED_data_mode1_idx % led_size;
    //normal 1
    if (status == 1) {
        for (i = led_size_stripe; i > 0; i--) {
            // if(idx<LED_data_mode1_len)
            //  {
            WS2813_SendByte(Smartdevice_LED_status1[(color_pixel_idx)][0]); //G
            WS2813_SendByte(Smartdevice_LED_status1[(color_pixel_idx)][1]); //R
            WS2813_SendByte(Smartdevice_LED_status1[(color_pixel_idx)][2]); //B
            color_pixel_idx++;
            //  }

        }


    
    }//alert 2
    else if (status == 2) {
        color_pixel_idx=0;
        for (i = led_size_stripe; i > 0; i--) {

            WS2813_SendByte(Smartdevice_LED_status2[(color_pixel_idx)][0]); //G
            WS2813_SendByte(Smartdevice_LED_status2[(color_pixel_idx)][1]); //R
            WS2813_SendByte(Smartdevice_LED_status2[(color_pixel_idx)][2]); //B
            color_pixel_idx++;

        }

    }//no wifi
    else if (status == 3) {
        color_pixel_idx=0;
        for (i = led_size_stripe; i > 0; i--) {

            WS2813_SendByte(Smartdevice_LED_status3[(color_pixel_idx)][0]); //G
            WS2813_SendByte(Smartdevice_LED_status3[(color_pixel_idx)][1]); //R
            WS2813_SendByte(Smartdevice_LED_status3[(color_pixel_idx)][2]); //B
            color_pixel_idx++;
            //  }

        }

    } else {
        color_pixel_idx=0;
        for (i = led_size_stripe; i > 0; i--) {
            
            WS2813_SendByte(Smartdevice_LED_status4[(color_pixel_idx)][0]); //G
            WS2813_SendByte(Smartdevice_LED_status4[(color_pixel_idx)][1]); //R
            WS2813_SendByte(Smartdevice_LED_status4[(color_pixel_idx)][2]); //B
            color_pixel_idx++;
            //  }

        }
    }

}

void cycle_color_function(uint8_t mode, uint8_t idx) {
    uint8_t i;
    //mode 8: 8 LED
    //mode 24: 24 LED Ring
    //led_size_stripe  8
    if (mode == 8) {

        for (i = led_size_stripe; i > 0; i--) {
            // if(idx<LED_data_mode1_len)
            //  {

            WS2812_SendByte(LED_data_mode1[(idx % LED_data_mode1_len)][1]); //R
            WS2812_SendByte(LED_data_mode1[(idx % LED_data_mode1_len)][0]); //G
            WS2812_SendByte(LED_data_mode1[(idx % LED_data_mode1_len)][2]); //B
            idx++;
            //  }

        }
    } // end of 8 LED mode

    if (mode == 24) {

        for (i = led_ring_size; i > 0; i--) {
            // if(idx<LED_data_mode1_len)
            //  {

            WS2812_SendByte(LED_data_mode1[(idx % LED_data_mode1_len)][1]); //R
            WS2812_SendByte(LED_data_mode1[(idx % LED_data_mode1_len)][0]); //G
            WS2812_SendByte(LED_data_mode1[(idx % LED_data_mode1_len)][2]); //B
            idx++;
            //  }
            //RGB_LED_Reset();

        }

    }//end of 24 LED mode


    else {
        flow_color_function(mode, idx, LED_data_mode1_idx);

    }

    LED_data_mode1_idx++;
    // LED_data_mode1_idx/=led_size;
    //LED_data_mode1_idx%=LED_data_mode1_len;


    // RGB_LED_Reset();

}


//called by previous cycle_color_function())

void flow_color_function(uint8_t mode, uint8_t idx, uint16_t LED_data_mode1_idx) {
    //mode 1  single dir flow
    //mode 2  3 pixels flows
    //mode 3 2 pixels flows in oppo dir
    uint8_t pixel_color = LED_data_mode1_idx % led_size;
    uint8_t pixel_idx = idx % led_size; //(0 to led_size)
    uint8_t i;
    //pixel_color++; // 
    if (pixel_color > 44)pixel_color = 0;
    if (mode == 0) mode = 1;

    if (mode == 1) {
        for (i = 0; i < led_size; i++) {

            if (i == pixel_idx) {

                WS2812_SendByte(LED_data_mode1[pixel_color][1]); //R
                WS2812_SendByte(LED_data_mode1[pixel_color][0]); //G
                WS2812_SendByte(LED_data_mode1[pixel_color][2]); //B
            } else {
                //send black pixel
                WS2812_SendByte(0); //R
                WS2812_SendByte(0); //G
                WS2812_SendByte(0); //B
            }

        }
    }//3 pixels flows
    else if (mode == 2) {

        for (i = 0; i < led_size; i++) {
            //for (i = led_size; i > 0; i--) {


            if (i == pixel_idx - 1) {
                //send one color pixel
                //            WS2812_SendByte(200); //G ws2812
                //            WS2812_SendByte(0); //B
                //            WS2812_SendByte(0); //R

                WS2812_SendByte(LED_data_mode1[pixel_color][1] / 8); //R
                WS2812_SendByte(LED_data_mode1[pixel_color][0] / 8); //G
                WS2812_SendByte(LED_data_mode1[pixel_color][2] / 8); //B
            } else if (i == pixel_idx) {
                //send one color pixel
                //            WS2812_SendByte(200); //G ws2812
                //            WS2812_SendByte(0); //B
                //            WS2812_SendByte(0); //R

                WS2812_SendByte(LED_data_mode1[pixel_color][1]); //R
                WS2812_SendByte(LED_data_mode1[pixel_color][0]); //G
                WS2812_SendByte(LED_data_mode1[pixel_color][2]); //B
            } else if (i == pixel_idx + 1) {
                //send one color pixel
                //            WS2812_SendByte(200); //G ws2812
                //            WS2812_SendByte(0); //B
                //            WS2812_SendByte(0); //R

                WS2812_SendByte(LED_data_mode1[pixel_color][1] / 8); //R
                WS2812_SendByte(LED_data_mode1[pixel_color][0] / 8); //G
                WS2812_SendByte(LED_data_mode1[pixel_color][2] / 8); //B
            } else {
                //send black pixel
                WS2812_SendByte(0); //R
                WS2812_SendByte(0); //G
                WS2812_SendByte(0); //B
            }

        }

    }//2 pixels flows in oppo dir
    else if (mode == 3) {

        for (i = 0; i < led_size; i++) {
            //for (i = led_size; i > 0; i--) {


            if (i == led_size - pixel_idx) {
                //send one color pixel
                //            WS2812_SendByte(200); //G ws2812
                //            WS2812_SendByte(0); //B
                //            WS2812_SendByte(0); //R

                WS2812_SendByte(LED_data_mode1[pixel_color][1]); //R
                WS2812_SendByte(LED_data_mode1[pixel_color][0]); //G
                WS2812_SendByte(LED_data_mode1[pixel_color][2]); //B
            } else if (i == pixel_idx) {
                //send one color pixel
                //            WS2812_SendByte(200); //G ws2812
                //            WS2812_SendByte(0); //B
                //            WS2812_SendByte(0); //R

                WS2812_SendByte(LED_data_mode1[pixel_color][1]); //R
                WS2812_SendByte(LED_data_mode1[pixel_color][0]); //G
                WS2812_SendByte(LED_data_mode1[pixel_color][2]); //B
            } else {
                //send black pixel
                WS2812_SendByte(0); //R
                WS2812_SendByte(0); //G
                WS2812_SendByte(0); //B
            }

        }

    }


}
/*
void ls_mode(void) {
    uint8_t i = 0, ys = 0, yz = 0;
    for (i = led_size; i > 0; i--) {
        if (i <= ls_t) {
            if (ys < ls_t) {
                ys++;
            } else
                ys = 0;
            WS2811_SendByte(LED_data[45 - (ys % 45)][1]); //R
            WS2811_SendByte(LED_data[45 - (ys % 45)][0]); //G
            WS2811_SendByte(LED_data[45 - (ys % 45)][2]); //B
        } else {
            if (yz > 0) {
                yz--;
            } else
                yz = led_size - ls_t;
            WS2811_SendByte(LED_data[yz % 45][1]);
            WS2811_SendByte(LED_data[yz % 45][0]);
            WS2811_SendByte(LED_data[yz % 45][2]);
        }
    }
    RGB_LED_Reset();
}

 */


uint8_t LED_data_mode1[45][3] = {

    {15, 0, 245}, //G,R,B
    {25, 0, 235},
    {45, 0, 225},
    {55, 0, 215}, //G,R,B
    {100, 0, 180}, //5
    {155, 0, 155},
    {180, 0, 100},
    {215, 0, 55},
    {235, 0, 35},
    {255, 0, 15}, //10
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0}, //15
    {255, 0, 0},
    {255, 15, 0},
    {235, 35, 0},
    {215, 55, 0},
    {180, 100, 0},
    {155, 155, 0}, //20
    {100, 180, 0}, //G,R,B
    {55, 215, 0},
    {35, 235, 0},
    {15, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0}, //30
    {0, 255, 15}, //G,R,B
    {0, 235, 35},
    {0, 215, 55},
    {0, 180, 100},
    {0, 155, 155},
    {0, 100, 180}, //35
    {0, 55, 215},
    {0, 35, 235},
    {0, 15, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255}//45
};



uint8_t LED_data_mode2[45][3] = {

    {15, 0, 235}, //G,R,B
    {25, 0, 225},
    {45, 0, 225},
    {55, 0, 215}, //G,R,B
    {100, 0, 180}, //5
    {155, 0, 155},
    {180, 0, 100},
    {215, 0, 55},
    {235, 0, 35},
    {245, 0, 25}, //10
    {250, 0, 15},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {245, 0, 0}, //15
    {235, 5, 0},
    {235, 15, 0},
    {225, 35, 0},
    {215, 55, 0},
    {180, 100, 0},
    {155, 155, 0}, //20
    {100, 180, 0}, //G,R,B
    {55, 215, 0},
    {35, 235, 0},
    {15, 245, 0},
    {5, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 245, 0},
    {0, 235, 5}, //30
    {0, 255, 15}, //G,R,B
    {0, 235, 35},
    {0, 215, 55},
    {0, 180, 100},
    {0, 155, 155},
    {0, 100, 180}, //35
    {0, 55, 215},
    {0, 35, 235},
    {0, 15, 245},
    {0, 5, 245},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 245}//45
};











uint8_t LED_data[45][3] = {
    {55, 0, 255}, //G,R,B
    {100, 0, 200},
    {155, 0, 155},
    {200, 0, 100},
    {255, 0, 55}, //5
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0}, //15
    {255, 55, 0},
    {200, 100, 0},
    {155, 155, 0},
    {100, 200, 0},
    {55, 255, 0}, //20
    {0, 255, 0}, //G,R,B
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0}, //30
    {0, 255, 55}, //G,R,B
    {0, 200, 100},
    {0, 155, 155},
    {0, 100, 200},
    {0, 55, 255},
    {0, 0, 255}, //35
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255},
    {0, 0, 255}//45
};


//Smartdevice v2
//LED1: PWR, LED2: WiFi, LED3: Alert
//Status1: Power ON ,WiFi OK , no alert default status
uint8_t Smartdevice_LED_status1[3][3] = {
    {240, 10, 10}, //G,R,B  WS2813A
    {200, 128, 0},
    {128, 0, 128},
};
//Status2: Power ON , alert active
uint8_t Smartdevice_LED_status2[3][3] = {
    {240, 10, 10}, //G,R,B  WS2813A
    {240, 128, 0},
    {10, 240, 10},
};

//Status3: Power ON ,WiFi OFF , no alert default status
uint8_t Smartdevice_LED_status3[3][3] = {
    {240, 10, 10}, //G,R,B  WS2813A
    {64, 240, 240},
    {10, 240, 10},
};

//Status def: Power ON ,init
uint8_t Smartdevice_LED_status4[3][3] = {
    {255, 255, 255}, //G,R,B  WS2813A
    {255, 230, 230},
    {255, 200, 200},
};