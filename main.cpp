#include "zf_common_headfile.h"
#include "main.h"

//退出处理
void sigint_handler(int signum) 
{
    printf("收到Ctrl+C，程序即将退出\n");
    shutoff();
    exit(0);
}
void cleanup()
{
    printf("程序异常退出，执行清理操作\n");
    // 处理程序退出！！！
    // 这里需要关闭电机，关闭电调等。
    shutoff();
}
void shutoff()
{
    pwm_set_duty(MOTOR1_PWM, 0);
    pwm_set_duty(MOTOR2_PWM, 0);
    ips200_clear();
    big_state = 0;
    pwm_set_duty(SERVO_MOTOR1_PWM, (uint16)SERVO_MOTOR_DUTY(90));
}

//编码器中断
void pit_callback()
{
    if(big_state == 1)
    {
        uint16 speed_out;  
        encoder_left  = encoder_get_count(ENCODER_1);
        encoder_right = -encoder_get_count(ENCODER_2);
        // 编码器值即为速度值
        speed_real = encoder_left;
        // PID更新 
        speed_pwm = 4.5 * PidLocCtrl(&speed_pid_l, speed_target_l - speed_real, 1.f);
        //
        //解释：4.5为编码器读数映射到占空比的大概系数。此处speed_target和speed_real均以编码器读数为参考基准。
        //
        // 只考虑一个方向转动
        speed_out = speed_pwm;
        pwm_set_duty(MOTOR1_PWM, speed_out);
        gpio_set_level(MOTOR1_DIR, 1);
        // 编码器值即为速度值
        speed_real = encoder_right;
        // PID更新 
        speed_pwm = 4.5 * PidLocCtrl(&speed_pid_r, speed_target_r - speed_real, 1.f);
         // 只考虑一个方向转动

        pwm_set_duty(MOTOR2_PWM, speed_out);
        gpio_set_level(MOTOR2_DIR, 1);
    }
}



//初始化
void init()
{
    // 注册清理函数
    atexit(cleanup);

    // 注册SIGINT信号的处理函数
    signal(SIGINT, sigint_handler);

    // 初始化屏幕
    ips200_init("/dev/fb0");
    pwm_get_dev_info(MOTOR1_PWM, &motor_1_pwm_info);
    pwm_get_dev_info(MOTOR2_PWM, &motor_2_pwm_info);

    // 初始化UVC摄像头
    if(uvc_camera_init("/dev/video0") < 0)
    {
        shutoff();
        exit(0);
    }
    big_state = 0;
    ips_image_flag = 1;
    tcp_image_flag = 0;

    // 创建pid对象
    My_Pid_Init();

    tcp_camera_init();

    //定时器控制角度环和速度环
    pit_ms_init(10, pit_callback);

    key_busy = false;

}


//主程序
int main(int, char**) 
{
    init();

    while(1)
    {
        get_keyandswitch();
        if(!switch0_flag)
        {
            sigint_handler(1);
        }
        if(big_state == 0)
        {
            Menu();
        }
        else if(big_state == 1)
        {
            
            if(wait_image_refresh() < 0)
            {
                // 摄像头未采集到图像，这里需要关闭电机，关闭电调等。
                shutoff();
                exit(0);
            }
            Camera_All_Deal();
            if(ips_image_flag)
            {
                ips200_show_gray_image(0, 0, (const uint8 *)image_01, UVC_WIDTH, UVC_HEIGHT);
            }
            //parameter_show();
            if(tcp_image_flag)
            {
                memcpy(image_copy[0], image_01[0], UVC_WIDTH * UVC_HEIGHT);
                seekfree_assistant_camera_send();
            }
            if(!key0_flag)
            {
                big_state = 0;
            }

        }
    }
}