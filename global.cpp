#include "menu.h"
// 全局变量定义（实际分配内存）
bool key1_flag = false;
bool key2_flag = false;
bool key3_flag = false;
bool key0_flag = false;
bool switch0_flag = false;
bool switch1_flag = false;
bool key_busy = false;

int16 encoder_left = 0;
int16 encoder_right = 0;
float battery_vol = 0;

int8 big_state;
bool ips_image_flag;
bool tcp_image_flag;