/**
  Smart device V2  main.c file from MPLAB Code Configurator
 Author ling zhou 2019.11.4
 * version 1.3 add uart com, receive msg from esp32
 * version 1.2
 * When ESP32 is ON, PIC24 sleep is off; when ESP32 is OFF, PIC24 sleep is enabled to save power , 
 * running power consumption about 2 mA @ 0.5Mhz CPU ; 14mA @ 32Mhz
 * U1TX,U1RX ---> ESP32 UART serial port
 * RA1 LED run indication
 * RB4 ESP32 Vdd gate control, must set SOSC to digital mode, otherwise pin output not working!
 * RB14 INT1 Key/s3 user input
 * RA8 digi out RGB_LED 1 data
 * RA9 digi out RGB_LED 2 data
  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system intialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.95-b-SNAPSHOT
        Device            :  PIC24FV32KA304
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.36
        MPLAB 	          :  MPLAB X v5.10
 */

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
 */


//system para definition place them before include, otherwise will cause compiling error regarding delay function
#define SYS_FREQ         32000000L
#define FCY   SYS_FREQ/2
#define Reset() __asm__ volatile ("reset")
#define  NOP()  __asm__ volatile ("nop")
/**
  Section: Included Files
 */
#include "mcc_generated_files/system.h"
#include <stdlib.h>      //Data Conversion Functions
#include <string.h>   // memset
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <stdio.h>
#include <libpic30.h>  //delay us ms



//----------------RGB LED number ---------------
//RB8-11: sig 1-4
uint8_t RGB_LED_mode = 3; //led loop mode 1,2,3,8,24
extern uint8_t led_size;
extern uint8_t led_mode_idx;
extern uint16_t LED_data_mode1_idx;
#define led_status_group 4  //long strip
#define Light_SDA2  LATAbits.LATA9
#define Light_SDA  LATAbits.LATA8
//----------------LED SSR Pin ---------------
#define LED_ON()  LATAbits.LATA1=1
#define LED_OFF()  LATAbits.LATA1=0
#define SSR_ON() LATCbits.LATC9 = 1 //SSR high alarm
#define SSR_OFF() LATCbits.LATC9 = 0 //SSR OFF
#define ESP32_PWR_ON() LATBbits.LATB4=0 //PMOS gate low,
#define ESP32_PWR_OFF() LATBbits.LATB4=1

//----------------global sys vars ---------------
bool ESP32_PWR_status = false;
bool Toggle_ESP32_PWR = false;
bool LED_status = false;
bool UART1_EN = true;
uint8_t alert_st = 0 ; 
bool sleep_ENA = false; //test : false
//smartdevice ws2813 led test
bool Pixel_LED_EN = false; //for test only
bool Smartdevice_LED_EN = true; //for smartdevice demo
uint8_t Sys_LED_status = 1;
int Btn_push_cnt = 0;


//UART 115200 COM  ESP32,
//PIC HW uart buf 4 bytes?
#define UART_BUFFER_SIZE 128  
uint8_t Msg_buf[UART_BUFFER_SIZE];
//UART1_STATUS UART_state = 0;
uint16_t PIC_framehead = 0xAA24;
uint16_t PIC_frameend = 0xFF90;
uint8_t PIC_framehead_1 = 0xAA;
uint8_t PIC_framehead_2 = 0x24;
uint8_t PIC_frameend_1 = 0xFF;
uint8_t PIC_frameend_2 = 0x90;


void Toggle_LED(void) {

    LATAbits.LATA1 = !LATAbits.LATA1;

}

/*
                         Main application
 */
