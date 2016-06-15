/******************************************************************************
* File       : Btn_SM_Module.c
* Function   : Check button state and return short/long press or release
* description: Button state machine module. It realise button short/long press or
*              release checking and return responding events,Debounce checking can
*              be enable/disable, and debounce time can be configured by "u16DebounceTm".
*              Meanwhile, the time for long press distinguish can be set by "u16LongPressTm".
*              __________
*              HOW TO USE: 
*              Step 1: Modify MAX_BTN_CH in Btn_SM_Module.h with desired button numbers
*              Step 2: Copy Demo code in Btn_SM_Module.h to upper layer, Create parameter 
*                      structures and init them with desired parameters.
*              Step 3: Create a "uint8 Get_General_Time()" function to get General time
*              Step 4: Create a "uint8 Get_Button_Status(uint8 u8Ch)" function to get button
*                      status(0/1).
*              Step 5: Call "Btn_General_Init()" first to get previous interface functions.
*                      If defined __BTN_SM_SPECIFIED_BTN_ST_FN, and NULL is used for input
*                      parameter " PF_GET_BTN pfGetBtnSt", each button will use specified
*                      Button state get function.
*              Step 6: Call "Btn_Channel_Init()" per channel to init every button channel.
*              Step 7: Poll "Btn_Channel_Process()" per channel to check button state.
*              Button function can be enabled/disabled by calling "Btn_Func_En_Dis()".              
* Version    : V1.00
* Author     : Ian
* Date       : 27th Jan 2016
******************************************************************************/

#include "common.h"
#include "Btn_SM_Module.h"

const uint8 cg_aau8StateMachine[13][4] = 
{
    /*  Situation 1  */    /*  Situation 2 */    /*  Situation 3  */     /* Situation 4  */
    /* Btn NOT press */    /* Btn press    */    /* Btn NOT press */     /* Btn press    */
    /* Time NOT out  */    /* Time NOT out */    /* Time out      */     /* Time out     */
    {BTN_IDLE_ST         , BTN_PRESS_EVT       , BTN_IDLE_ST          ,  BTN_PRESS_EVT       },    /* BTN_IDLE             */  
    {BTN_PRESS_PRE_ST    , BTN_PRESS_PRE_ST    , BTN_PRESS_PRE_ST     ,  BTN_PRESS_PRE_ST    },    /* BTN_PRESS_EVT        */
    {BTN_IDLE_ST         , BTN_PRESS_PRE_ST    , BTN_IDLE_ST          ,  BTN_PRESSED_EVT     },    /* BTN_PRESS_PRE        */
    {BTN_PRESS_AFT_ST    , BTN_PRESS_AFT_ST    , BTN_PRESS_AFT_ST     ,  BTN_PRESS_AFT_ST    },    /* BTN_PRESSED_EVT      */
    {BTN_S_RELEASE_EVT   , BTN_PRESS_AFT_ST    , BTN_S_RELEASE_EVT    ,  BTN_LONG_PRESSED_EVT},    /* BTN_PRESS_AFT        */
    {BTN_HOLDING_ST      , BTN_HOLDING_ST      , BTN_HOLDING_ST       ,  BTN_HOLDING_ST      },    /* BTN_LONG_PRESSED_EVT */
    {BTN_L_RELEASE_EVT   , BTN_HOLDING_ST      , BTN_L_RELEASE_EVT    ,  BTN_HOLDING_ST      },    /* BTN_HOLDING          */
    {BTN_SHORT_RELEASE_ST, BTN_SHORT_RELEASE_ST, BTN_SHORT_RELEASE_ST ,  BTN_SHORT_RELEASE_ST},    /* BTN_S_RELEASE_EVT    */
    {BTN_LONG_RELEASE_ST , BTN_LONG_RELEASE_ST , BTN_LONG_RELEASE_ST  ,  BTN_LONG_RELEASE_ST },    /* BTN_L_RELEASE_EVT    */ 
    {BTN_SHORT_RELEASE_ST, BTN_PRESS_AFT_ST    , BTN_S_RELEASED_EVT   ,  BTN_PRESS_AFT_ST    },    /* BTN_SHORT_RELEASE    */
    {BTN_LONG_RELEASE_ST , BTN_HOLDING_ST      , BTN_L_RELEASED_EVT   ,  BTN_HOLDING_ST      },    /* BTN_LONG_RELEASE     */
    {BTN_IDLE_ST         , BTN_IDLE_ST         , BTN_IDLE_ST          ,  BTN_IDLE_ST         },    /* BTN_S_RELEASED_EVT   */
    {BTN_IDLE_ST         , BTN_IDLE_ST         , BTN_IDLE_ST          ,  BTN_IDLE_ST         }     /* BTN_L_RELEASED_EVT   */
};


