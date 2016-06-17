/******************************************************************************
* File       : Btn_SM_Module.c
* Function   : Check button state and return short/long press or release events and states.
* description: Button state machine module. It realise button short/long press or release
*              checking and return responding events and states.
*              - Following parameters are configurable:
*                  * Number of buttons.
*                  * Normal state of each buttons.
*                  * Enable/Disable control of each buttons.
*                  * Debounce checking time of each buttons.
*                  * Long-pressed time of each buttons.
*                  * Button state getting function of each buttons.
*
*              - Following events (transient) can be provided
*                  * BTN_PRESSED_EVT        Button is just short pressed                       
*                  * BTN_LONG_PRESSED_EVT   Button is just long pressed                              
*                  * BTN_S_RELEASED_EVT     Button is just released from short pressed                 
*                  * BTN_L_RELEASED_EVT     Button is just released from long pressed    
*
*              - Following states (stable-state) can be provided
*                  * BTN_IDLE_ST            Button stays in idle state (Not pressed)
*                  * BTN_PRESS_AFT_ST       Button stays in short pressed state
*                  * BTN_HOLDING_ST         Button stays in long pressed state              
*              __________
*              HOW TO USE(Quick start): 
*              Step 1: Modify MAX_BTN_CH in Btn_SM_Config.h with desired button numbers
*              Step 2: Create a "uint16 (*)()" function to get system time according to
*                      your hardware.
*              Step 3: Create a "uint8 (*)(uint8 u8Ch)" function to get button status(0/1)
*                      according to your hardware.
*              Step 4: Call "Btn_SM_Easy_Init()" to get previous interface functions.
*                      If defined __BTN_SM_SPECIFIED_BTN_ST_FN, each button will use 
*                      specified button state getting function.
*              Step 7: Poll "Btn_Channel_Process()" per channel to get events and states.
*
*              NOTE: For advanced configuration, please use Btn_General_Init() and
*                    Btn_channel_Init().
*              NOTE: Button function can be enabled/disabled by calling "Btn_Func_En_Dis().
*              NOTE: This software is modularization designed, so "Btn_SM_Module.c" and 
*                    "Btn_SM_Module.h" should work without any modification. 
*         
* Version    : V1.10
* Author     : Ian
* Date       : 15th Jun 2016
* History    :  No.  When          Who   Version   What        
*               1    27/Jan/2016   Ian   V1.00     Create      
*               2    15/Jun/2016   Ian   V1.10     Re-design the state machine with state
*                                                  table, return "Event" and "State"                                                          
******************************************************************************/

#include "common.h"
#include "Btn_SM_Config.h"
#include "Btn_SM_Module.h"

