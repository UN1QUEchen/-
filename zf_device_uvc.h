#ifndef _zf_driver_uvc_h
#define _zf_driver_uvc_h


#include "zf_common_typedef.h"

#define UVC_WIDTH   160
#define UVC_HEIGHT  120
#define UVC_FPS     120

int8 uvc_camera_init_main(const char *path, int16 expo);
int8 wait_image_refresh_main();
int8 uvc_camera_init_low(const char *path, int16 expo);
int8 wait_image_refresh_low();
int8 image_conbine();
void bgr888_to_rgb565(uint16_t* rgb565);

extern uint8 *rgay_image_main;
extern uint8 *rgay_image_low;
extern uint8 *rgay_image_conbine;
extern uint16 *rgb_image_low;
#endif