static T_BTN_PARA *sg_aptBtnPara[MAX_BTN_CH] = {0};          /* Parameter interface          */
static T_BTN_ST    sg_atBtnSt[MAX_BTN_CH]    = {0};          /* Running status               */
static PF_GET_TM   sg_pfGetTm                = NULL;         /* Function to get general time */
static PF_GET_BTN  sg_pfGetBtnSt             = NULL;         /* Function to get button state */

static T_BTN_RESULT sg_tBtnRes;


/******************************************************************************
* Name       : void Btn_Func_En_Dis(uint8 u8Ch, uint8 u8EnDis)
* Function   : Enable or disable button function
* Input      : uint8 u8Ch       1~255               The number of setting button channel
*              uint8 u8EnDis    BTN_FUNC_ENABLE     Enable the button function
*                               BTN_FUNC_DISABLE    Disable the button function
* Output:    : None
* Return     : None
* description: If the button function is enabled, the button events can be returned
*              according to the button operation; If the button function is disabled
*              There will be no event being returned to the upper layer even button
*              is operated. 
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
* description: This function should be called once before use button checing.
*              This function get two necessary interface function: 
*              "pfGetTm()":Function to get gengeral time;
*              "pfGetBtnSt":Function to get button state(0/1).
*              If the general init is failed, DO NOT continue!!
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
    

    /* If the "Get button state" function has NOT registered yet */
    if(NULL == sg_pfGetBtnSt)
    {   /* Register the function */
        sg_pfGetBtnSt = pfGetBtnSt;
    }
    /* If the "Get button state" function has already registered, Do NOT re-regiter */
    
    return SUCCESS;
}

/******************************************************************************
* Name       : uint8 Btn_Channel_Init(uint8 u8Ch ,T_BTN_PARA *ptBtnPara)
* Function   : Init operation for each button channel
* Input      : uint8 u8Ch             1~255      The number of setting button channel
*              T_BTN_PARA *ptBtnPara             Parameter of each button channel
* Output:    : None
* Return     : BTN_ERROR        Input parameter is invalid
*              SUCCESS          Init operation is successed
* description: After general init operation, call this function for each channel.
*              Call this function before button checking,  If the channel init 
*              is failed, DO NOT continue!!
*              This function save the parameter structure pointer for further 
*              operation, and init running state of such channel.
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

    sg_aptBtnPara[u8Ch - 1] = ptBtnPara;                  /* Get the parameters              */

    sg_atBtnSt[u8Ch - 1].u8BtnSt           = BTN_IDLE_ST; /* Init the state of state machine */
    sg_atBtnSt[u8Ch - 1].u16DebounceOldTm  = 0;           /* Reset debounce timer            */
    sg_atBtnSt[u8Ch - 1].u16LongPressOldTm = 0;           /* Reset long press timer          */
    
    return SUCCESS;
}