/* State transition table */
const uint8 cg_aau8StateMachine[BTN_STATE_NUM][BTN_TRG_NUM] = 
{
    /*  Situation 1  */    /*  Situation 2 */    /*  Situation 3  */     /* Situation 4  */
    /* Btn NOT press */    /* Btn press    */    /* Btn NOT press */     /* Btn press    */
    /* Time NOT out  */    /* Time NOT out */    /* Time out      */     /* Time out     */
    {BTN_PRESS_PRE_ST    , BTN_PRESS_PRE_ST    , BTN_PRESS_PRE_ST     ,  BTN_PRESS_PRE_ST    },    /* BTN_PRESS_EVT        */
    {BTN_SHORT_RELEASE_ST, BTN_SHORT_RELEASE_ST, BTN_SHORT_RELEASE_ST ,  BTN_SHORT_RELEASE_ST},    /* BTN_S_RELEASE_EVT    */
    {BTN_LONG_RELEASE_ST , BTN_LONG_RELEASE_ST , BTN_LONG_RELEASE_ST  ,  BTN_LONG_RELEASE_ST },    /* BTN_L_RELEASE_EVT    */
    {BTN_PRESS_AFT_ST    , BTN_PRESS_AFT_ST    , BTN_PRESS_AFT_ST     ,  BTN_PRESS_AFT_ST    },    /* BTN_PRESSED_EVT      */
    {BTN_HOLDING_ST      , BTN_HOLDING_ST      , BTN_HOLDING_ST       ,  BTN_HOLDING_ST      },    /* BTN_LONG_PRESSED_EVT */
    {BTN_IDLE_ST         , BTN_IDLE_ST         , BTN_IDLE_ST          ,  BTN_IDLE_ST         },    /* BTN_S_RELEASED_EVT   */
    {BTN_IDLE_ST         , BTN_IDLE_ST         , BTN_IDLE_ST          ,  BTN_IDLE_ST         },    /* BTN_L_RELEASED_EVT   */
    {BTN_IDLE_ST         , BTN_PRESS_PRE_ST    , BTN_IDLE_ST          ,  BTN_PRESSED_EVT     },    /* BTN_PRESS_PRE        */
    {BTN_SHORT_RELEASE_ST, BTN_PRESS_AFT_ST    , BTN_S_RELEASED_EVT   ,  BTN_PRESS_AFT_ST    },    /* BTN_SHORT_RELEASE    */
    {BTN_LONG_RELEASE_ST , BTN_HOLDING_ST      , BTN_L_RELEASED_EVT   ,  BTN_HOLDING_ST      },    /* BTN_LONG_RELEASE     */
    {BTN_IDLE_ST         , BTN_PRESS_EVT       , BTN_IDLE_ST          ,  BTN_PRESSED_EVT     },    /* BTN_IDLE             */  
    {BTN_S_RELEASE_EVT   , BTN_PRESS_AFT_ST    , BTN_S_RELEASE_EVT    ,  BTN_LONG_PRESSED_EVT},    /* BTN_PRESS_AFT        */
    {BTN_L_RELEASE_EVT   , BTN_HOLDING_ST      , BTN_L_RELEASED_EVT   ,  BTN_HOLDING_ST      }     /* BTN_HOLDING          */
};

static T_BTN_PARA *sg_aptBtnPara[MAX_BTN_CH] = {0};          /* Parameter interface          */
static T_BTN_ST    sg_atBtnSt[MAX_BTN_CH]    = {0};          /* Running status               */
static PF_GET_TM   sg_pfGetTm                = NULL;         /* Function to get general time */

#ifndef __BTN_SM_SPECIFIED_BTN_ST_FN
static PF_GET_BTN  sg_pfGetBtnSt             = NULL;         /* Function to get button state */
#endif

/******************************************************************************
* Name       : void Btn_Func_En_Dis(uint8 u8Ch, uint8 u8EnDis)
* Function   : Enable or disable button function
* Input      : uint8 u8Ch       1~255               The number of setting button channel
*              uint8 u8EnDis    BTN_FUNC_ENABLE     Enable the button function
*                               BTN_FUNC_DISABLE    Disable the button function
* Output:    : None
* Return     : None
* description: If the button function is enabled, the button events and states could
*              be returned according to the button operation; If the button function 
*              is disabled, the returned event and state will be BTN_NONE_EVT and 
*              BTN_DIS_STeven button is operated. 
* Version    : V1.00
* Author     : Ian
* Date       : 27th Jan 2016
******************************************************************************/
void Btn_Func_En_Dis(uint8 u8Ch, uint8 u8EnDis)
{   
    sg_aptBtnPara[u8Ch - 1]->u8BtnEn = u8EnDis;      /* Enable or Disable the button functions */
    sg_atBtnSt[u8Ch - 1].u8BtnSt     = BTN_IDLE_ST;  /* Reset the state machine of button      */
}

