/*
 * Menu.h
 *
 *  Created on: 2025年2月6日
 *      Author: HIL
 */

 #ifndef CODE_MENU_H_
 #define CODE_MENU_H_


 #include "zf_common_headfile.h"
 #include "parameter.h"

 #define KEY_0       "/dev/zf_driver_gpio_key_0"
#define KEY_1       "/dev/zf_driver_gpio_key_1"
#define KEY_2       "/dev/zf_driver_gpio_key_2"
#define KEY_3       "/dev/zf_driver_gpio_key_3"
#define SWITCH_0    "/dev/zf_driver_gpio_switch_0"
#define SWITCH_1    "/dev/zf_driver_gpio_switch_1"
 //函数声明
 void  Menu(void);//菜单函数
 void Menu_run(void);
 void Menu_parameter(void);
 void get_keyandswitch(void);

//按键标志位
extern bool key1_flag;
extern bool key2_flag;
extern bool key3_flag;
extern bool key0_flag;
extern bool switch0_flag;
extern bool switch1_flag;
extern bool key_busy;
 
 //结构体声明
 typedef struct
 {
     int current;
     int up;//向上翻索引号
     int down;//向下翻索引号
     int enter;//确认索引号
     void (*current_operation)();//当前页面的索引号要执行的显示函数，这是一个函数指针
 }key_table;
 
 
 
 extern key_table run_table[100];
 extern key_table parameter_table[100];
 
 extern void fun_run_a1();
 extern void fun_run_b1();
 extern void fun_run_c1();
 extern void fun_run_d1();
 extern void fun_run_e1();
 extern void fun_run_f1();
 
 ///////////////////////////////////////////////
 
 extern void fun_run_a21();
 extern void fun_run_a22();
 extern void fun_run_a23();
 extern void fun_run_a24();
 extern void fun_run_a25();
 extern void fun_run_a26();
 
 extern void fun_run_b21();
 extern void fun_run_b22();
 extern void fun_run_b23();
 extern void fun_run_b24();
 extern void fun_run_b25();
 extern void fun_run_b26();
 
 extern void fun_run_c21();
 extern void fun_run_c22();
 extern void fun_run_c23();
 extern void fun_run_c24();
 extern void fun_run_c25();
 extern void fun_run_c26();
 
 extern void fun_run_d21();
 extern void fun_run_d22();
 extern void fun_run_d23();
 extern void fun_run_d24();
 extern void fun_run_d25();
 extern void fun_run_d26();
 
 extern void fun_run_e21();
 extern void fun_run_e22();
 extern void fun_run_e23();
 extern void fun_run_e24();
 extern void fun_run_e25();
 extern void fun_run_e26();
 
 //////////////////////////////////////////////
 
 extern void fun_run_a31();
 extern void fun_run_a32();
 extern void fun_run_a33();
 extern void fun_run_a34();
 extern void fun_run_a35();
 
 extern void fun_run_b31();
 extern void fun_run_b32();
 extern void fun_run_b33();
 extern void fun_run_b34();
 extern void fun_run_b35();
 
 extern void fun_run_c31();
 extern void fun_run_c32();
 extern void fun_run_c33();
 extern void fun_run_c34();
 extern void fun_run_c35();
 
 extern void fun_run_d31();
 extern void fun_run_d32();
 extern void fun_run_d33();
 extern void fun_run_d34();
 extern void fun_run_d35();
 
 extern void fun_run_e31();
 extern void fun_run_e32();
 extern void fun_run_e33();
 extern void fun_run_e34();
 extern void fun_run_e35();
 
 
 
 extern void fun_run_0();
 
 extern void fun_parameter_a1();
 extern void fun_parameter_b1();
 extern void fun_parameter_c1();
 extern void fun_parameter_d1();
 extern void fun_parameter_e1();
 extern void fun_parameter_f1();
 
 ///////////////////////////////////////////////
 
 extern void fun_parameter_a21();
 extern void fun_parameter_a22();
 extern void fun_parameter_a23();
 extern void fun_parameter_a24();
 extern void fun_parameter_a25();
 extern void fun_parameter_a26();
 
 extern void fun_parameter_b21();
 extern void fun_parameter_b22();
 extern void fun_parameter_b23();
 extern void fun_parameter_b24();
 extern void fun_parameter_b25();
 extern void fun_parameter_b26();
 
 extern void fun_parameter_c21();
 extern void fun_parameter_c22();
 extern void fun_parameter_c23();
 extern void fun_parameter_c24();
 extern void fun_parameter_c25();
 extern void fun_parameter_c26();
 
 extern void fun_parameter_d21();
 extern void fun_parameter_d22();
 extern void fun_parameter_d23();
 extern void fun_parameter_d24();
 extern void fun_parameter_d25();
 extern void fun_parameter_d26();
 
 extern void fun_parameter_e21();
 extern void fun_parameter_e22();
 extern void fun_parameter_e23();
 extern void fun_parameter_e24();
 extern void fun_parameter_e25();
 extern void fun_parameter_e26();
 
 //////////////////////////////////////////////
 
 extern void fun_parameter_a31();
 extern void fun_parameter_a32();
 extern void fun_parameter_a33();
 extern void fun_parameter_a34();
 extern void fun_parameter_a35();
 
 extern void fun_parameter_b31();
 extern void fun_parameter_b32();
 extern void fun_parameter_b33();
 extern void fun_parameter_b34();
 extern void fun_parameter_b35();
 
 extern void fun_parameter_c31();
 extern void fun_parameter_c32();
 extern void fun_parameter_c33();
 extern void fun_parameter_c34();
 extern void fun_parameter_c35();
 
 extern void fun_parameter_d31();
 extern void fun_parameter_d32();
 extern void fun_parameter_d33();
 extern void fun_parameter_d34();
 extern void fun_parameter_d35();
 
 extern void fun_parameter_e31();
 extern void fun_parameter_e32();
 extern void fun_parameter_e33();
 extern void fun_parameter_e34();
 extern void fun_parameter_e35();
 
 
 
 extern void fun_parameter_0();
 
 
 #endif /* CODE_MENU_H_ */