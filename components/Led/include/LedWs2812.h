
#ifndef _LEDWS2812_H
#define _LEDWS2812_H

#include "Timers.h"


//=========================================== PUBLIC (Function Prototypes) ===========================================
void LedWS2812__Init(void);
void LedWS2812__SetPixel(uint8_t led_num,bool OnOff_state, uint8_t R, uint8_t G,uint8_t B );
void led_fashion(void);
//=====================================================================================================================


typedef enum
{
    //This is Linked with "LED_MANAGER_STATE_ENUM", do not change the sequence or number
	OPERATION__LED_ON       = 1,
	OPERATION__LED_OFF      = 2,
	OPERATION__LED_FLASH    = 3,
	OPERATION__LED_FADEIN   = 5,
	OPERATION__LED_FADEOUT  = 6,		
	OPERATION__NULL  
} OPEARTION_TYPE;


struct LED_STRUCT_TYPE
{
    uint8_t Hardware_Id;
    MS_TIMER_LABEL Timer_num;
    OPEARTION_TYPE Operation;
    uint32_t OnTime;
    uint32_t OffTime;
    uint8_t Color_R;
    uint8_t Color_G;
    uint8_t Color_B;
    bool ToogleBit;
    bool PendingReq;
   
};

#endif


