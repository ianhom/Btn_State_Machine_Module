/******************************************************************************
* File       : Btn_SM_Module.h
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



#ifndef _BTN_SM_MODULE_
#define _BTN_SM_MODULE_

#ifdef __cplusplus
extern "C" {
#endif
  
#ifndef MAX_BTN_CH
#define MAX_BTN_CH                   (1)         /* Max number of buttons, please define it in upper layer */
#endif

#define BTN_SM_SPECIFIED_BTN_ST_FN               /* Use specified button state get function             */

/* States of button state machine */
#define BTN_IDLE                     (0)         /* Button is NOT pressed or released                   */
#define BTN_PRESS_PRE                (1)         /* Button is pressed before debounce                   */
#define BTN_PRESS_AFT                (2)         /* Button is short pressed after debounce              */
#define BTN_HOLDING                  (4)         /* Button is long pressed                              */
#define BTN_SHORT_RELEASE            (5)         /* Button is released before debounce form short press */
#define BTN_LONG_RELEASE             (6)         /* Button is released before debounce form long press  */

/* Return events */
#define BTN_NONE_EVENT               (0)         /* There is None button operation         */
#define BTN_PRESSED_EVENT            (1)         /* Button is pressed from idle state      */
#define BTN_LONG_PRESSED_EVENT       (2)         /* Button is long pressed                 */
#define BTN_SHORT_RELEASED_EVENT     (3)         /* Button is released before long pressed */
#define BTN_LONG_RELEASED_EVENT      (4)         /* Button is released after long pressed  */

#define SUCCESS                      (0)         /* Correct condition                 */
#define BTN_ERROR                    (0xFF)      /* Error condition                   */

/* Parameters */
#define BTN_FUNC_ENABLE              (1)         /* Button input function is enabled  */
#define BTN_FUNC_DISABLE             (0)         /* Button input function is disabled */

#define BTN_NORMAL_0                 (0)         /* The normal state of button is "0" */
#define BTN_NORMAL_1                 (1)         /* The normal state of button is "1" */

#define BTN_STATE_0                  (0)         /* The state of button is "0"        */
#define BTN_STATE_1                  (1)         /* The state of button is "1"        */

/* Function type definition */
/******************************************************************************
* Name       : uint8  (*)(uint8 u8Ch)
* Function   : Get the button state 0/1
* Input      : uint8 u8Ch       1~255     The number of setting button channel
* Output:    : None
* Return     : BTN_STATE_0      Button state is logic "0" 
*              BTN_STATE_1      Button state is logic "1"
*              BTN_ERROR        Failed to get button state.
* description: This function should return the button state according to the channel number
* Version    : V1.00
* Author     : Ian
* Date       : 27th Jan 2016
******************************************************************************/
typedef uint8  (*PF_GET_BTN)(uint8 u8Ch);      

/******************************************************************************
* Name       : uint16 (*)()
* Function   : Get the time of General time
* Input      : None
* Output:    : None
* Return     : 0~65535  Free running general clock time for system(for example, 1ms timer)
* description: This function should return the general clock time.
* Version    : V1.00
* Author     : Ian
* Date       : 27th Jan 2016
******************************************************************************/
typedef uint16 (*PF_GET_TM)();      


/* Structure type definition */
/*******************************************************************************
* Structure  : T_BTN_PARA
* Description: Structure of button state machine parameters.
* Memebers   : Type    Member          Range             Descrption     
               PF_GET_BTN  pfGetBtnSt                    Function to get button state    
               uint16  u16LongPressTm  0~65535           Time units for long press distinguish
               uint16  u16DebounceTm   0~65535           Time uints for debounce check        
               uint8   u8BtnEn         BTN_FUNC_ENABLE   Enable button function 
                                       BTN_FUNC_DISABLE  Disable button function    
               uint8   u8NormalSt      BTN_NORMAL_0      The normal state of button is "0"
                                       BTN_NORMAL_1      The normal state of button is "1"
               uint8   u8Ch            1~255             Channel number of button       
*******************************************************************************/
typedef struct _T_BTN_PARA_
{
#ifdef __BTN_SM_SPECIFIED_BTN_ST_FN
    PF_GET_BTN  pfGetBtnSt;         /* Function to get button state    */
#endif
    uint16      u16LongPressTm;     /* Time for long press distinguish */
    uint16      u16DebounceTm;      /* Time for debounce check         */
    uint8       u8BtnEn;            /* Enable or disable function      */
    uint8       u8NormalSt;         /* Normal(stable) state of button  */
    uint8       u8Ch;               /* Channel number of button        */
}T_BTN_PARA;


/*******************************************************************************
* Structure  : T_BTN_ST
* Description: Structure of button running status.
* Memebers   : Type    Member             Range              Descrption     
               uint16  u16DebounceOldTm   0~65535            The start time of debounce check   
               uint16  u16LongPressOldTm  0~65535            The start time of long press check 
               uint8   u8BtnSt            BTN_IDLE           Button is NOT pressed or released           
                                          BTN_PRESS_PRE      Button is pressed before debounce                   
                                          BTN_PRESS_AFT      Button is short pressed after debounce              
                                          BTN_HOLDING        Button is long pressed                              
                                          BTN_SHORT_RELEASE  Button is released before debounce form short press 
                                          BTN_LONG_RELEASE   Button is released before debounce form long press  
*******************************************************************************/
typedef struct _T_BTN_ST_
{
    uint16  u16DebounceOldTm;       /* The start time of debounce check   */
    uint16  u16LongPressOldTm;      /* The start time of long press check */
    uint8   u8BtnSt;                /* The state of state machine         */
}T_BTN_ST;




/* Function declaration */
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
void Btn_Func_En_Dis(uint8 u8Ch, uint8 u8EnDis);

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
uint8 Btn_General_Init(PF_GET_TM pfGetTm, PF_GET_BTN pfGetBtnSt);

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
uint8 Btn_Channel_Init(uint8 u8Ch ,T_BTN_PARA *ptBtnPara);

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
uint8 Btn_Channel_Process(uint8 u8Ch);



#if(0) /* Example for how to create button object, COPY and MODIFY follow code to upper layer */

/* Create button parameter structure */
static T_BTN_PARA sg_atBtnPara[MAX_BTN_CH] = {0};

/* Init all parameter */
void Btn_SM_Module_Ctor()
{   /* Init all channel */
    for(uint8 u8i = 0; u8i < MAX_BTN_CH; u8i++)
    {   /* Modify follow parameter for actual application */
        sg_atBtnPara[u8i].u8BtnEn        = BTN_FUNC_ENABLE;   /* Button is enabled at the first time      */
        sg_atBtnPara[u8i].u8NormalSt     = BTN_NORMAL_1;      /* The stable state of button is logic "1"  */
        sg_atBtnPara[u8i].u16DebounceTm  = 10;                /* Debounce checking time is 10 units       */
        sg_atBtnPara[u8i].u16LongPressTm = 100;               /* Long press distinguish time is 100 units */
    }
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BTN_SM_MODULE_ */