/******************************************************************************
* Name       : T_BTN_RESULT* Btn_Channel_Process(uint8 u8Ch)
* Function   : Main process of button checking
* Input      : uint8 u8Ch             1~255      The number of setting button channel
* Output:    : None
* Return     : T_BTN_RESULT*                     State and event of button. 
* description: This function should be called after general init and channel init.
*              This function should be polled to check button state.
*              ------------------------------------------------------------------------------
*             | No.1 Demo of long press with debounce check
*             |           ___________ _________________ _______________ 
*             |          |  Debounce |                 |               | Debounce |
*             |    Idle  | Press PRE |   Press AFT     |    Holding    | Long Rls |  Idle         
*             |  ________|           |                 |               |__________|________
*             |                      |                 |                          |
*             |                      V                 V                          V
*             |                  Press Evt        Long press Evt            Long release Evt
*             |
*             | Note: At the first stage, button is in "idle" state, if the button is pressed,
*             |       it will switch to "press pre" state which is used to check debounce. If
*             |       button is released in this state, it will switch back to "idle" state.
*             |       If the button is still pressed and the debounce check time is over, 
*             |       function will returns a "Press event" and switch to "press after" state 
*             |       in which state, long press will be checked here. If button is released 
*             |       at this time, please see No.2 Demo; If button is still pressed and long
*             |       press time is over, function will return a "Long press event" and switch
*             |       to "Holding" state, in"holding" state it will wait button releasing, if
*             |       button is released, it will switch to "long release" state to do debounce
*             |       Checking. If button is re-pressed again, it will switch back to "Holding"
*             |       state; If button is still released, function returns a "Long release event"
*             |       and switch to "idle" state for next button checking.
*              ------------------------------------------------------------------------------
*             | No.2 Demo of short press with debounce check
*             |           ___________ ______________ 
*             |          |  Debounce |              | Debounce |
*             |    Idle  | Press PRE |   Press AFT  | Short Rls|  Idle         
*             |  ________|           |              |__________|________
*             |                      |                         |
*             |                      V                         V
*             |                  Press Evt              Short release Evt
*             |
*             | Note: At the first stage, button is in "idle" state, if the button is pressed,
*             |       it will switch to "press pre" state which is used to check debounce. If
*             |       button is released in this state, it will switch back to "idle" state.
*             |       If the button is still pressed and the debounce check time is over, 
*             |       function will returns a "Press event" and switch to "press after" state 
*             |       in which state, long press will be checked here. If button is still 
*             |       pressed and long press time is over, please see No.1 Demo; If button is
*             |       released at this time, it will switch to "short release" state to do 
*             |       debounce Checking. If button is re-pressed again, it will switch back 
*             |       to "press after" state; If button is still released, function returns a 
*             |       "Short release event" and switch to "idle" state for next button checking.
*              ------------------------------------------------------------------------------
*             | No.3 Demo of long press WITHOUT debounce check
*             |           _________________ _______________ 
*             |          |                 |               |
*             |    Idle  |   Press AFT     |    Holding    |  Idle         
*             |  ________|                 |               |________
*             |          |                 |               |
*             |          V                 V               V
*             |       Press Evt       Long press Evt    Long release Evt
*             |
*             | Note: At the first stage, button is in "idle" state, if the button is pressed,
*             |       function will returns a "Press event" and switch to "press after" state 
*             |       in which state, long press will be checked here. If button is released 
*             |       at this time, please see No.4 Demo; If button is still pressed and long
*             |       press time is over, function will return a "Long press event" and switch
*             |       to "Holding" state, in "holding" state it will wait button releasing, if
*             |       button is released, function returns a "Long release event" and switch 
*             |       to "idle" state for next button checking.
*              -----------------------------------------------------------------------------
*             | No.4 Demo of short press WITHOUT debounce check
*             |           ______________ 
*             |          |              |
*             |    Idle  |   Press AFT  |  Idle         
*             |  ________|              |________
*             |          |              |
*             |          V              V
*             |      Press Evt      Short release Evt
*             |
*             | Note: At the first stage, button is in "idle" state, if the button is pressed,
*             |       function will returns a "Press event" and switch to "press after" state 
*             |       in which state, long press will be checked here. If button is still 
*             |       pressed and long press time is over, please see No.3 Demo; If button is
*             |       released at this time, function returns a "Short release event" and 
*             |       switch to "idle" state for next button checking.
*              -----------------------------------------------------------------------------
* Version    : V1.00
* Author     : Ian
* Date       : 27th Jan 2016
******************************************************************************/
T_BTN_RESULT* Btn_Channel_Process(uint8 u8Ch)
{
    uint8 u8TmOut  = 0;
    uint8 u8NextSt = 0x00;  
    uint8 u8BtnSt  = 0;

    T_BTN_PARA *ptBtnPara = sg_aptBtnPara[u8Ch - 1];
    T_BTN_ST   *ptBtnSt   = &(sg_atBtnSt[u8Ch - 1]);
        
    /* Check if the channel number is invalid */
    if((0 == u8Ch) || (u8Ch > MAX_BTN_CH))
    {   /* If the channel number is NOT in the range of 1~MAX_BTN_CH, return error */
        return NULL;
    }

    /* Check if the button function is enabled or NOT */
    if(ptBtnPara->u8BtnEn != BTN_FUNC_ENABLE)
    {   /* If the function is NOT enabled, return none event and disabled state */
        sg_tBtnRes.u8Evt   = BTN_NONE_EVT;
        sg_tBtnRes.u8State = BTN_DIS_ST;
        return &sg_tBtnRes;
    }
    /* If the function is enabled, go on */
            
    /* Get the state of button first */
    if(NULL != sg_pfGetBtnSt)                  /* If the general button state function is registered     */
    {
        u8BtnSt = sg_pfGetBtnSt(u8Ch);         /* Use the general one  */
    }
#ifdef __BTN_SM_SPECIFIED_BTN_ST_FN
    else                                       /* If the general button state function is NOT registered */   
    { 
        u8BtnSt = ptBtnPara->pfGetBtnSt(u8Ch); /* Use the specified one */
    }
#endif
        
    /* If the state invalid */
    if(BTN_ERROR == u8BtnSt)
    {   /* Return error */
        return NULL;
    }  

    switch(ptBtnSt->u8BtnSt)
    {
        case BTN_PRESS_EVT: 
        case BTN_S_RELEASE_EVT: 
        case BTN_L_RELEASE_EVT:
        { 
            ptBtnSt->u16DebounceOldTm  = sg_pfGetTm();   
            break;
        }
        case BTN_PRESSED_EVT: 
        { 
            ptBtnSt->u16LongPressOldTm = sg_pfGetTm();
            /* No break here */
        }
        case BTN_LONG_PRESSED_EVT:
        case BTN_S_RELEASED_EVT:
        case BTN_L_RELEASED_EVT:
        {
            sg_tBtnRes.u8Evt   = ptBtnSt->u8BtnSt;
            sg_tBtnRes.u8State = cg_aau8StateMachine[ptBtnSt->u8BtnSt][0];
            break;
        }
        case BTN_PRESS_AFT_ST:
        {   
            u8TmOut = ((sg_pfGetTm() - ptBtnSt->u16LongPressOldTm) >= ptBtnPara->u16LongPressTm);
            /* No break here */
        }
        case BTN_IDLE_ST:    
        case BTN_HOLDING_ST:     
        {
            sg_tBtnRes.u8State = ptBtnSt->u8BtnSt;
            sg_tBtnRes.u8Evt   = BTN_NONE_EVT;
            break;
        }                                    
        case BTN_PRESS_PRE_ST:                                                    
        case BTN_SHORT_RELEASE_ST:                                                                                     
        case BTN_LONG_RELEASE_ST:
        {
           u8TmOut = ((sg_pfGetTm() - ptBtnSt->u16DebounceOldTm) >= ptBtnPara->u16DebounceTm);
           sg_tBtnRes.u8Evt = BTN_NONE_EVT;
           break;
        }
        default:              
        {                                                           
            break;
        }
    }
    

    if(u8BtnSt != ptBtnPara->u8NormalSt)
    {
      u8NextSt++;
    }
    
    if(u8TmOut)
    {
      u8NextSt += 2;
    }
      
    ptBtnSt->u8BtnSt = cg_aau8StateMachine[ptBtnSt->u8BtnSt][u8NextSt];
    
    return &sg_tBtnRes;
}





/* end-of-file */

//u8BtnValue = !(GPIOA_PDIR & (1 << 5));


