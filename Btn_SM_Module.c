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
*                      parameter " PF_GET_BTN pfGetBtnSt", each button will use sepcified
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

static T_BTN_PARA *sg_aptBtnPara[MAX_BTN_CH] = {0};          /* Parameter interface          */
static T_BTN_ST    sg_atBtnSt[MAX_BTN_CH]    = {0};          /* Running status               */
static PF_GET_TM   sg_pfGetTm                = NULL;         /* Function to get general time */
static PF_GET_BTN  sg_pfGetBtnSt             = NULL;         /* Function to get button state */

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
    sg_aptBtnPara[u8Ch - 1]->u8BtnEn = u8EnDis;   /* Enable or Disable the button functions */
    sg_atBtnSt[u8Ch - 1].u8BtnSt     = BTN_IDLE;  /* Reset the state machine of button      */
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
    if((NULL == pfGetTm) || (NULL == pfGetBtnSt))
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

    sg_aptBtnPara[u8Ch - 1] = ptBtnPara;                 /* Get the parameters              */

    sg_atBtnSt[u8Ch - 1].u8BtnSt           = BTN_IDLE;   /* Init the state of state machine */
    sg_atBtnSt[u8Ch - 1].u16DebounceOldTm  = 0;          /* Reset debounce timer            */
    sg_atBtnSt[u8Ch - 1].u16LongPressOldTm = 0;          /* Reset long press timer          */
    
    return SUCCESS;
}

/******************************************************************************
* Name       : uint8 Btn_Channel_Process(uint8 u8Ch)
* Function   : Main process of button checking
* Input      : uint8 u8Ch             1~255      The number of setting button channel
* Output:    : None
* Return     : BTN_ERROR                 Input parameter is invalid
*              BTN_NONE_EVENT            There is None button operation        
*              BTN_PRESSED_EVENT         Button is pressed from idle state      
*              BTN_LONG_PRESSED_EVENT    Button is long pressed                 
*              BTN_SHORT_RELEASED_EVENT  Button is released before long pressed 
*              BTN_LONG_RELEASED_EVENT   Button is released after long pressed  
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
uint8 Btn_Channel_Process(uint8 u8Ch)
{
    uint8 u8Return = BTN_NONE_EVENT;
    uint8 ucBtnSt;
    T_BTN_PARA *ptBtnPara = sg_aptBtnPara[u8Ch - 1];
    T_BTN_ST   *ptBtnSt   = &(sg_atBtnSt[u8Ch - 1]);
    
    /* Check if the channel number is invalid */
    if((0 == u8Ch) || (u8Ch > MAX_BTN_CH))
    {   /* If the channel number is NOT in the range of 1~MAX_BTN_CH, return error */
        return BTN_ERROR;
    }

    /* Check if the button function is enabled or NOT */
    if(ptBtnPara->u8BtnEn != BTN_FUNC_ENABLE)
    {   /* If the function is NOT enabled, return none event */
        return BTN_NONE_EVENT;
    }
    /* If the function is enabled, go on */
        
    /* Get the state of button first */
    if(NULL != sg_pfGetBtnSt)                  /* If the general button state function is registered     */
    {
        ucBtnSt = sg_pfGetBtnSt(u8Ch);         /* Use the general one  */
    }
#ifdef __BTN_SM_SPECIFIED_BTN_ST_FN
    else                                       /* If the general button state function is NOT registered */   
    { 
        ucBtnSt = ptBtnPara->pfGetBtnSt(u8Ch); /* Use the specified one */
    }
