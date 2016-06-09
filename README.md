# Btn_State_Machine_Module
## Introduce:
It is a button state machine module for general use.

It is a similar button state machine module which I have used in a real product. I have improve it and add something new while keeping it reliable.  
一直在做代码模块化的工作，分享一个自己写的按键扫描模块，欢迎大家试用
本帖计划长期更新，本人能力有限，有错误之处欢迎指正交流，我将根据大家的反馈进行修改优化，提供更可靠的软件模块给大家。

## 说明：
本模块可根据按键的按下及释放的状态向上层反馈“按键按下事件”，“按键短按释放事件”，“按键长按事件”和“按键长按释放事件”以满足应用中对按键不同类型操作的检测。
自带软件去抖，长短按识别。
按键常态（常开常闭）可配、去抖时间及长按识别时间可调、按键功能可屏蔽/开启，上述功能每个按键可以单独配置不同功能；按键数量可配。
采用状态机实现，不拖累系统效率。
本软件模块不限定平台，任何平台都可以用（目标如此）。

## 资源消耗参考：
M0+内核 + IAR 下 Low优化等级：  
代码消耗524字节Flash。  
配置参数/运行状态/接口变量消耗n*22+字节的静态全局区RAM （n为按键数量，22为平均值，因地址对齐原因会有所变化）。  
临时变量/函数调用消耗RAM栈区域的资源待测。  
         
## 使用方法：
1. 将Btn_SM_Module.c文件导入IDE工程，增加Btn_SM_Module.h的包含路径。
2. 修改Btn_SM_Module.h中MAX_BTN_CH的值为按键数量（或在该模块上层重新定义该宏，推荐!）
3. 编写系统时间获取函数，函数要求见Btn_SM_Module.h
4. 编写按键状态（逻辑1/0）获取函数，函数要求见Btn_SM_Module.h
5. 复制Btn_SM_Module.h中的Demo代码，创建配置参数的结构体并根据要求进行初始化配置（去抖时间，长按时间，按键常态、是否使能按键）
6. 调用一次Btn_General_Init()进行通用初始化
7. 调用n次"Btn_Channel_Init()"进行各个按键通道的初始化（n为按键数量）
8. 轮询Btn_Channel_Process()进行各个按键通道的状态，通过该函数返回值确定按键返回状态：短按、长按、短按释放、长按释放及无状态。
9. 通过Btn_Func_En_Dis()可在初始化之后屏蔽或启用按键功能。
