/******************************************************************************
* File       : Btn_SM_Module.c
* Function   : Provide demo of button state machine module.
* description: It is a demo for button state machine module.              
* Version    : V1.00
* Author     : Ian
* Date       : 15th Jun 2016
* History    :  No.  When          Who   Version   What        
*               1    15/Jun/2016   Ian   V1.00     Create      
******************************************************************************/

#include "common.h"
#include "KL25_Lpt_Time.h"
#include "Btn_SM_Config.h"
#include "Btn_SM_Module.h"

const uint8* cg_apu8State[] = {"PRESS_EVT            |  __  ",\
                               "SHORT_RELEASE_EVT    |  __  ",\
                               "LONG_RELEASE_EVT     |  __  ",\
                               "PRESSED_EVT          |  __  ",\
                               "JUST_LONG_PRESSED_EVT|  __  ",\
                               "SHORT_RELEASED_EVT   |  __  ",\
                               "LONG_RELEASED_EVT    |  __  ",\
                               "JUST_PRESS           |    | ",\
                               "SHORT_RELEASED       | |    ",\
                               "LONG_RELEASED        | |    ",\
                               "IDLE                 | |    ",\
                               "SHORT_PRESSED        |    | ",\
                               "LONG_PRESSED         |    | ",\
                               "NONE_EVT                    "};

const uint8* cg_apu8Vol[]   = {"Vol:[               ]",\
                               "Vol:[|              ]",\
                               "Vol:[||             ]",\
                               "Vol:[|||            ]",\
                               "Vol:[||||           ]",\
                               "Vol:[|||||          ]",\
                               "Vol:[||||||         ]",\
                               "Vol:[|||||||        ]",\
                               "Vol:[||||||||       ]",\
                               "Vol:[|||||||||      ]",\
                               "Vol:[||||||||||     ]",\
                               "Vol:[|||||||||||    ]",\
                               "Vol:[||||||||||||   ]",\
                               "Vol:[|||||||||||||  ]",\
                               "Vol:[|||||||||||||| ]",\
                               "Vol:[|||||||||||||||]"};

static T_BTN_RESULT sg_atBtn[MAX_BTN_CH];

/******************************************************************************
* Name       : uint8 Btn_St_Get(uint8 u8Ch)
* Function   : Provide button state according to the channel number
* Input      : uint8 u8Ch  1~255          The number of button channel
* Output:    : None
* Return     : uint8       BTN_STATE_0    0-State
*                          BTN_STATE_1    1-State
*                          BTN_ERROR      Failed
* description: None.
* Version    : V1.00
* Author     : Ian
* Date       : 15th Jun 2016
******************************************************************************/
uint8 Btn_St_Get(uint8 u8Ch)
{
    uint8 u8Temp;
    if(u8Ch == 1)    /* If it is button 1 */
    {   /* Get the button state */
        u8Temp = (!(GPIOA_PDIR & (1 << 5)));
    }
    if(u8Ch == 2)    /* If it is button 2 */
    {   /* Get the button state */
        u8Temp = (!(GPIOA_PDIR & (1 << 4)));
    }
    if(u8Ch == 3)    /* If it is button 3 */
    {   /* Get the button state */
        u8Temp = (!(GPIOA_PDIR & (1 << 12)));
    }
    else/* If the button channel number is invalid */
    {   /* Return error */
        u8Temp = BTN_ERROR;
    }
    return  u8Temp;
}

/******************************************************************************
* Name       : uint16 System_Time(void)
* Function   : Provide system time in ms
* Input      : None
* Output:    : None
* Return     : uint16      0~65535        The system time in ms
* description: None.
* Version    : V1.00
* Author     : Ian
* Date       : 15th Jun 2016
******************************************************************************/
uint16 System_Time(void)
{
    uint16 u16Temp = (uint16)App_GetSystemTime_ms();
    return u16Temp;
}

/******************************************************************************
* Name       : void Gpio_Init()
* Function   : Init GPIOs for as inputs for buttons
* Input      : None
* Output:    : None
* Return     : None
* description: None.
* Version    : V1.00
* Author     : Ian
* Date       : 15th Jun 2016
******************************************************************************/
void Gpio_Init()
{
    PORTA_PCR12 = PORT_PCR_MUX(0x1);  /* Set the PIN as a GPIO */
    PORTA_PCR5  = PORT_PCR_MUX(0x1);  /* Set the PIN as a GPIO */
    PORTA_PCR4  = PORT_PCR_MUX(0x1);  /* Set the PIN as a GPIO */
    GPIOA_PDDR  &= ~(1 << 12);        /* Set the GPIO as input */
    GPIOA_PDDR  &= ~(1 << 5);         /* Set the GPIO as input */
    GPIOA_PDDR  &= ~(1 << 4);         /* Set the GPIO as input */
    return;
}

