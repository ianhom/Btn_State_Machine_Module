/******************************************************************************
* File       : Btn_SM_Config.h
* Function   : Configure file for button state machine module.
* description: All configuration should be done here.
*              1. Modify number of used button with MAX_BTN_ST      
*              2. Define __BTN_SM_SPECIFIED_BTN_ST_FN if you want use specified
*                 button state getting function for each button.    
* Version    : V1.10
* Author     : Ian
* Date       : 15th Jun 2016
* History    :  No.  When          Who   Version   What        
*               1    15/Jun/2016   Ian   V1.10     Create      
******************************************************************************/


#ifndef _BTN_SM_CONFIG_
#define _BTN_SM_CONFIG_

#ifdef __cplusplus
extern "C" {
#endif
  
#define MAX_BTN_CH                   (3)         /* Max number of buttons, please define it here */

/* If you want to use specified button state getting function, define the MACRO */
//#define __BTN_SM_SPECIFIED_BTN_ST_FN             /* Use specified button state getting function  */

/* If type is NOT defined, define the type here */
typedef unsigned char       uint8;
typedef unsigned short int  uint16;
typedef unsigned long int   uint32;


#ifdef __cplusplus
}
#endif

#endif /* _BTN_SM_CONFIG_ */