/******************************************************************************
* Name       : uint8 Btn_General_Init(PF_GET_TM pfGetTm, PF_GET_BTN pfGetBtnSt)
* Function   : General init operation of button state machine.
* Input      : PF_GET_TM pfGetTm       Function to get gengeral time
*              PF_GET_BTN pfGetBtnSt   Function to get button state(0/1)
* Output:    : None
* Return     : BTN_ERROR               Input parameter is invalid
*              SUCCESS                 Init operation is successed
* description: This function should be called once before use button checking.
*              This function get two necessary interface function: 
*              "pfGetTm()":Function to get gengeral time;
*              "pfGetBtnSt":Function to get button state(0/1). (if the MACRO 
*              __BTN_SM_SPECIFIED_BTN_ST_FN is NOT defined)
*
*              NOTE:If the general init is failed, DO NOT continue!!
* Version    : V1.00
* Author     : Ian
* Date       : 27th Jan 2016
******************************************************************************/
uint8 Btn_General_Init(PF_GET_TM pfGetTm, PF_GET_BTN pfGetBtnSt)
{
    /* Check if the input parameter is valid or NOT */
    if(NULL == pfGetTm) 
    {   /* Return if the parameter is invalid */
        return BTN_ERROR;
    }
    
    /* If the "Get time" function has NOT registered yet */
    if(NULL == sg_pfGetTm)
    {   /* Register the function */
        sg_pfGetTm = pfGetTm;
    }
    /* If the "Get time" function has already registered, Do NOT re-register */


#ifndef __BTN_SM_SPECIFIED_BTN_ST_FN
    /* Check if the input parameter is valid or NOT */
    if(NULL == pfGetBtnSt) 
    {   /* Return if the parameter is invalid */
        return BTN_ERROR;
    }   

    /* If the "Get button state" function has NOT registered yet */
    if(NULL == sg_pfGetBtnSt)
    {   /* Register the function */
        sg_pfGetBtnSt = pfGetBtnSt;
    }
    /* If the "Get button state" function has already registered, Do NOT re-regiter */
#else
    /* If each button use specified button state getting function*/
    /* pfGetBtnSt will be useless                                */
    (void)pfGetBtnSt;             /* Avoid warning from complier */
#endif

    return SUCCESS;
}

/******************************************************************************
* Name       : uint8 Btn_Channel_Init(uint8 u8Ch ,T_BTN_PARA *ptBtnPara)
* Function   : Init operation for each button channel
* Input      : uint8       u8Ch      1~255      The number of setting button channel
*              T_BTN_PARA *ptBtnPara             Parameter of each button channel
* Output:    : None
* Return     : BTN_ERROR        Input parameter is invalid
*              SUCCESS          Init operation is successed
* description: After general init operation, call this function for each channel
*              before button checking.  
*              This function save the parameter structure pointer for further 
*              operation, and init running state of such channel.
*
*              NOTE:If the channel init is failed, DO NOT continue!!
* Version    : V1.00
* Author     : Ian
* Date       : 27th Jan 2016
******************************************************************************/
uint8 Btn_Channel_Init(uint8 u8Ch ,T_BTN_PARA *ptBtnPara)
{   
    /* Check if the channel number is invalid */
    if((0 == u8Ch) || (u8Ch > MAX_BTN_CH))
    {   /* If the channel number is NOT in the range of 1~MAX_BTN_CH, return error */
        return BTN_ERROR;
    }

    /* Check if the input parameter is invalid */
    if(NULL == ptBtnPara)
    {   /* Return error if parameter is invalid */
        return BTN_ERROR;
    }

#ifdef __BTN_SM_SPECIFIED_BTN_ST_FN
    /* Check if the input parameter is invalid */
    if(NULL == ptBtnPara->pfGetBtnSt)
    {   /* Return error if parameter is invalid */
        return BTN_ERROR;
    }
#endif

    sg_aptBtnPara[u8Ch - 1]      = ptBtnPara;   /* Get the parameters              */
    sg_atBtnSt[u8Ch - 1].u8BtnSt = BTN_IDLE_ST; /* Init the state of state machine */    

    return SUCCESS;
}

