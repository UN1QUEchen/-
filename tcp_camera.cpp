#include "tcp_camera.h"


uint8 image_copy_main[UVC_HEIGHT][UVC_WIDTH];
uint16 image_copy_low[UVC_HEIGHT][UVC_WIDTH];

void tcp_camera_init(void)
{
    // 初始化TCP客户端,需要先打开TCP服务器,这才不会卡主。
    // 初始化TCP客户端,需要先打开TCP服务器,这才不会卡主。
    // 初始化TCP客户端,需要先打开TCP服务器,这才不会卡主。
    if(ipaddress == 1)
    {
        char SERVER_IP[] = SERVER_IP_1;
        if(tcp_client_init(SERVER_IP, PORT) == 0)
        {
            printf("tcp_client ok\r\n");
        }
        else
        {
            printf("tcp_client error\r\n");
        
        }

        // 逐飞助手初始化 设置回调函数
         seekfree_assistant_interface_init(tcp_client_send_data, tcp_client_read_data);
    }
    else
    {
        char SERVER_IP[] = SERVER_IP_2;
        if(tcp_client_init(SERVER_IP, PORT) == 0)
        {
            printf("tcp_client ok\r\n");
        }
        else
        {
            printf("tcp_client error\r\n");
        
        }

        // 逐飞助手初始化 设置回调函数
         seekfree_assistant_interface_init(tcp_client_send_data, tcp_client_read_data);
    }

#if(0 == INCLUDE_BOUNDARY_TYPE)
    // 发送摄像头图像图像信息(仅包含原始图像信息)
#elif(1 == INCLUDE_BOUNDARY_TYPE)
    //以后可作为发送二值化后图像的部分
#elif(2 == INCLUDE_BOUNDARY_TYPE)
    //......
#endif



}

