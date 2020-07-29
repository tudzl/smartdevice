/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   WS2812_PIC.h
 * Author:   Ling zhou  
 * Comments: V2.0 for smartdevice v2 pic24fv32ka304
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>        /* For uint8_t definition */
#include <libpic30.h>  //delay us

//RA0  as driving pin 
#define Light_SDA  LATAbits.LATA8
#define Light_SDA2  LATAbits.LATA9
#define led_size_stripe 3  //long strip
//#define led_size_stripe 8  //long strip
#define led_ring_size 24  //long strip

#define LED_DGray       0x505050 // ??
#define LED_LGray       0x888888 // ??
#define LED_Gray        0xAAAAAA // ??
#define LED_White       0xFFFFFF  // ??
#define LED_Black       0x000000  // ??
#define LED_Green       0x00ff00  // ??
#define LED_Red 		0xff0000  // ??
#define LED_Blue        0x0000ff  // ??
#define LED_Yellow      0xffff00 // ??




void WS2811_SendByte (uint8_t dat);
void WS2812_SendByte (uint8_t dat);
void WS2811_pin_Init(void);
void RGB_LED_Reset(void);
void WS2811_send_data(uint8_t R,uint8_t G,uint8_t B);
void WS2812_send_data(uint8_t R,uint8_t G,uint8_t B);
void WS2811_send_color(uint32_t color);  //null  R  G  B
void WS2812_send_color(uint32_t color);  // G R B
void ls_mode(void);
void cycle_color_function(uint8_t mode, uint8_t idx);
void flow_color_function(uint8_t mode, uint8_t idx,uint16_t LED_data_mode1_idx ) ;
void smartdevice_ws2813_function(uint8_t status, uint8_t idx);
void RGB_LED_Idle(void);




// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