/******************************************************************************
* Name       : uint8 Btn_Channel_Process(uint8 u8Ch, T_BTN_RESULT* ptBtnRes)
* Function   : Main process of button checking
* Input      : uint8         u8Ch       1~255                 The number of setting button channel
* Output:    : T_BTN_RESULT* ptBtnRes
*                             ->u8Evt   BTN_PRESSED_EVT       Button is just short pressed
*                                       BTN_LONG_PRESSED_EVT  Button is just long pressed
*                                       BTN_S_RELEASED_EVT    Button is just released from short press
*                                       BTN_L_RELEASED_EVT    Button is just released from long press
*                             ->u8State BTN_IDLE_ST           Button is in idle state
*                                       BTN_PRESS_AFT_ST      Button is in short pressed state
*                                       BTN_HOLDING_ST        Button is in long pressed state
*                                       BTN_DIS_ST            Butoon is disabled
* Return     : BTN_ERROR     Input parameter or button state is invalid
*              SUCCESS       Process operation is successed
* description: This function should be called after general init and channel init.
*              This function should be polled to check button event and state.
*              ------------------------------------------------------------------------------
*             | No.1 Demo of long press and release with debounce check
*             |  ________                                                ______________ ________
*             |          |  Debounce |               |                  |   Debounce   |
*             |    Idle  | Press PRE |  Press AFT    |    Holding       |   Long Rls   |  Idle   
*             |  (output)|           |   (output)    |   (output)       |              | (output)     
*             |          |___________|_______________|__________________|              |
*             |          |           |               |                  |              |
*             |          V           V               V                  V              V
*             |     Press Evt    Pressed Evt   Long pressed Evt   Long release Evt   Long released Evt
*             |                   (output)        (output)                             (output)
*             |
*             | Note: At the first stage, button is in "Idle" state, if the button is pressed,
*             |       it will switch to "Press Evt" and start timing for debounce check, then
*             |       switch to "Press PRE". If the button is release at this time, it will 
*             |       switch back to "Idle". If the button is still pressed and debounce check
*             |       time is out, switch to "Pressed Evt", start long-pressed timing and output
*             |       event to caller, then switch to "Press AFT". If button is released now,
*             |       please refer to No.2 demo. when long-pressed time is out in "Press AFT",
*             |       switch to "Long pressed Evt" and output event to caller. Then switch to
*             |       "Holding". If button is released at this moment, switch to "Long release 
*             |       Evt" and start timing for debounce check. Then switch to "Long Rls". If 
*             |       button is pressed again, swtich back to "Holding". If button is still
*             |       released and debounce time is out, siwtch to "Long released Evt" and 
*             |       output event to caller. At the last, switch to "Idle" for next checking.
*              ------------------------------------------------------------------------------
*             | No.2 Demo of short press and release with debounce check
*             |  ________                             _____________ ________ 
*             |          |  Debounce |               |  Debounce   |
*             |    Idle  | Press PRE |   Press AFT   |  Short Rls  |  Idle         
*             |  (output)|           |   (output)    |             | (output)
*             |          |___________|_______________|             |
*             |          |           |               |             |
*             |          V           V               V             V
*             |     Press Evt    Pressed Evt   Short release Evt   Short release Evt
*             |                   (output)                            (output)
*             |
*             | Note: At the first stage, button is in "Idle" state, if the button is pressed,
*             |       it will switch to "Press Evt" and start timing for debounce check, then
*             |       switch to "Press PRE". If the button is release at this time, it will 
*             |       switch back to "Idle". If the button is still pressed and debounce check
*             |       time is out, switch to "Pressed Evt", start long-pressed timing and output
*             |       event to caller, then switch to "Press AFT". If button is still pressed,
*             |       and long-press time is out, please refer to No.1 demo. If the button is 
*             |       released in "Press AFT", switch to "Short release Evt" and start timing
*             |       for debounce check. Then switch to "Short Rls". If button is pressed again,
*             |       swtich back to "Press AFT". If button is still released and debounce time
*             |       is out, siwtch to "Short released Evt" and output event to caller. At the
*             |       last, switch to "Idle" for next checking.
*              ------------------------------------------------------------------------------
*             | No.3 Demo of long press and release without debounce check
*             |  ________                                    ________
*             |          |               |                  |
*             |    Idle  |  Press AFT    |    Holding       |  Idle   
*             |  (output)|   (output)    |   (output)       | (output)     
*             |          |_______________|__________________|
*             |          |               |                  |
*             |          V               V                  V
*             |     Pressed Evt   Long pressed Evt   Long released Evt
*             |       (output)        (output)             (output)
*             |
*             | Note: At the first stage, button is in "Idle" state, if the button is pressed,
*             |       it will switch to "Pressed Evt", start long-pressed timing and output
*             |       event to caller, then switch to "Press AFT". If button is released now,
*             |       please refer to No.4 demo. when long-pressed time is out in "Press AFT",
*             |       switch to "Long pressed Evt" and output event to caller. Then switch to
*             |       "Holding". If button is released at this moment, switch to "Long released
*             |       Evt" and output event to caller. At the last, switch to "Idle" for next 
*             |       checking.
*              ------------------------------------------------------------------------------
*             | No.4 Demo of short press and release without debounce check
*             |  ________                 ________
*             |          |               |
*             |    Idle  |   Press AFT   |  Idle         
*             |  (output)|   (output)    | (output)
*             |          |_______________|
*             |          |               |
*             |          V               V
*             |     Pressed Evt    Short release Evt
*             |      (output)          (output)
*             |
*             | Note: At the first stage, button is in "Idle" state, if the button is pressed,
*             |       it will switch to "Pressed Evt", start long-pressed timing and output
*             |       event to caller, then switch to "Press AFT". If button is still pressed,
*             |       and long-press time is out, please refer to No.3 demo. If the button is 
*             |       released in "Press AFT", switch to "Short released Evt" and output event
*             |       to caller. At the last, switch to "Idle" for next checking.
*              -----------------------------------------------------------------------------
* Version    : V1.10
* Author     : Ian
* Date       : 15th Jun 2016
******************************************************************************/
uint8 Btn_Channel_Process(uint8 u8Ch, T_BTN_RESULT* ptBtnRes)
{
    uint8 u8TmOut  = 0;
    uint8 u8NextSt = 0x00;  
    uint8 u8BtnSt;

    T_BTN_PARA *ptBtnPara = sg_aptBtnPara[u8Ch - 1];
    T_BTN_ST   *ptBtnSt   = &(sg_atBtnSt[u8Ch - 1]);
        
    /* Check if the channel number is invalid */
    if((0 == u8Ch) || (u8Ch > MAX_BTN_CH))
    {   /* If the channel number is NOT in the range of 1~MAX_BTN_CH, return error */
        return BTN_ERROR;
    }

    /* Check if the button function is enabled or NOT */
    if(ptBtnPara->u8BtnEn != BTN_FUNC_ENABLE)
    {   /* If the function is NOT enabled, return none event and disabled state */
        ptBtnRes->u8Evt   = BTN_NONE_EVT;
        ptBtnRes->u8State = BTN_DIS_ST;
        return SUCCESS;
    }
    /* If the function is enabled, go on */
            
    /* Get the state of button first */
#ifdef __BTN_SM_SPECIFIED_BTN_ST_FN    
    u8BtnSt = ptBtnPara->pfGetBtnSt(u8Ch); /* Use the specified one */
#else
    u8BtnSt = sg_pfGetBtnSt(u8Ch);         /* Use common one        */
#endif
        
    /* If the state invalid */
    if(BTN_ERROR == u8BtnSt)
    {   /* Return error */
        return BTN_ERROR;
    }  

    /************************ Do operations of state ***************************/
    ptBtnRes->u8Evt   = BTN_NONE_EVT;         /* Clear the old event */
    ptBtnRes->u8State = ptBtnSt->u8BtnSt;     /* Fill current state  */
    
    /* If the current state is :           */
    /* Button just pressed event           */
    /* Button just short released event    */
    /* Button just long released event     */
    /* Button pressed totally event        */
    /* Button is long pressed              */
    /* Button short released totally event */
    /* Button long released totally event  */
    if(ptBtnSt->u8BtnSt < BTN_PRESS_PRE_ST)
    {
        ptBtnRes->u8State = cg_aau8StateMachine[ptBtnSt->u8BtnSt][0];    /* Use the next state as result */
        
        /* If the current state is :           */
        /* Button just pressed event           */
        /* Button just short released event    */
        /* Button just long released event     */        
        if(ptBtnSt->u8BtnSt < BTN_PRESSED_EVT)
        {
            ptBtnSt->u16DebounceOldTm = sg_pfGetTm();        /* Start timing debounce time */
        }

        /* If the current state is :           */
        /* Button pressed totally event        */
        /* Button is long pressed              */
        /* Button short released totally event */
        /* Button long released totally event  */
        else
        {
            ptBtnRes->u8Evt = ptBtnSt->u8BtnSt;              /* Update event of result */

            /* If the current state is :       */
            /* Button pressed totally event    */
            if(ptBtnSt->u8BtnSt == BTN_PRESSED_EVT)
            {
                ptBtnSt->u16LongPressOldTm = sg_pfGetTm();   /* Start timing long-press time */
            }
        }
    }

    /* If the current state is :           */
    /* Button is pressed before debounce                   */
    /* Button is released before debounce form short press */
    /* Button is released before debounce form long press  */
    else if(ptBtnSt->u8BtnSt < BTN_IDLE_ST)
    {
        ptBtnRes->u8State += BTN_GO_BACK_OFFSET;    /* Do not provide a debounce state with the result  */ 
        /* Check if debounce time is out */                  
        u8TmOut = ((sg_pfGetTm() - ptBtnSt->u16DebounceOldTm) >= ptBtnPara->u16DebounceTm);   
    }

    /* If the current state is :              */
    /* Button is short pressed after debounce */
    else if(ptBtnSt->u8BtnSt == BTN_PRESS_AFT_ST)
    {   /* Check if long-press time is out */
        u8TmOut = ((sg_pfGetTm() - ptBtnSt->u16LongPressOldTm) >= ptBtnPara->u16LongPressTm);
    }
        
    /* If the current state is :           */
    /* Button is NOT pressed or released   */
    /* Button is long pressed              */
    /* ----------DO NOTHING!!------------- */


    /*************************** Find the Next state ***************************/
    /* Check if button is press or NOT */
    if(u8BtnSt != ptBtnPara->u8NormalSt)
    {   /* If button is pressed, update index number  */
        u8NextSt++;
    }
    
    /* Check if the debounce or long-press time is out or NOT */
    if(u8TmOut)
    {   /* If time is out, update index number */
        u8NextSt += BTN_TM_TRG_EVT_OFFSET;
    }
    
    /************************* Do the state transition *************************/  
    ptBtnSt->u8BtnSt = cg_aau8StateMachine[ptBtnSt->u8BtnSt][u8NextSt];
    
    return SUCCESS;
}


