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
    delete[] rgb_image_low; // 对应new[]
    exit(0);
}
uint16 times = 0;
uint16 counts = 0;
float ofs_last = 0;
//编码器中断
void pit_callback()
{
    if(big_state == 1)
    {
        float speed_target_l_ = speed_target_l + speed_dif_p * offset + speed_dif_d * (offset - ofs_last);
        float speed_target_r_ = speed_target_r - speed_dif_p * offset - speed_dif_d * (offset - ofs_last);

        int16 speed_out_l;
        int16 speed_out_r;  
        encoder_left  = encoder_get_count(ENCODER_1);
        encoder_right = -encoder_get_count(ENCODER_2);
        // 编码器值即为速度值
        speed_real_l = encoder_left;
        // PID更新 
        speed_pwm_l = PidIncCtrl(&speed_pid_l, speed_target_l_ - speed_real_l, 0.01);
        
        // 只考虑一个方向转动
        if(speed_pwm_l >= 0)
            speed_out_l = (speed_pwm_l + speed_b_l) / speed_k_l;
        else
            speed_out_l = (speed_pwm_l - speed_b_l) / speed_k_l;
        if(speed_out_l >= 0)
        {
            pwm_set_duty(MOTOR1_PWM, speed_out_l * 100);
            gpio_set_level(MOTOR1_DIR, 1);
        }
        else
        {
            pwm_set_duty(MOTOR1_PWM, -speed_out_l * 100);
            gpio_set_level(MOTOR1_DIR, 0);
        }
        // 编码器值即为速度值
        speed_real_r = encoder_right;
        // PID更新 
        speed_pwm_r = PidIncCtrl(&speed_pid_r, speed_target_r_ - speed_real_r, 0.01);
         // 只考虑一个方向转动
        if(speed_pwm_r >= 0)
            speed_out_r = (speed_pwm_r + speed_b_r) / speed_k_r;
        else
            speed_out_r = (speed_pwm_r - speed_b_r) / speed_k_r;
        
        if(speed_out_r >= 0)
        {
            pwm_set_duty(MOTOR2_PWM, speed_out_r * 100);
            gpio_set_level(MOTOR2_DIR, 1);
        }
        else
        {
            pwm_set_duty(MOTOR2_PWM, -speed_out_r * 100);
            gpio_set_level(MOTOR2_DIR, 0);
        }

        adc_reg = adc_convert(ADC_REG_PATH);
        adc_scale = adc_get_scale(ADC_SCALE_PATH);
        battery_vol = adc_reg * adc_scale * 11;

        printf("offset = %f.\r\n", offset);
        printf("speed_target_l_ = %f.\r\n", speed_target_l_);
        printf("speed_target_r_ = %f.\r\n", speed_target_r_);
        printf("speed_pwm_l = %f.\r\n", speed_pwm_l);
        printf("speed_pwm_r = %f.\r\n", speed_pwm_r);
        printf("ecd_l = %d.\r\n", encoder_left);
        printf("ecd_r = %d.\r\n", encoder_right);
        printf("battery_vol = %f.\r\n", battery_vol);
        //printf("expo=%d.\r\n", expo);

        times++;
        if(times >= 100)
        {
            times = 0;
            //printf("counts = %d.\r\n", counts);
            counts = 0;
            //printf("expo=%d.\r\n", expo);
        }

    }
    if(big_state == 0)
    {
        // 关闭电机
        pwm_set_duty(MOTOR1_PWM, 0);
        pwm_set_duty(MOTOR2_PWM, 0);
    }
}

void pit_camera_low()
{
    if(big_state == 1 && double_uvc_flag == 1)
    {
        if(wait_image_refresh_low() < 0)
        {
            // 摄像头未采集到图像，这里需要关闭电机，关闭电调等。
            shutoff();
            exit(0);
        }
        if(ips_image_flag && ips_image_choose == 2)
        {
            //ips200_show_gray_image(0, 0, (const uint8 *)image_01, UVC_WIDTH, UVC_HEIGHT);
            ips200_show_color_image(0, 0, rgb_image_low, UVC_WIDTH, UVC_HEIGHT);
            

        }
        if(tcp_image_flag && tcp_image_choose == 2)
        {
            memcpy(image_copy_low[0], rgb_image_low, UVC_WIDTH * UVC_HEIGHT * 2);
            seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_RGB565, image_copy_low[0], UVC_WIDTH, UVC_HEIGHT);
            seekfree_assistant_camera_send();
            printf("testtesttesttest\r\n");
        }

    }
}


void servo_control()
{
    servo_out = int(servo_motor_duty_middle - (servo_KP * offset + servo_KD * (offset - ofs_last)) * SERVO_DIR);
    servo_out = servo_out < SERVO_MOTOR_R_MAX ? servo_out : SERVO_MOTOR_R_MAX;
    servo_out = servo_out > SERVO_MOTOR_L_MAX ? servo_out : SERVO_MOTOR_L_MAX;
    ofs_last = offset;
    pwm_set_duty(SERVO_MOTOR1_PWM, (uint16)SERVO_MOTOR_DUTY(servo_out));
}

//初始化
void init()
{
    // 注册清理函数
    atexit(cleanup);

    // 注册SIGINT信号的处理函数
    signal(SIGINT, sigint_handler);
    read_params();


    // 初始化屏幕
    ips200_init("/dev/fb0");
    pwm_get_dev_info(MOTOR1_PWM, &motor_1_pwm_info);
    pwm_get_dev_info(MOTOR2_PWM, &motor_2_pwm_info);

    // 初始化UVC摄像头
    if(uvc_camera_init_main("/dev/video0", expo) < 0)
    {
        shutoff();
        exit(0);
    }
    if(uvc_camera_init_low("/dev/video2", expo) < 0)
    {
        shutoff();
        exit(0);
    }
    big_state = 0;
    ips_image_flag = 1;
    tcp_image_flag = 0;
    double_uvc_flag = 0;

    // 获取PWM设备信息
    pwm_get_dev_info(SERVO_MOTOR1_PWM, &servo_pwm_info);

    // 创建pid对象
    My_Pid_Init();

    tcp_camera_init();

    //定时器控制角度环和速度环
    pit_ms_init(10, pit_callback);
    pit_ms_init(30, pit_camera_low);

    key_busy = false;

 
}


//主程序
int main(int argc, char* argv[]) 
{


    init();

    while(1)
    {
        get_keyandswitch();
        if(switch0_flag)
        {
            sigint_handler(1);
        }
        if(big_state == 0)
        {
            Menu();
        }
        else if(big_state == 1)
        {
            
            if(wait_image_refresh_main() < 0)
            {
                // 摄像头未采集到图像，这里需要关闭电机，关闭电调等。
                shutoff();
                exit(0);
            }
            if(img_process_flag)
            {
                Camera_All_Deal();
            }
            else
            {
                offset = offset_constant;
            }
            if(servo_control_flag)
            {
                servo_control();
            }
            //parameter_show();
            if(ips_image_flag && ips_image_choose == 1)
            {
                ips200_show_gray_image(0, 0, rgay_image_main, UVC_WIDTH, UVC_HEIGHT);
            }
            if(tcp_image_flag && tcp_image_choose == 1)
            {
                memcpy(image_copy_main[0], rgay_image_main, UVC_WIDTH * UVC_HEIGHT);
                seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_GRAY, image_copy_main[0], UVC_WIDTH, UVC_HEIGHT);
                seekfree_assistant_camera_send();
            }
            if(key0_flag)
            {
                big_state = 0;
            }

        }
        counts++;
        
        
    }
}