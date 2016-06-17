/******************************************************************************
* File       : Btn_SM_Module.h
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




#ifndef _BTN_SM_MODULE_
#define _BTN_SM_MODULE_

#ifdef __cplusplus
extern "C" {
#endif
  
#ifndef MAX_BTN_CH
#define MAX_BTN_CH                   (1)         /* Max number of buttons, please define it in upper layer */
#endif

#define BTN_STATE_NUM                (13)        /* The number of states in state machine               */
#define BTN_TRG_NUM                  (4)         /* The number of trigger event in state machine        */

/* States of button state machine */
#define BTN_PRESS_EVT                (0)         /* Button just pressed event                           */
#define BTN_S_RELEASE_EVT            (1)         /* Button just short released event                    */
#define BTN_L_RELEASE_EVT            (2)         /* Button just long released event                     */
#define BTN_PRESSED_EVT              (3)         /* Button pressed totally event                        */
#define BTN_LONG_PRESSED_EVT         (4)         /* Button is long pressed                              */
#define BTN_S_RELEASED_EVT           (5)         /* Button short released totally event                 */
#define BTN_L_RELEASED_EVT           (6)         /* Button long released totally event                  */

#define BTN_PRESS_PRE_ST             (7)         /* Button is pressed before debounce                   */
#define BTN_SHORT_RELEASE_ST         (8)         /* Button is released before debounce form short press */
#define BTN_LONG_RELEASE_ST          (9)         /* Button is released before debounce form long press  */
#define BTN_IDLE_ST                  (10)        /* Button is NOT pressed or released                   */
#define BTN_PRESS_AFT_ST             (11)        /* Button is short pressed after debounce              */
#define BTN_HOLDING_ST               (12)        /* Button is long pressed                              */
#define BTN_NONE_EVT                 (13)        /* No event with the button                            */
#define BTN_DIS_ST                   (14)        /* Button is disabled                                  */

#define BTN_GO_BACK_OFFSET           (3)         /* Offset betwen debounce state and previous ones      */
#define BTN_TM_TRG_EVT_OFFSET        (2)         /* Offset for time out trigger in state table          */

#define SUCCESS                      (0)         /* Correct condition                 */
#define BTN_ERROR                    (0xFF)      /* Error condition                   */

#ifdef NULL
#undef NULL
#define NULL                         (0)         /* Null type potinter                                  */
#endif


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
* Structure  : T_BTN_RESULT
* Description: Structure of button result: Event & State.
* Memebers   : Type    Member   Range                 Descrption     
*              uint8   u8Evt    BTN_PRESSED_EVT       Button is just short pressed
*                               BTN_LONG_PRESSED_EVT  Button is just long pressed
*                               BTN_S_RELEASED_EVT    Button is just released from short press
*                               BTN_L_RELEASED_EVT    Button is just released from long press
*              uint8   u8State  BTN_IDLE_ST           Button is in idle state
*                               BTN_PRESS_AFT_ST      Button is in short pressed state
*                               BTN_HOLDING_ST        Button is in long pressed state
*                               BTN_DIS_ST            Butoon is disabled
*******************************************************************************/
typedef struct _T_BTN_RESULT
{
    uint8       u8Evt;         /* Event of button */
    uint8       u8State;       /* State of button */
}T_BTN_RESULT;


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
* description: If the button function is enabled, the button events and states could
*              be returned according to the button operation; If the button function 
*              is disabled, the returned event and state will be BTN_NONE_EVT and 
*              BTN_DIS_STeven button is operated. 
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
uint8 Btn_General_Init(PF_GET_TM pfGetTm, PF_GET_BTN pfGetBtnSt);

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
uint8 Btn_Channel_Init(uint8 u8Ch ,T_BTN_PARA *ptBtnPara);

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
uint8 Btn_Channel_Process(uint8 u8Ch, T_BTN_RESULT* ptBtnRes);

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
uint8 Btn_SM_Easy_Init(PF_GET_TM pfGetTm, PF_GET_BTN pfGetBtnSt);



#ifdef __cplusplus
}
#endif

#endif /* _BTN_SM_MODULE_ */




