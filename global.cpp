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

float servo_KP = 1.1;
float servo_KD = 3.1;
float servo_out = 0;

int16 expo = 200;
int8 ipaddress = 1; //1为146.39，2为169.177

int8 ips_image_choose = 1; //1为主摄，2为低摄
int8 tcp_image_choose = 1; //1为主摄，2为低摄
float speed_dif_p = 0;
float speed_dif_d = 0;

bool servo_control_flag = 0;
bool img_process_flag = 0;

int offset_constant = 0;

bool double_uvc_flag; //双目摄像头标志位