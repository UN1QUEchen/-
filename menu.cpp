/*
 * Menu.c
 *
 *  Created on: 2025年2月6日
 *      Author: HIL
 */
 #include "menu.h"

 /*菜单代码思路:首先定义一个结构体包含(索引，上翻，下翻，确定，显示内容(该显示内容是一个函数指针void (*current_operation)();)
   然后用该结构体类型定义一个数组，该数组为结构体数组，每个元素都是结构体，可以访问结构体成员，(直观来看是将数组的值赋给成员)，实现上翻下翻和确认
   而操作函数的实现来源于结构体成员(函数指针)，该指针最后会赋值给一个指针变量，经过一系列变换后，最终实现函数操作。
   第一级菜单中选择某个功能，进入对应的二级菜单，实现具体功能。
 */
 
 //按键获取
void get_keyandswitch()
{
    //按下为1，若主板修改后需去掉！
    key0_flag = !gpio_get_level(KEY_0);
    key1_flag = !gpio_get_level(KEY_1);
    key2_flag = !gpio_get_level(KEY_2);
    key3_flag = !gpio_get_level(KEY_3);
    switch0_flag = gpio_get_level(SWITCH_0);
    switch1_flag = gpio_get_level(SWITCH_1);
    //一次只能使用一个按键，一个按键只会被读取一次
    if(key0_flag || key1_flag || key2_flag || key3_flag)
    {
        if(key_busy)
        {
            //第n次执行
            key0_flag = 0;
            key1_flag = 0;
            key2_flag = 0;
            key3_flag = 0;

        }
        else
        {
            //第一次执行
            key_busy = true;
        }

    }
    else
    {
        key_busy = false;
    }
}
 
 int  func_run_index = 0; //初始显示欢迎界面
 int  last_run_index = 127; //last初始为无效值
 int func_parameter_index = 0;
 int last_parameter_index = 127;
 //struct PID PID1, PID2, PID3;
 
 void (*current_operation_index)(void);       //显示函数索引指针(当前操作索引)
 
 key_table run_table[100]=                 //结构体数组
 {
 //{索引，向上，向下，确认，显示函数}
     //第0层
     {0,1,1,1,(*fun_run_0)},                     //AIIT_meun
 
     //第1层
     {1,5,2, 6,(*fun_run_a1)},//just run
     {2,1,3,7,(*fun_run_b1)},//ips set
     {3,2,4,11,(*fun_run_c1)},//tcp set
     {4,3,5,16,(*fun_run_d1)},//run set
     {5,4,1,0,(*fun_run_e1)},//esc
 
     //第2层
     {6,20, 20, 20, (*fun_run_a21)},//just run

     {7,10,8,21, (*fun_run_b21)},//ips set//ips on
     {8,7,9,8, (*fun_run_b22)},//ips para
     {9,8,10,6, (*fun_run_b23)},//run
     {10,9,7,2, (*fun_run_b24)},//esc
 
     {11,15,12,25, (*fun_run_c21)},//tcp set
     {12,11,13,12, (*fun_run_c22)},//tcp ip
     {13,12,14,13, (*fun_run_c23)},//tcp para
     {14,13,15,6, (*fun_run_c24)},//run
     {15,14,11,3, (*fun_run_c25)},//esc
 
     {16,19,17,30, (*fun_run_d21)},//run set
     {17,16,18,17, (*fun_run_d22)},//null
     {18,17,19,6, (*fun_run_d23)},//run
     {19,18,16,4, (*fun_run_d24)},//null
 
     //第3层
     {20,20,20,20, (*fun_run_a31)},//out
 
     {21,24,22,7,(*fun_run_b31)},//ips off
     {22,21,23,22,(*fun_run_b32)},//ips para
     {23,22,24,6,(*fun_run_b33)},//run
     {24,23,21,2,(*fun_run_b34)},//esc
 
     {25,29,26,11,(*fun_run_c31)},//tcp off
     {26,25,27,26,(*fun_run_c32)},
     {27,26,28,27,(*fun_run_c33)},
     {28,27,29,6,(*fun_run_c34)},
     {29,28,25,3,(*fun_run_c35)},
 
     {30,33,31,30,(*fun_run_d31)},
     {31,30,32,31,(*fun_run_d32)},
     {32,31,33,32,(*fun_run_d33)},
     {33,32,30,33,(*fun_run_d34)},
 };


 key_table parameter_table[100]=                 //结构体数组
 {
 //{索引，向上，向下，确认，显示函数}
     //第0层
     {0,1,1,1,(*fun_parameter_0)},                     //AIIT_meun
 
     //第1层
     {1,1,1, 0,(*fun_parameter_a1)},
 
 };
 
 
 void Menu(void)//菜单函数
 {
    if(!switch1_flag)
    {
        Menu_run();
        func_parameter_index = 0;
        last_parameter_index = 127;
    }
    else
    {
        Menu_parameter();
        func_run_index = 0;
        last_run_index = 127;
    }
}

