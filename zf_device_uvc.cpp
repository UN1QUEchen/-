#include "zf_device_uvc.h"


#include <opencv2/imgproc/imgproc.hpp>  // for cv::cvtColor
#include <opencv2/highgui/highgui.hpp> // for cv::VideoCapture
#include <opencv2/opencv.hpp>

#include <iostream> // for std::cerr
#include <fstream>  // for std::ofstream
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>
#include <atomic>

using namespace cv;

cv::Mat frame_rgb_main;      // 构建opencv对象 彩色
cv::Mat frame_rgay_main;     // 构建opencv对象 灰度
cv::Mat flip_rgb_main;       // 构建opencv对象 翻转
uint8_t *rgay_image_main;    // 灰度图像数组指针

cv::Mat frame_rgb_low;      // 构建opencv对象 彩色
cv::Mat frame_rgay_low;     // 构建opencv对象 灰度
cv::Mat flip_rgb_low;       // 构建opencv对象 翻转
uint8_t *rgay_image_low;    // 灰度图像数组指针
uint16_t *rgb_image_low;
uint8_t *rgay_image_conbine; // 灰度图像数组指针

VideoCapture cap_main;
VideoCapture cap_low;

int8 uvc_camera_init_main(const char *path, int16 expo)
{
    cap_main.open(path);

    if(!cap_main.isOpened())
    {
        printf("find main camera error.\r\n");
        return -1;
    } 
    else 
    {
        printf("find uvc camera Successfully.\r\n");
    }
    cap_main.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); // 设置视频编码格式


    cap_main.set(CAP_PROP_FRAME_WIDTH, UVC_WIDTH);     // 设置摄像头宽度
    cap_main.set(CAP_PROP_FRAME_HEIGHT, UVC_HEIGHT);    // 设置摄像头高度
    cap_main.set(CAP_PROP_FPS, 120);              // 显示屏幕帧率

    // 关闭自动曝光（根据摄像头不同，值可能有差异）
    cap_main.set(CAP_PROP_AUTO_EXPOSURE, 0.25);  // 通常0.25表示手动模式
    // 设置曝光值（具体值需要实验确定，典型范围0-10000）
    cap_main.set(CAP_PROP_EXPOSURE, expo);  // 初始值设为50，根据效果调整



    return 0;
}


int8 wait_image_refresh_main()
{
    try 
    {
        // 阻塞式等待图像刷新
        cap_main >> frame_rgb_main;
        // cap.read(frame_rgb);
        if (frame_rgb_main.empty()) 
        {
            std::cerr << "未获取到有效图像帧" << std::endl;
            return -1;
        }
        cv::flip(frame_rgb_main,flip_rgb_main,-1);
    } 
    catch (const cv::Exception& e) 
    {
        std::cerr << "OpenCV 异常: " << e.what() << std::endl;
        return -1;
    }

    // rgb转灰度
    cv::cvtColor(flip_rgb_main, frame_rgay_main, cv::COLOR_BGR2GRAY);


    // cv对象转指针
    rgay_image_main = reinterpret_cast<uint8 *>(frame_rgay_main.ptr(0));
    //double actualExposure = cap_main.get(CAP_PROP_EXPOSURE);
    //printf("Main Camera expo = : %.2f\n", actualExposure);

    return 0;
}

int8 uvc_camera_init_low(const char *path, int16 expo)
{
    cap_low.open(path);

    if(!cap_low.isOpened())
    {
        printf("find low camera error.\r\n");
        return -1;
    } 
    else 
    {
        printf("find uvc camera Successfully.\r\n");
    }
    cap_low.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); // 设置视频编码格式
    cap_low.set(CAP_PROP_FRAME_WIDTH, UVC_WIDTH);     // 设置摄像头宽度
    cap_low.set(CAP_PROP_FRAME_HEIGHT, UVC_HEIGHT);    // 设置摄像头高度
    cap_low.set(CAP_PROP_FPS, 60);              // 显示屏幕帧率
    // 关闭自动曝光（根据摄像头不同，值可能有差异）
    cap_low.set(CAP_PROP_AUTO_EXPOSURE, 0.75);  // 通常0.25表示手动模式
    // 设置曝光值（具体值需要实验确定，典型范围0-10000）
    cap_low.set(CAP_PROP_EXPOSURE, expo);  // 初始值设为50，根据效果调整
    double actualExposure = cap_low.get(CAP_PROP_EXPOSURE);
    printf("Low Camera expo = : %.2f\n", actualExposure);
    rgb_image_low = new uint16_t[UVC_WIDTH * UVC_HEIGHT]; // C++方式

    return 0;

}


int8 wait_image_refresh_low()
{
    try 
    {
        // 阻塞式等待图像刷新
        cap_low >> frame_rgb_low;
        if (frame_rgb_low.empty()) 
        {
            std::cerr << "未获取到有效图像帧" << std::endl;
            return -1;
        }
        cv::flip(frame_rgb_low, flip_rgb_low, -1);
    } 
    catch (const cv::Exception& e) 
    {
        std::cerr << "OpenCV 异常: " << e.what() << std::endl;
        return -1;
    }

    //BGR888转RGB565
    bgr888_to_rgb565(rgb_image_low);

    // rgb转灰度
    cv::cvtColor(flip_rgb_low, frame_rgay_low, cv::COLOR_BGR2GRAY);

    // cv对象转指针
    rgay_image_low = reinterpret_cast<uint8 *>(frame_rgay_low.ptr(0));

    return 0;
}


int8 image_conbine()
{
    // 组合图像
    // 创建目标图像容器
    if(frame_rgay_main.empty() || frame_rgay_low.empty())
    {
        std::cerr << "图像为空，无法进行拼接" << std::endl;
        return -1;
    }
    else
    {
        Mat combined(frame_rgay_main.rows + frame_rgay_low.rows, frame_rgay_main.cols, frame_rgay_main.type());

        // 图像拼接
        frame_rgay_main.copyTo(combined(Rect(0, 0, frame_rgay_main.cols, frame_rgay_main.rows)));
        frame_rgay_low.copyTo(combined(Rect(0, frame_rgay_main.rows, frame_rgay_low.cols, frame_rgay_low.rows)));

        rgay_image_conbine = reinterpret_cast<uint8 *>(combined.ptr(0));

    return 0;
    }
}

void bgr888_to_rgb565(uint16_t* rgb565) {
    if (flip_rgb_low.type() != CV_8UC3) return;

    for (int y = 0; y < flip_rgb_low.rows; ++y) {
        const uint8_t* row = flip_rgb_low.ptr<uint8_t>(y);
        for (int x = 0; x < flip_rgb_low.cols; ++x) {
            // 提取 BGR 通道（注意 OpenCV 默认顺序为 BGR）
            uint8_t b = row[3*x];
            uint8_t g = row[3*x + 1];
            uint8_t r = row[3*x + 2];

            // 转换为 RGB565
            rgb565[y * flip_rgb_low.cols + x] = 
                ((r >> 3) << 11) |  // 红色 5位
                ((g >> 2) << 5)  |  // 绿色 6位
                (b >> 3);           // 蓝色 5位
        }
    }
}