#endif
    
    /* If the state invalid */
    if(BTN_ERROR == ucBtnSt)
    {   /* Return error */
        return BTN_ERROR;
    }

    /* Check state of button state machine */
    switch(ptBtnSt->u8BtnSt)
    {
        /* If the button is in idle state */
        case BTN_IDLE:
        {   /* If the button is pressed */
            if(ptBtnPara->u8NormalSt != ucBtnSt)
            {   /* If debounce check time is necessary */
                if(ptBtnPara->u16DebounceTm)
                {
                    ptBtnSt->u8BtnSt          = BTN_PRESS_PRE;      /* Switch the state to BTN_PRESS_PRE for debounce checking */
                    ptBtnSt->u16DebounceOldTm = sg_pfGetTm();       /* Get the start time of debounce checking                 */
                }
                else/* If debounce check time is 0 */
                {
                    ptBtnSt->u8BtnSt           = BTN_PRESS_AFT;     /* Switch the state to BTN_PRESS_AFT for long press checking */
                    ptBtnSt->u16LongPressOldTm = sg_pfGetTm();      /* Get the start time of long press checking                 */
                    u8Return                   = BTN_PRESSED_EVENT; /* Return Button press event                                 */
                }
            }
            /* If the button is NOT pressed, stay in idle state */
            break;
        }

        /* If the buttion is just pressed from the idle state, check debounce */
        case BTN_PRESS_PRE:
        {   /* If the button is still pressed */        
            if(ptBtnPara->u8NormalSt != ucBtnSt)
            {   /* If the debounce check time is over */
                if((sg_pfGetTm() - ptBtnSt->u16DebounceOldTm) >= ptBtnPara->u16DebounceTm)
                {
                    ptBtnSt->u8BtnSt           = BTN_PRESS_AFT;     /* Switch the state to BTN_PRESS_AFT for long press checking */
                    ptBtnSt->u16LongPressOldTm = sg_pfGetTm();      /* Get the start time of long press checking                 */
                    u8Return                   = BTN_PRESSED_EVENT; /* Return Button press event                                 */
                }
                /* Else if the debounce check time is NOT over, stay in such state */
            }
            else/* If the button is NOT pressed any more */
            {
                ptBtnSt->u8BtnSt = BTN_IDLE;                        /* Switch the state back to the idle */
            }
            break;
        }

        /* If the button is still pressed after debounce checking time, check long press time */
        case BTN_PRESS_AFT:
        {   /* If the button is still pressed */
            if(ptBtnPara->u8NormalSt != ucBtnSt)
            {   /* If the long press time is over */
                if((sg_pfGetTm() - ptBtnSt->u16LongPressOldTm) >= ptBtnPara->u16LongPressTm)
                { 
                    ptBtnSt->u8BtnSt = BTN_HOLDING;                 /* Switch the state to BTN_HOLDING to wait release after long press */
                    u8Return         = BTN_LONG_PRESSED_EVENT;      /* Return Long press event                                          */

                }
                /* Else if the long press time is NOT over, stay in such state */
            }
            else/* If the button is NOT pressed any more */
            {   /* If debounce check time is necessary */
                if(ptBtnPara->u16DebounceTm)
                {
                    ptBtnSt->u16DebounceOldTm = sg_pfGetTm();         /* Get the start time of short release debounce time */
                    ptBtnSt->u8BtnSt          = BTN_SHORT_RELEASE;    /* Switch to BTN_SHORT_RELEASE to check debounce     */
                }
                else
                {
                    u8Return         = BTN_SHORT_RELEASED_EVENT;      /* Return "release after short press" event          */
                    ptBtnSt->u8BtnSt = BTN_IDLE;                      /* Switch to BTN_IDLE for next button check          */
                }
            }
            break;
        }

        /* If the button is still pressed after long press time, check release event */
        case BTN_HOLDING:
        {   /* If the button is released */
            if(ptBtnPara->u8NormalSt == ucBtnSt)
            {   /* If debounce check time is necessary */
                if(ptBtnPara->u16DebounceTm)
                {
                    ptBtnSt->u16DebounceOldTm = sg_pfGetTm();            /* Get the start time of release debounce       */
                    ptBtnSt->u8BtnSt          = BTN_LONG_RELEASE;        /* Switch to BTN_LONG_RELEASE to check debounce */
                }
                else/* If the debounce check time is 0 */
                {
                    ptBtnSt->u8BtnSt          = BTN_IDLE;                /* Switch to BTN_IDLE for next button check */
                    u8Return                  = BTN_LONG_RELEASED_EVENT; /* Return release after long pressed event  */
                }
            }
            /* If the button is still pressed, stay in such state */
            break;
        }

        /* If the button is relased before the long press, check release debounce */
        case BTN_SHORT_RELEASE:
        {   /* If button is still released*/
            if(ptBtnPara->u8NormalSt == ucBtnSt)
            {   /* If the release debounce time is over */
                if((sg_pfGetTm() - ptBtnSt->u16DebounceOldTm) >= ptBtnPara->u16DebounceTm)
                {
                    u8Return         = BTN_SHORT_RELEASED_EVENT;      /* Return "release after short press" event          */
                    ptBtnSt->u8BtnSt = BTN_IDLE;                      /* Switch to BTN_IDLE for next button check          */
                }
                /* If the release debounce time is NOT over, stay in such state */
            }
            else/* If button is pressed again */
            {
                ptBtnSt->u8BtnSt     = BTN_PRESS_AFT;                 /* Switch back to BTN_PRESS_AFT to check long press */
            }
            break;
        }
        
        /* If the button is released after long pressed, check release debounce */
        case BTN_LONG_RELEASE:
        {   /* If the button is still released */
            if(ptBtnPara->u8NormalSt == ucBtnSt)
            {   /* If the release debounce time is over */
                if((sg_pfGetTm() - ptBtnSt->u16DebounceOldTm) >= ptBtnPara->u16DebounceTm)
                {
                    u8Return         = BTN_LONG_RELEASED_EVENT;     /* Return "release after long press" event  */
                    ptBtnSt->u8BtnSt = BTN_IDLE;                    /* Switch to BTN_IDLE for next button check */
                }
                /* If the release debounce time is NOT over, stay in such state */
            }
            else/* If the button is pressed again */
            {
                ptBtnSt->u8BtnSt = BTN_HOLDING;                     /* Switch back to BTN_HOLDING state         */
            }
            break;
        }
    }
        
    return u8Return;
}



/* end-of-file */