void Menu_run(void)//菜单函数
 {
    if(key3_flag)
    {
        func_run_index = run_table[func_run_index].up;    //向上翻
    }
 
    if(key2_flag)
    {
 
        func_run_index = run_table[func_run_index].down;    //向下翻
    }
 
    if(key1_flag)
    {
        func_run_index = run_table[func_run_index].enter;    //确认
    }
 
    if (func_run_index != last_run_index)
    {
        current_operation_index = run_table[func_run_index].current_operation;
        ips200_clear();
        (*current_operation_index)();//执行当前操作函数
        last_run_index = func_run_index;
    }
}

void Menu_parameter(void)//菜单函数
 {
    if(key3_flag)
    {
        func_parameter_index = parameter_table[func_parameter_index].up;    //向上翻
    }
 
    if(key2_flag)
    {
 
        func_parameter_index = parameter_table[func_parameter_index].down;    //向下翻
    }
 
    if(key1_flag)
    {
        func_parameter_index = parameter_table[func_parameter_index].enter;    //确认
    }
 
    if (func_parameter_index != last_parameter_index)
    {
        current_operation_index = parameter_table[func_parameter_index].current_operation;
        ips200_clear();
        (*current_operation_index)();//执行当前操作函数
        last_parameter_index = func_parameter_index;
    }
}
 
 ///*********第0层***********/
 void fun_run_0()    //欢迎界面
 {
     //show_rgb565_image(0,16*5, (const uint16 *)gImage_SU, 240, 80, 240, 80, 0);
     ips200_show_string(8*6, 300, "Southeast University");
 }
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////第一层///////////////////////////////////////////////////////////////////////////////////////////////////////////
 void fun_run_a1()   //舵机
 {
     ips200_show_string(0,  16*1, "->");
     ips200_show_string(20, 16*1, "just run");    //默认开跑，开启显示屏，不开图传，默认参数显示
     ips200_show_string(20, 16*2, "ips200 set");   //显示屏设置，开启/关闭显示图像，设置显示参数
     ips200_show_string(20, 16*3, "tcp set");      //图传设置，启动图传与参数传递，设置udp与tcp的相关参数
     ips200_show_string(20, 16*4, "run state set");     //运行设置
     ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
     ips200_show_string(8*23, 16*19, "Page_1");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_b1()   //电机
 {
    ips200_show_string(0,  16*2, "->");
    ips200_show_string(20, 16*1, "just run");    //默认开跑，开启显示屏，不开图传，默认参数显示
    ips200_show_string(20, 16*2, "ips200 set");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp set");      //图传设置，启动图传与参数传递，设置udp与tcp的相关参数
    ips200_show_string(20, 16*4, "run state set");     //获取当前角度
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_1");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_c1()   //编码器
 {
    ips200_show_string(0,  16*3, "->");
    ips200_show_string(20, 16*1, "just run");    //默认开跑，开启显示屏，不开图传，默认参数显示
    ips200_show_string(20, 16*2, "ips200 set");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp set");      //图传设置，启动图传与参数传递，设置udp与tcp的相关参数
    ips200_show_string(20, 16*4, "run state set");     //获取当前角度
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_1");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_d1()   //按键调参
 {
    ips200_show_string(0,  16*4, "->");
    ips200_show_string(20, 16*1, "just run");    //默认开跑，开启显示屏，不开图传，默认参数显示
    ips200_show_string(20, 16*2, "ips200 set");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp set");      //图传设置，启动图传与参数传递，设置udp与tcp的相关参数
    ips200_show_string(20, 16*4, "run state set");     //获取当前角度
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_1");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_e1()   //GPS
 {
    ips200_show_string(0,  16*5, "->");
    ips200_show_string(20, 16*1, "just run");    //默认开跑，开启显示屏，不开图传，默认参数显示
    ips200_show_string(20, 16*2, "ips200 set");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp set");      //图传设置，启动图传与参数传递，设置udp与tcp的相关参数
    ips200_show_string(20, 16*4, "run state set");     //获取当前角度
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_1");             //显示页码，page1就是第一级，下同
 }
 
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////第二层///////////////////////////////////////////////////////////////////////////////////////////////////////////
 void fun_run_a21()      //默认开跑
 {
   ips200_show_string(8*23, 16*19, "Page_2");
   ips200_show_string(20, 16*1, "Press any Key to continue");
   ips200_show_string(20, 16*2, "ips_image show:");
   ips200_show_string(20, 16*3, "tcp_image show:");
   if(ips_image_flag)
      ips200_show_string(140, 16*2, "on");
   else
      ips200_show_string(140, 16*2, "off");
   if(tcp_image_flag)
      ips200_show_string(140, 16*3, "on");
   else
      ips200_show_string(140, 16*3, "off");

 }
 
 
 void fun_run_b21()  //ips set
 {
    ips200_show_string(0,  16*1, "->");
    ips200_show_string(20, 16*1, "ips_camera show: on");    //开启屏幕显示
    ips200_show_string(20, 16*2, "ips_parameter choose:/");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
    ips_image_flag = 1;
 }
 
 void fun_run_b22()  //ips set
 {
    ips200_show_string(0,  16*2, "->");
    ips200_show_string(20, 16*1, "ips_camera show: on");    //开启屏幕显示
    ips200_show_string(20, 16*2, "ips_parameter choose:/");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_b23()
 {
    ips200_show_string(0,  16*3, "->");
    ips200_show_string(20, 16*1, "ips_camera show: on");    //开启屏幕显示
    ips200_show_string(20, 16*2, "ips_parameter choose:/");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_b24()
 {
    ips200_show_string(0,  16*4, "->");
    ips200_show_string(20, 16*1, "ips_camera show: on");    //开启屏幕显示
    ips200_show_string(20, 16*2, "ips_parameter choose:/");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void fun_run_c21()  //tcp set
 {
    ips200_show_string(0,  16*1, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: on");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
    tcp_image_flag = 1;
 }
 
 void fun_run_c22()  //
 {
    ips200_show_string(0,  16*2, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: on");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_c23()
 {
    ips200_show_string(0,  16*3, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: on");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }

 void fun_run_c24()
 {
    ips200_show_string(0,  16*4, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: on");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }

 void fun_run_c25()
 {
    ips200_show_string(0,  16*5, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: on");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void fun_run_d21()      //run set
 {
    ips200_show_string(0,  16*1, "->");
    ips200_show_string(20, 16*1, "Hello world");    //开启tcp显示
    ips200_show_string(20, 16*2, "Hello world");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_d22()      //按键调速度环PID2
 {
    ips200_show_string(0,  16*2, "->");
    ips200_show_string(20, 16*1, "Hello world");    //开启tcp显示
    ips200_show_string(20, 16*2, "Hello world");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 void fun_run_d23()      //按键调方向速度环PID3
 {
    ips200_show_string(0,  16*3, "->");
    ips200_show_string(20, 16*1, "Hello world");    //开启tcp显示
    ips200_show_string(20, 16*2, "Hello world");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }

 void fun_run_d24()      //按键调方向速度环PID3
 {
    ips200_show_string(0,  16*4, "->");
    ips200_show_string(20, 16*1, "Hello world");    //开启tcp显示
    ips200_show_string(20, 16*2, "Hello world");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////第三层///////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 
 void fun_run_a31()
 {
    ips200_clear();
    big_state = 1;
 }
 
 
 void fun_run_b31()
 {
    ips200_show_string(0,  16*1, "->");
    ips200_show_string(20, 16*1, "ips_camera show: off");    //开启屏幕显示
    ips200_show_string(20, 16*2, "ips_parameter choose:/");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
    ips_image_flag = 0; 
 }
 
 void fun_run_b32()
 {
    ips200_show_string(0,  16*2, "->");
    ips200_show_string(20, 16*1, "ips_camera show: off");    //开启屏幕显示
    ips200_show_string(20, 16*2, "ips_parameter choose:/");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 
 
 }
 
 void fun_run_b33()
 {
    ips200_show_string(0,  16*3, "->");
    ips200_show_string(20, 16*1, "ips_camera show: off");    //开启屏幕显示
    ips200_show_string(20, 16*2, "ips_parameter choose:/");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }

 void fun_run_b34()
 {
    ips200_show_string(0,  16*4, "->");
    ips200_show_string(20, 16*1, "ips_camera show: off");    //开启屏幕显示
    ips200_show_string(20, 16*2, "ips_parameter choose:/");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "run");
    ips200_show_string(20, 16*4, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }

 void fun_run_c31()
 {
    ips200_show_string(0,  16*1, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: off");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
    tcp_image_flag = 0;
 
 }
 
 void fun_run_c32()
 {
    ips200_show_string(0,  16*2, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: off");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 
 }
 
 void fun_run_c33()
 {
    ips200_show_string(0,  16*3, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: off");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 
 }
 
 void fun_run_c34()
 {
    ips200_show_string(0,  16*4, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: off");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 }
 
 
 void fun_run_c35()
 {
    ips200_show_string(0,  16*5, "->");
    ips200_show_string(20, 16*1, "tcp_camera show: off");    //开启tcp显示
    ips200_show_string(20, 16*2, "tcp_ip set:");   //显示屏设置，开启/关闭显示图像，设置显示参数
    ips200_show_string(20, 16*3, "tcp_parameter show:");
    ips200_show_string(20, 16*4, "run");
    ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
    ips200_show_string(8*23, 16*19, "Page_2");             //显示页码，page1就是第一级，下同
 
 }
 
 void fun_run_d31()
 {
 
 
 }
 
 void fun_run_d32()
 {
 
 
 }
 
 void fun_run_d33()
 {
 
 
 }
 
 void fun_run_d34()
 {
 
 
 }
 
 



 void fun_parameter_0()    //欢迎界面
 {
     //show_rgb565_image(0,16*5, (const uint16 *)gImage_SU, 240, 80, 240, 80, 0);
     ips200_show_string(8*6, 300, "CANSHU__Southeast University");
 }
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////第一层///////////////////////////////////////////////////////////////////////////////////////////////////////////
 void fun_parameter_a1()   //舵机
 {
     ips200_show_string(0,  16*1, "->");
     ips200_show_string(20, 16*1, "Set left max value");    //设置左向最大打角
     ips200_show_string(20, 16*2, "Set right max value");   //设置右向最大打角
     ips200_show_string(20, 16*3, "Set target angle");      //设置目标角度
     ips200_show_string(20, 16*4, "Get current angle");     //获取当前角度
     ips200_show_string(20, 16*5, "ESC");                   //返回上一级，下同
     ips200_show_string(8*23, 16*19, "Page_1");             //显示页码，page1就是第一级，下同
 }