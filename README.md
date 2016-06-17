# Btn_State_Machine_Module
## Introduce:
一直在做代码模块化的工作，分享一个自己写的按键扫描模块，欢迎大家试用
本模块计划长期更新，本人能力有限，有错误之处欢迎指正交流，我将根据大家的反馈进行修改优化，提供更可靠的软件模块给大家。
**更新说明**
**2016年6月15日**： 本模块目前更新至V1.10，相比于采用switch状态机的V1.00版本，V1.10的状态机采用了状态转移表来实现，在增加状态数量和功能的前提下，不经提高了执行效率，同时减少了10%的FLASH消耗。

## 说明：
本模块可灵活配置如下参数：
* 按键数量；
* 按键常开常闭类型（未按下时稳态值）；
* 按键检测使能/非能控制；
* 按键去抖时间；
* 按键长按识别时间；
* 每个按键独立的按键状态获取函数。
   
本模块可以为上层提供：
* 按键事件（瞬态）：
  * BTN_PRESSED_EVT        按键刚被短按；
  * BTN_LONG_PRESSED_EVT   按键刚被长按；
  * BTN_S_RELEASED_EVT     按键刚从短按状态下释放；
  * BTN_L_RELEASED_EVT     按键刚从长按状态下释放。
  
* 按键状态（稳态）：
  * BTN_IDLE_ST            按键处于空闲状态（未按下）；
  * BTN_PRESS_AFT_ST       按键处于短按状态；
  * BTN_HOLDING_ST         按键处于长按状态。

## 资源消耗参考：
###M0+内核 + IAR 下 Low优化等级：  

 内容 | 区域 | 大小 | 说明                                                  
 ------------ | ------------ | ------------ | -------------
 代码 | Flash | 468字节 | 程序416字节 + 52字节的状态转移表                                                    
 配置参数/状态/接口变量 | RAM静态储存区 | n*20 + 8字节 | n为按键数量  
 临时变量/函数调用开销 | RAM栈区域 | 40字节 | 无   
         
## 使用方法：
1. 将Btn_SM_Module.c文件导入IDE工程，增加Btn_SM_Module.h和Btn_SM_Config.h的包含路径。
2. 修改Btn_SM_Config.h中MAX_BTN_CH的值为按键数量, 如需要为每个按键定义独立的“按键状态获取函数”，可在Btn_SM_Config.h定义 __BTN_SM_SPECIFIED_BTN_ST_FN
3. 编写系统时间获取函数，要求反馈系统ms时钟，类型为 uint16 (*)();
4. 编写按键状态获取函数，要求根据输入参数通道号反馈对应按键状态（逻辑1/0），类型为 uint8 (*)(uint8 u8Ch)。
5. 参考Btn_SM_Demo.c的代码，若需快速配置，可调用Btn_SM_Easy_Init()函数初始化模块；或参考Btn_SM_Easy_Init()函数，创建配置参数结构体实体并根据需求进行初始化配置（按键编号、去抖时间，长按时间，按键常态、是否使能按键、按键状态获取函数）和进行初始化工作。
8. 轮询Btn_Channel_Process()进行各个按键通道的状态，通过该函数输出参数确定按键返回事件及状态。
9. 通过Btn_Func_En_Dis()可在初始化之后屏蔽或启用按键功能。