/******************************************************************************
* Name       : uint8 Btn_SM_Easy_Init(PF_GET_TM pfGetTm, PF_GET_BTN pfGetBtnSt)
* Function   : Easy init operation of button state machine for quick start.
* Input      : PF_GET_TM pfGetTm       Function to get gengeral time
*              PF_GET_BTN pfGetBtnSt   Function to get button state(0/1)
* Output:    : None
* Return     : BTN_ERROR               Input parameter is invalid
*              SUCCESS                 Init operation is successed
* description: This function should be called once before use button checking.
*              This function get two necessary interface function: 
*              "pfGetTm()":Function to get gengeral time;
*              "pfGetBtnSt":Function to get button state(0/1). (if the MACRO 
*              __BTN_SM_SPECIFIED_BTN_ST_FN is NOT defined)
*
*              NOTE:If the esay init is failed, DO NOT continue!!
* Version    : V1.00
* Author     : Ian
* Date       : 15th Jun 2016
******************************************************************************/
uint8 Btn_SM_Easy_Init(PF_GET_TM pfGetTm, PF_GET_BTN pfGetBtnSt)
{
    static T_BTN_PARA s_atBtnPara[MAX_BTN_CH];
    uint8 u8Idx,u8Temp;

    /* Button general init */
    u8Temp = Btn_General_Init(pfGetTm, pfGetBtnSt);
    if(BTN_ERROR == u8Temp)
    {
        return BTN_ERROR;
    }
    
    for(u8Idx = 0; u8Idx < MAX_BTN_CH; u8Idx++)
    {      
        /* Configure the button parameters */
        s_atBtnPara[u8Idx].u8Ch           = u8Idx + 1;       /* Channel number                  */
        s_atBtnPara[u8Idx].u16DebounceTm  = 50;              /* Debounce time is 50 ms          */
        s_atBtnPara[u8Idx].u16LongPressTm = 1000;            /* Long-press time is 1000ms       */
        s_atBtnPara[u8Idx].u8NormalSt     = 0;               /* The normal state of button is 0 */
        s_atBtnPara[u8Idx].u8BtnEn        = BTN_FUNC_ENABLE; /* Enable button at the beginning  */
#ifdef __BTN_SM_SPECIFIED_BTN_ST_FN
        s_atBtnPara[u8Idx].pfGetBtnSt     = pfGetBtnSt;      /* Function to get button state    */
#endif

        /* Button channels init */
        Btn_Channel_Init(u8Idx + 1 ,&(s_atBtnPara[u8Idx]));
    }
    return SUCCESS;
}


/* end-of-file */