int main(void) {
    // initialize the device
    SYSTEM_Initialize();
    ESP32_PWR_ON();
    ESP32_PWR_status = true;
    LED_ON();
    uint8_t idx = 0;
    RGB_LED_Reset();

    if (UART1_EN) {
        printf("Smart device V2 XLP-MCU PIC24FV32KA304 is running, FW 1.3");
        printf("PIC UART RX is active, 115200bps");
        // UART1_Write(0x90);
    }
    uint8_t buf_idx = 0;

    while (1) {



        //LED_ON();
        if (Toggle_ESP32_PWR) {
            if (ESP32_PWR_status) {
                //when ESP32 is OFF, PIC24 sleep is enabled to save power , 
                ESP32_PWR_OFF();
                ESP32_PWR_status = false;
                LED_OFF();
                sleep_ENA = true;
                //sleep_ENA = false;//test only 
                //SSR_OFF();
            } else {
                //When ESP32 is ON, PIC24 sleep is disabled
                ESP32_PWR_ON();
                ESP32_PWR_status = true;
                LED_ON();
                sleep_ENA = false; // for future use
                //sleep_ENA = true; //test only 
                //SSR_ON();
            }
        }
        //reset status
        Toggle_ESP32_PWR = false;
        // __delay_us(100);
        //LED_OFF();
        // Toggle_LED();
        if (ESP32_PWR_status)
            LED_ON();
        // sleep to save power test, sleep will end if INTs happens


        //        if (Btn_push_cnt >= 3) {
        //            Pixel_LED_EN = false;
        //
        //        } else if (Btn_push_cnt > 1) {
        //Pixel_LED_EN = true;
        if (Btn_push_cnt >= 1) {
            //sleep_ENA = false;
            Sys_LED_status = (Btn_push_cnt % led_status_group) + 1;
        }



        if (Btn_push_cnt >= 5) {
            sleep_ENA = false;

        }
        //UART 
        if (UART1_EN) {
            /* Must clear the overrun error to keep UART receiving */
            //UART_state = UART1_StatusGet();

            //            if (UART1_StatusGet() & UART1_RX_OVERRUN_ERROR) {
            //                U1STAbits.OERR = 0;
            //                continue;
            //            }
            /* Must clear the overrun error to keep UART receiving */
            if (U1STAbits.OERR == 1) {
                U1STAbits.OERR = 0;
                //continue;
            }


            while (UART1_IsRxReady()) {
                Msg_buf[buf_idx] = UART1_Read();
                buf_idx++;
                if (buf_idx >= UART_BUFFER_SIZE) {
                    //idx = 0;
                    break;
                }

            }

            //parse data 2020.2
            if (buf_idx >= UART_BUFFER_SIZE) {
                for (buf_idx = UART_BUFFER_SIZE - 5; buf_idx > 0; buf_idx--) {
                    //if ( (Msg_buf[buf_idx] == PIC_framehead_1)&&  (Msg_buf[buf_idx+1] == PIC_framehead_2) ) {
                    if ( (Msg_buf[buf_idx] == PIC_framehead_1)&&(Msg_buf[buf_idx+1] == PIC_framehead_2)  ) {
                        
                        alert_st = Msg_buf[buf_idx+2];
                        //buf_idx = 0;
                        Sys_LED_status = alert_st;
                        break;
                    }
                }
                buf_idx = 0;
            }


            //printf("Smart device V2 XLP-MCU PIC24FV32KA304 is running, FW 1.3");
            //printf("PIC UART RX is active, 115200bps");
            // UART1_Write(0x90);
        }







        //test LED effects
        if (Pixel_LED_EN) {

            Light_SDA2 = 1;
            //----------WS2812 RGB LED-------------------------
            cycle_color_function(RGB_LED_mode, idx);
            //GRB
            // WS2811_SendByte( 0xF0) ; // for test timing
            idx++;
            RGB_LED_Idle();
            //----------WS2812 RGB LED  END-------------------------
        }


        if (Smartdevice_LED_EN) {

            Light_SDA2 = 1;
            //----------WS2812 RGB LED-------------------------
            // Sys_LED_status defines LED colors, idx  not used
            smartdevice_ws2813_function(Sys_LED_status, idx);
            //GRB
            // WS2811_SendByte( 0xF0) ; // for test timing
            idx++;
            RGB_LED_Idle();
            //----------WS2812 RGB LED  END-------------------------
        }


        __delay_us(200);
        if (sleep_ENA)
            Sleep();
        //The device will wake-up from Sleep mode on any of these events:
        // Interrupts, WDT time-out,device-reset
    }

    return 1;
}
/**
 End of File
 */