/******************************************************************************
* Name       : int main (void)
* Function   : A demo shows how to use button state machine module
* Input      : None
* Output:    : None
* Return     : None
* description: There are three buttons:
*              Button 3 is used to switch operating mode between "button state 
*              display" and "Vol control". Long pressed to swtich.
*              Button 1 is used to show button state in "button state display"
*              mode, and increase vol in "Vol control" mode.
*              Button 2 is used to decrease vol in "Vol control" mode only.
* Version    : V1.00
* Author     : Ian
* Date       : 15th Jun 2016
******************************************************************************/
int main (void)
{   
    T_BTN_PARA tBtnPara;
    uint8 u8Idx,u8Vol = 0,u8FnCode = 1;;
    uint16 u16Tm = App_GetSystemTime_ms();

    /* Init hardware */
    Gpio_Init();
    Timer_Init();

    /* Button general init */
    Btn_General_Init(System_Time, Btn_St_Get);

    /* Configure the button parameters */
    tBtnPara.u16DebounceTm  = 50;              /* Debounce time is 50 ms          */
    tBtnPara.u16LongPressTm = 1000;            /* Long-press time is 1000ms       */
    tBtnPara.u8NormalSt     = 0;               /* The normal state of button is 1 */
    tBtnPara.u8BtnEn        = BTN_FUNC_ENABLE; /* Enable button at the beginning  */

    /* Button channels init */
    for(u8Idx = 1; u8Idx <= MAX_BTN_CH; u8Idx++)
    {
        Btn_Channel_Init(u8Idx ,&tBtnPara);
    }
 
    while(1)
    {   
        /* Button process and get button event & state */
        for(u8Idx = 1; u8Idx <= MAX_BTN_CH; u8Idx++)
        {
            Btn_Channel_Process(u8Idx, &(sg_atBtn[u8Idx-1]));
        }

        /* If the button 3 is long pressed */
        if(BTN_LONG_PRESSED_EVT == sg_atBtn[2].u8Evt)
        {   /* Switch operating mode */
            u8FnCode = !u8FnCode;
        }
        
        /* If it is the "button state" operation mode  */
        if(u8FnCode)     
        {   /* If there is any event with button 1 */
            if(sg_atBtn[0].u8Evt != BTN_NONE_EVT)
            {   /* Display the event of button 1 */
                printf("EVENT:%s\n",cg_apu8State[sg_atBtn[0].u8Evt]);
            }
            /* If the refresh time is up */
            if(App_GetSystemDelay_ms(u16Tm) > 100)
            {   /* Display the state of button 1 */
                printf("STATE:%s\n",cg_apu8State[sg_atBtn[0].u8State]);
                u16Tm = App_GetSystemTime_ms();
            }
        }
      
        /* If it is the "Vol control" operating mode */
        else
        {   /* If button 1 is short pressed (event) */
            if(sg_atBtn[0].u8Evt == BTN_PRESSED_EVT)
            {
                if(15 != u8Vol)
                {   /* Increase the vol within the range */
                    u8Vol++;
                }
                /* Reset the timing */
                u16Tm = App_GetSystemTime_ms();
            }
            /* If button 1 is in long pressed state */
            if(sg_atBtn[0].u8State == BTN_HOLDING_ST)
            {   /* If re-do time is up */
                if(App_GetSystemDelay_ms(u16Tm) > 150)
                {
                    if(15 != u8Vol)
                    {   /* Increase the vol within the range */
                        u8Vol++;
                    }
                    /* Reset the timing */
                    u16Tm = App_GetSystemTime_ms();
                }
            }
            
            /* If button 2 is short pressed (event) */
            if(sg_atBtn[1].u8Evt == BTN_PRESSED_EVT)
            {
                if(0 != u8Vol)
                {   /* Decrease the vol within the range */
                    u8Vol--;
                }
                /* Reset the timing */
                u16Tm = App_GetSystemTime_ms();
            }
            /* If button 2 is in long pressed state */
            if(sg_atBtn[1].u8State == BTN_HOLDING_ST)
            {   /* If re-do time is up */
                if(App_GetSystemDelay_ms(u16Tm)>150)
                {
                    if(0 != u8Vol)
                    {   /* Decrease the vol within the range */
                        u8Vol--;
                    }
                    /* Reset the timing */
                    u16Tm = App_GetSystemTime_ms();
                }
            }
            printf("%s\n", cg_apu8Vol[u8Vol]);
        }     
    }
    return 0;
}

/*End of file*/

