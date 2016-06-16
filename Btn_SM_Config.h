/******************************************************************************
* File       : Btn_SM_Config.h
* Function   : Check button state and return short/long press or release
* description: To be done          
* Version    : V1.10
* Author     : Ian
* Date       : 15th Jun 2016
* History    :  No.  When          Who   Version   What        
*               1    27/Jan/2016   Ian   V1.00     Create      
*               2    15/Jun/2016   Ian   V1.10     Re-design the state machine with state
*                                                  table, return "Event" and "State"   
******************************************************************************/



#ifndef _BTN_SM_CONFIG_
#define _BTN_SM_CONFIG_

#ifdef __cplusplus
extern "C" {
#endif
  
#define MAX_BTN_CH                   (2)         /* Max number of buttons, please define it here */

/* If you want to use specified button state getting function, define the MARCO */
//#define BTN_SM_SPECIFIED_BTN_ST_FN             /* Use specified button state getting function  */

typedef unsigned char       uint8;
typedef unsigned short int  uint16;
typedef unsigned int        uint32;


#endif

#ifdef __cplusplus
}
#endif

#endif /* _BTN_SM_CONFIG_ */





