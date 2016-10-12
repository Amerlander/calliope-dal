/*
The MIT License (MIT)

Copyright (c) 2016 Calliope GbR
This software is provided by DELTA Systems (Georg Sommer) - Thomas Kern
und Björn Eberhardt GbR by arrangement with Calliope GbR. 

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


#include "CalliopeRGB.h"
#include "MicroBitSystemTimer.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

//Pin of RGB LED on the MicroBit
#define CALLIOPE_PIN_RGB                    P0_18

//Default values for the LED color
#define RGB_LED_DEFAULT_GREEN               0
#define RGB_LED_DEFAULT_RED                 0
#define RGB_LED_DEFAULT_BLUE                0
#define RGB_LED_DEFAULT_WHITE               0
#define RGB_KEEP_VALUE                      -1

//max light intensity
#define RGB_LED_MAX_INTENSITY               255

//the following defines are timed specifically to the sending algorithm in CalliopeRGB.cpp
//timings for sending to the RGB LED: 
//logical '0': time HIGH: 0.35 us ±150 ns   time LOW: 0.9 us ±150 ns 
//logical '1': time HIGH: 0.9 us ±150 ns    time LOW: 0.35 us ±150 ns

CalliopeRGB::CalliopeRGB()
{
    //init pin
    nrf_gpio_cfg_output(PIN);
    nrf_gpio_pin_clear(PIN);
    
    state = 0;
    
    //init color settings
    GRBW[0] = RGB_LED_DEFAULT_GREEN;
    GRBW[1] = RGB_LED_DEFAULT_RED;
    GRBW[2] = RGB_LED_DEFAULT_BLUE;
    GRBW[3] = RGB_LED_DEFAULT_WHITE;
    
    //check intensity
    for(uint8_t i=0; i<4; i++) {
        if(GRBW[i] > RGB_LED_MAX_INTENSITY) GRBW[i] = RGB_LED_MAX_INTENSITY;
    }
    
    //add RGB LED object to the sytem timer
    system_timer_add_component(this);
}

CalliopeRGB::~CalliopeRGB()
{
    system_timer_remove_component(this);
}
 

//sets all 4 color settings to the given values        
void CalliopeRGB::Set_Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    //set color
    GRBW[0] = green;      
    GRBW[1] = red;
    GRBW[2] = blue;
    GRBW[3] = white;
    
    //check intensity
    for(uint8_t i=0; i<4; i++) {
        if(GRBW[i] > RGB_LED_MAX_INTENSITY) GRBW[i] = RGB_LED_MAX_INTENSITY;
    }
    
    //apply settings
    this->Send_to_LED();
}

void CalliopeRGB::On()
{
    this->Send_to_LED();
}


void CalliopeRGB::Off()
{
    //save color settings
    uint8_t GRBW_temp[4];
    for(uint8_t i=0; i<4; i++) GRBW_temp[i] = GRBW[i];
    
    //set all color values to zero and send to LED
    for(uint8_t i=0; i<4; i++) GRBW[i] = 0;
    this->Send_to_LED();
    
    //restore color values
    for(uint8_t i=0; i<4; i++) GRBW[i] = GRBW_temp[i];
}


//sends current color settings to the RGB LED
void CalliopeRGB::Send_to_LED()
{   
    const uint32_t CONST_BIT = 1UL << PIN;

    //set PIN to LOW for 50 us
    NRF_GPIO->OUTCLR = (CONST_BIT);
    nrf_delay_us(50);
    
    SERIAL_DEBUG->printf("RGB(%02x, %02x, %02x, %02x)\r\n", GRBW[1], GRBW[0], GRBW[2], GRBW[3]);
    //send bytes
    for (uint8_t i=0; i<4; i++)
    {
        for(int8_t j=7; j>-1; j--) 
        {
            if (GRBW[i] & (1 << j)) 
            {
                NRF_GPIO->OUTSET = (CONST_BIT);
                __ASM volatile (
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                );
                NRF_GPIO->OUTCLR = (CONST_BIT);
            }
            else 
            {
                 NRF_GPIO->OUTSET = (CONST_BIT);
                __ASM volatile (
                    "NOP\n\t"
                );
                NRF_GPIO->OUTCLR = (CONST_BIT);
                __ASM volatile (
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                    "NOP\n\t"
                );

            }
        }
    }
    
    //set state
    uint16_t temp = GRBW[0] + GRBW[1] + GRBW[2] + GRBW[3];
    if(temp > 0) state = 1;
    else state = 0;
}


//getter functions for current color settings
uint8_t CalliopeRGB::Get_Red()
{
    return GRBW[1];   
}

uint8_t CalliopeRGB::Get_Green()
{
    return GRBW[0];  
}

uint8_t CalliopeRGB::Get_Blue()
{
    return GRBW[2];
}

uint8_t CalliopeRGB::Get_White()
{
    return GRBW[3]; 
}

        
//check function
bool CalliopeRGB::Is_On()
{
    if(state) return true;
    else return false;  
}
        
    
void CalliopeRGB::systemTick()
{
    //currently not in use
}
