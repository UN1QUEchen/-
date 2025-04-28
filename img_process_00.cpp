#include "img_process_00.h"
#include "math.h"
//#include "fuse.h"


/******此处设置为偏差几行到几行，间距最好是20，方便调PID********/
#define CONTROLL_H_MAX  90
#define CONTROLL_H_MIN  70

uint8 controll_h_max = 80;  //固定间隔为20行，设置一个即可，另外一个自动偏移（按键调参中有）
uint8 controll_h_min;

uint32 u32ChuKu_Time=0;  //发车计时
uint32 u32Barrier_Time=0;  //横断计时

uint16 time_prevent_check_startline;

 /***********赛道宽度，下面数据是标定出来的********/
uint8 kuandu_saidao [UVC_HEIGHT-1] =
{
        155,154,154,152,152,151,150,149,149,147,
        146,144,144,143,142,141,141,139,138,137,
        137,136,135,135,134,133,132,132,130,130,
        129,129,127,127,126,125,125,124,123,122,
        121,121,119,119,118,117,117,116,114,114,
        114,112,112,110,110,109,108,107,107,105,
        105,104,103,102,102,100,100,99,98,98,
        96,95,94,93,93,91,91,89,88,87,
        86,84,84,82,82,81,80,79,77,77,
        76,75,74,74,72,72,71,70,70,
}; //赛道宽度

/**********元素处理结构体**********/
struct YUAN_SU road_type, stage, distance, num;

uint8 image_yuanshi[UVC_HEIGHT][UVC_WIDTH];      //原始图像
uint8 image_01[UVC_HEIGHT][UVC_WIDTH];           //二值化图像
uint8  op;

void Show_Judge_Cirque(void)
{
    ips200_show_uint(0, 120, Qianzhan_Middle, 3);            ips200_show_string(50, 120, "Qianzhan_Middle");
    ips200_show_uint(0, 140, left_guai, 3);                  ips200_show_string(50, 140, "left_guai");
    ips200_show_uint(0, 160, right_guai, 3);                 ips200_show_string(50, 160, "right_guai");
    ips200_show_uint(0, 180, r_haveline, 3);                 ips200_show_string(50, 180, "r_haveline");
    ips200_show_uint(0, 200, l_haveline, 3);                 ips200_show_string(50, 200, "l_haveline");
    ips200_show_uint(0, 220, road_type.LeftCirque, 3);       ips200_show_string(50, 220, "road_type.LeftCirque");
}

void Show_Left_Cirque()
{
    ips200_show_uint(0, 120, l_haveline, 3);            ips200_show_string(50, 120, "l_haveline");
    ips200_show_uint(0, 140, r_haveline, 3);            ips200_show_string(50, 140, "r_haveline");

    ips200_show_uint(0, 160, length_l_line, 3);         ips200_show_string(50, 160, "length_l_line");
    ips200_show_uint(0, 180, length_r_line, 3);         ips200_show_string(50, 180, "length_r_line");

    ips200_show_uint(0, 200, road_type.LeftCirque, 3);  ips200_show_string(50, 200, "T_LeftCirque");

    ips200_show_uint(0, 220, Qianzhan_Middle, 3);       ips200_show_string(50, 220, "Qianzhan_Middle");
    ips200_show_uint(0, 240, stage.LeftCirque, 3);      ips200_show_string(50, 240, "stage.LC");
    ips200_show_uint(0, 260, road_width[50], 3);        ips200_show_string(50, 260, "road_width[50]");

    ips200_show_uint(0, 280, Left_CirqueIntegration, 4);    ips200_show_string(50, 280, "Left_CirqueIntegration");

    Draw_Cross(l_line_x[left_guai], left_guai, RGB565_BLUE, 15);
    Draw_Cross(l_line_x[right_guai], right_guai, RGB565_BLUE, 15);
}

void Show_Right_Cirque()
{
    ips200_show_uint(0, 120, l_haveline, 3);            ips200_show_string(50, 120, "l_haveline");
    ips200_show_uint(0, 140, r_haveline, 3);            ips200_show_string(50, 140, "r_haveline");

    ips200_show_uint(0, 160, length_l_line, 3);         ips200_show_string(50, 160, "length_l_line");
    ips200_show_uint(0, 180, length_r_line, 3);         ips200_show_string(50, 180, "length_r_line");

    ips200_show_uint(0, 200, road_type.RightCirque, 3); ips200_show_string(50, 200, "T_RightCirque");

    ips200_show_uint(0, 220, Qianzhan_Middle, 3);       ips200_show_string(50, 220, "Qianzhan_Middle");
    ips200_show_uint(0, 240, stage.RightCirque, 3);     ips200_show_string(50, 240, "stage.RC");
    ips200_show_uint(0, 260, road_width[60], 3);            ips200_show_string(50, 260, "road_width[60]");
    ips200_show_uint(0, 280, Right_CirqueIntegration, 4);   ips200_show_string(50, 280, "Right_CirqueIntegration");

    Draw_Cross(l_line_x[left_guai], left_guai, RGB565_BLUE, 15);
    Draw_Cross(l_line_x[right_guai], right_guai, RGB565_BLUE, 15);
}


////摄像头处理全流程
void Camera_All_Deal(void)
{
    Transfer_Camera(rgay_image, image_yuanshi[0], UVC_WIDTH*UVC_HEIGHT);  //图像转存

    //        Horizontal_line();
    Get01change_Dajin();                          //图像二值化
    Search_Line();                                //搜线
    //ips200_show_gray_image(0, 0, (const uint8 *)image_01, UVC_WIDTH, UVC_HEIGHT);
    //Show_Judge_Cirque();
    //Show_Left_Cirque();
    //Show_Right_Cirque();
    Element_Test();                               //元素识别
    Element_Handle();                             //元素处理
    Handle_Roadblock();
    Calculate_Offset();                           //计算偏差
    Blacking();

    //ips200_displayimage03x(image_01[0], UVC_WIDTH, UVC_HEIGHT);//显示二值化黑白图像
    //ips200_displayimage03x(image_yuanshi[0], UVC_WIDTH, UVC_HEIGHT);//显示灰度图像，此部分我主要用于调试断路区域的
    //Blacking();
}


void Camera_Parameter_Init(st_Camera *sptr)
{
    sptr->Ramp = 0;
    sptr->straight = 0;
    sptr->bend  = 0;
}

//-----------------------------------------------------------------------------------------
//  @brief      图像转存
//  @param      *p              待转存数组地址
//  @param      *q              转存后数组地址
//  @param      pixel_num       数组长度
//  @return     void
//  Sample usage:    Transfer_Camera(mt9v03x_image[0], image_yuanshi[0], UVC_WIDTH*UVC_HEIGHT);
//-----------------------------------------------------------------------------------------
void Transfer_Camera(uint8 *p, uint8 *q, int16 pixel_num)
{
    for(int16 i = 0; i < pixel_num; i++)
        *(q +i) = *(p +i);
}

//-----------------------------------------------------------------------------------------
//  @brief      求阈值并二值化
//  @param      void
//  @return     void
//  Sample usage:    Get01change_Dajin();
//  explain       该函数里面调用了Threshold_Deal(image_yuanshi[0], UVC_WIDTH, UVC_HEIGHT, Threshold_detach);
//-----------------------------------------------------------------------------------------
uint8 Threshold;                //阈值
uint8 Threshold_static = 150;   //阈值静态下限225
uint16 Threshold_detach = 255;  //阳光算法分割阈值
void Get01change_Dajin()
{
      Threshold = Threshold_Deal(image_yuanshi[0], UVC_WIDTH, UVC_HEIGHT, Threshold_detach);

      if (Threshold < Threshold_static)
      {
          Threshold = Threshold_static;
      }

      uint8 thre;
      for(uint8 y = 0; y < UVC_HEIGHT; y++)  //这里考虑到了图像边缘的光照偏弱的问题
      {
        for(uint8 x = 0; x < UVC_WIDTH; x++)
        {
          if (x <= 15)                      //受影响的边缘范围
            thre = Threshold - 10;          //修改可调节图像边缘的清晰度
          else if (x >= UVC_WIDTH-15)
            thre = Threshold - 10;
          else
            thre = Threshold;

          if (image_yuanshi[y][x] > thre)         //数值越大，显示的内容越多，较浅的图像也能显示出来
            image_01[y][x] = 255;  //白
          else
            image_01[y][x] = 0;   //黑
        }
      }
}

//-----------------------------------------------------------------------------------------
//  @brief      求阈值
//  @param      *image              待求阈值的图像数组
//  @param      x                   图像宽度
//  @param      y                   图像高度
//  @param      pixel_threshold     阳光算法分割阈值
//  @return     void
//  Sample usage:    Threshold_Deal(image_yuanshi[0], UVC_WIDTH, UVC_HEIGHT, Threshold_detach);
//  explain：       该函数在Get01change_Dajin();里面被调用
//  Threshold = Threshold_Deal(image_yuanshi[0], UVC_WIDTH, UVC_HEIGHT, Threshold_detach);
//-----------------------------------------------------------------------------------------

uint8 Threshold_Deal(uint8* image, uint16 x, uint16 y, uint32 pixel_threshold)
{
      #define GrayScale 256
      uint16 width = x;
      uint16 height = y;
      int pixelCount[GrayScale];
      float pixelPro[GrayScale];
      int i, j;
      int pixelSum = width * height;
      uint8 threshold = 0;
      uint8* data = image;  //指向像素数据的指针
      for (i = 0; i < GrayScale; i++)
      {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
      }
      uint32 gray_sum = 0;
      //统计灰度级中每个像素在整幅图像中的个数
      for (i = 0; i < height; i += 1)
      {
        for (j = 0; j < width; j += 1)
        {
          pixelCount[(int)data[i * width + j]]++;  //将当前的点的像素值作为计数数组的下标
          gray_sum += (int)data[i * width + j];  //灰度值总和
          //}
        }
      }
      //计算每个像素值的点在整幅图像中的比例
      for (i = 0; i < GrayScale; i++)
      {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
      }

      //遍历灰度级[0,255]
      float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
      w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
      for (uint32 j = 0; j < pixel_threshold; j++)
      {
        w0 += pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和 即背景部分的比例
        u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值

        w1 = 1 - w0;
        u1tmp = gray_sum / pixelSum - u0tmp;

        u0 = u0tmp / w0;    //背景平均灰度
        u1 = u1tmp / w1;    //前景平均灰度
        u = u0tmp + u1tmp;  //全局平均灰度
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
        if (deltaTmp > deltaMax)
        {
          deltaMax = deltaTmp;
          threshold = (uint8)j;
        }
        if (deltaTmp < deltaMax)
        {
          break;
        }
      }
  return threshold;
}

//像素滤波
void Pixle_Filter()
{
  for(uint8 y = 10; y < UVC_HEIGHT-10; y++)
  {
    for(uint8 x = 10; x < UVC_WIDTH -10; x++)
    {
      if((image_01[y][x] == 0) && (image_01[y-1][x] + image_01[y+1][x] + image_01[y][x-1] + image_01[y][x+1] >= 3*255))
      { //一个黑点的上下左右的白点大于等于三个，令这个点为白
          image_01[y][x] = 255;
      }
      else if((image_01[y][x] != 0) && (image_01[y-1][x] + image_01[y+1][x] + image_01[y][x-1] + image_01[y][x+1] < 2*255))
       {//一个白点的上下左右的黑点大于等于三个，令这个点为黑
          image_01[y][x] = 0;
       }
    }
  }
}

uint8 search_line_end = 0;
int16 l_line_x[UVC_HEIGHT], r_line_x[UVC_HEIGHT], m_line_x[UVC_HEIGHT];        //储存原始图像的左右边界的列数
int16 l_line_x_l[UVC_HEIGHT], r_line_x_l[UVC_HEIGHT], m_line_x_l[UVC_HEIGHT];  //储存原始图像的左右边界的列数
uint8 l_lose_value = 0, r_lose_value = 0;                                   //左右丢线数
uint8 l_search_flag[UVC_HEIGHT], r_search_flag[UVC_HEIGHT];                   //是否搜到线的标志
uint8 l_width, r_width;                                                     //循环变量名
uint8 l_search_start, r_search_start;                                       //搜线起始x坐标
uint8 r_losemax, l_losemax;
uint8 l_line_x_yuanshi[UVC_HEIGHT], r_line_x_yuanshi[UVC_HEIGHT];
uint8 road_width[UVC_HEIGHT];


 /************************************************************
【函数名称】Horizontal_line
【功    能】横向巡线函数(未启用)
【实    例】Horizontal_line();
***********************************************************/
/*
uint8 cross_liewhite[UVC_WIDTH];
uint8 cross_hangwhite[UVC_HEIGHT];
uint8 mid = 0;
uint8 heidianzuobiao[13];
uint8 cankaobaidian = 250;
void Horizontal_line(void)
{
  uint8 i,j,m=0;
  uint8 hang=0;
  uint8 lie=0;
  uint8 a=0;

  uint8 max;
  uint8 max_heidian;
  uint16 value;

  for( lie=0; lie<UVC_WIDTH; lie++)        //从左边向右遍历每一列
  {
      cross_liewhite[lie]=0;         //初始化直方图
  }
  //存白点 列
  for( lie=10; lie<UVC_WIDTH; lie=lie+10)        //从左边向右遍历每一列
  {
      for( hang=UVC_HEIGHT-1;hang>5;hang=hang-1)  //hang>5    //从底部向上遍历每一行
      {
          value = abs((100*(cankaobaidian-mt9v03x_image[hang][lie])/(cankaobaidian+mt9v03x_image[hang][lie])));
          if(value < 15)   //判断为白点
              cross_liewhite[lie]++;
          if(value > 20 )   //判断为黑点
          {
              heidianzuobiao[m] = hang;
              m++;
              break;
          }
      }
  }
  max = 0;
  for( lie=0; lie<UVC_WIDTH; lie=lie+10)        //从左边向右遍历每一列
  {
      if(max < cross_liewhite[lie])
      {
          max = cross_liewhite[lie];      //找出最长白列
          mid = lie;                      //获得最长白列的横坐标
      }
  }
  max_heidian = 0;
  for( m=0; m<17; m++)                 //从左边向右遍历每一黑点的纵坐标
    {
        if(max_heidian < heidianzuobiao[m])
        {
            max_heidian = heidianzuobiao[m];
            //获得最近黑点的纵坐标
        }
    }
  ips200_draw_line(0, max_heidian, 180, max_heidian, RGB565_BLACK);

  a = mt9v03x_image[max_heidian][mid];
  for(i=UVC_HEIGHT-1;i>0;i--)  //i>0     // 从下面开始向上遍历每一行
  {
      for(j=mid;j>5;j=j-1 )    // 从最长白列位置向左遍历每一列
      {
          if(abs(100*(a - mt9v03x_image[i][j])/(a + mt9v03x_image[i][j])) > 20 || j==5)      // 如果当前像素为黑色或者已经到达左边界
          {
              l_line_x[i]=j;              // 将当前行的左线位置设置为当前列
              break;                      // 跳出内层循环
          }
      }

      for(j=mid;j<UVC_WIDTH-5;j=j+1)      // 从中线位置向右遍历每一列
      {
          if(abs(100*(a - mt9v03x_image[i][j])/(a + mt9v03x_image[i][j])) > 20 || j==175)    // 如果当前像素为黑色或者已经到达右边界
          {
              r_line_x[i]=j;     // 将当前行的右线位置设置为当前列
              break;      // 跳出内层循环
          }
      }
//      ips200_draw_point(l_line_x[i], i, RGB565_BLUE);
//      ips200_draw_point(r_line_x[i], i, RGB565_GREEN);
//      midlin[i]=(leftline[i]+rightlin[i])/2;     // 计算当前行的中线位置为左线位置和右线位置的平均值
//      ips200_draw_point(midlin[i], i, RGB565_RED);
      road_width[i] = r_line_x[i] - l_line_x[i];
  }
}
*/

//-----------------------------------------------------------------------------------------
//  @brief      搜左右边线
//  @param      void
//  @return     void
//  Sample usage:    Search_Line();
//-----------------------------------------------------------------------------------------
 void Search_Line()
 {
     uint8 l_flag = 0, r_flag = 0;
     r_lose_value = l_lose_value = 0;                              //搜线之前将丢线数清零

     for(uint8 y = UVC_HEIGHT - 1; y > search_line_end; y--)        //确定每行的搜线起始横坐标
     {
         if( (y > UVC_HEIGHT - 5) || ( (l_line_x[y+1] == 0) && (r_line_x[y+1] == UVC_WIDTH -1) && (y < UVC_HEIGHT - 4) ))   //前四行，或者左右都丢线的行
         {
             l_search_start = r_search_start = UVC_WIDTH/2;
         }
         else if((l_line_x[y+1] != 0) && (r_line_x[y+1] != UVC_WIDTH - 1) && (y < UVC_HEIGHT - 4))   //左右都不丢线
         {
             l_search_start = l_line_x[y+1] + 5;
             r_search_start = r_line_x[y+1] - 5;
         }
         else if((l_line_x[y+1] != 0 && r_line_x[y+1] == UVC_WIDTH - 1) && (y < UVC_HEIGHT - 4))   //左不丢线,右丢线
         {
             l_search_start = l_line_x[y+1] + 5;
             r_search_start = UVC_WIDTH/2;
         }
         else if((l_line_x[y+1] == 0 && r_line_x[y+1] != UVC_WIDTH - 1) && (y < UVC_HEIGHT - 4))   //右不丢线,左丢线
         {
             l_search_start = UVC_WIDTH/2;
             r_search_start = r_line_x[y+1] - 5;
         }
         for(l_width = l_search_start; l_width > 1; l_width--)      //左边搜线
         {
            if(image_01[y][l_width -2] == 0 && image_01[y][l_width - 1] == 0 && image_01[y][l_width] != 0 && l_width > 2)
            {//黑黑白
                l_line_x[y] = l_width - 1;
                l_line_x_yuanshi[y] = l_width - 1;
                l_search_flag[y] = 1;
                break;
            }
            else if(l_width == 2)
            {
                if(l_flag==0)
                {
                    l_flag = 1;
                    l_losemax = y +1;
                }

                l_line_x[y] = 0;
                l_line_x_yuanshi[y] = 0;
                l_search_flag[y] = 0;
                l_lose_value++;
              break;
            }
          }

          for(r_width = r_search_start; r_width < (UVC_WIDTH - 2); r_width++)      //右边搜线
             {
                 if(image_01[y][r_width] != 0 && image_01[y][r_width +1] == 0 && image_01[y][r_width +2] == 0 && r_width < UVC_WIDTH - 3)
                 {//白黑黑
                     r_line_x[y] = r_width + 1;
                     r_line_x_yuanshi[y] = r_width + 1;
                     r_search_flag[y] = 1;
                     break;
                 }
                 else if(r_width == UVC_WIDTH - 3)
                 {
                     if(r_flag==0)
                     {
                         r_flag = 1;
                         r_losemax = y + 1;
                     }
                 r_line_x[y] = UVC_WIDTH - 1;
                 r_line_x_yuanshi[y] = UVC_WIDTH - 1;
                 r_search_flag[y] = 0;
                 r_lose_value++;
                    break;
                }
             }

          road_width[y] = r_line_x[y] - l_line_x[y];
         }
 }


 /************************************************************
 【函数名称】Lost_line_right
 【功    能】右侧图像丢线检查函数
 【参    数】无
 【返 回 值】-1为未丢线 其他为丢线起始行
 【实    例】Lost_line_right();
 【注意事项】无
 ************************************************************/

 int8 Lost_line_right(void)
 {
   uint8 i;
   for(i=60;i>20;i--)
   if(r_line_x[i]==179)     return i;
   return -1;
 }

 /************************************************************

 【函数名称】Lost_line_left
 【功    能】左侧图像丢线检查函数
 【参    数】无
 【返 回 值】-1为未丢线 其他为丢线起始行
 【实    例】Lost_line_left();
 【注意事项】无
 ************************************************************/

 int8 Lost_line_left(void)
 {
   uint8 i;
   for(i=60;i>20;i--)
   if(l_line_x[i]==0) return i;
   return -1;
 }

 //检测边线
 uint8 length_l_line, length_r_line;
 void Check_bianxian(uint8 type)
 {
     uint8 length_l_line, length_r_line;
     if(type == 1)
     {
         length_l_line = 0;
         for(uint8 y = UVC_HEIGHT-1; y > 10; y--)
         {
             if((l_line_x[y] <= l_line_x[y-1]) && l_line_x[y] != 0)   //两边不丢线才计算直道长度
             {
                 length_l_line++;
             }
         }
     }
     else if(type == 2)
     {
         length_r_line = 0;
         for(uint8 y = UVC_HEIGHT-1; y > 10; y--)
         {
             if((r_line_x[y] >= r_line_x[y-1]) && r_line_x[y] != UVC_WIDTH -1)
             {
                 length_r_line++;
             }
         }
     }
 }

//-----------------------------------------------------------------------------------------
//  @brief      偏差计算
//  @param      void
//  @return     void
//  Sample usage:    Calculate_Offset();
//-----------------------------------------------------------------------------------------
uint16 bizhangtime,bizhangertime;
uint8 bizhangset;
int32 offset_1;
float offset;    //摄像头处理得到的偏差(车在偏左侧是负值，在偏右侧是正值)

void Calculate_Offset()
{
    offset_1 = 0;
    for(uint8 y = UVC_HEIGHT -1; y >= 10; y--)
    {
        m_line_x[y] = (l_line_x[y] + r_line_x[y])/2;
    }

    for(uint8 y = CONTROLL_H_MAX; y >=CONTROLL_H_MIN; y=y-2)//for(uint8 y = UVC_HEIGHT -35; y >=UVC_HEIGHT -45; y--)
    {
        offset_1 += (m_line_x[y] - UVC_WIDTH/2);
    }
    if(road_type.L_RoadBlock)
    {
            offset_1 += 200;
    }
    else if(road_type.R_RoadBlock)
    {
            offset_1 -= 200;
    }

       offset = (float)(offset_1/20);
}

//-----------------------------------------------------------------------------------------
//  @brief      画边线和中线(彩色)
//  @param      void
//  @return     void
//  Sample usage:    Blacking();
//  explain:  如果屏幕显示的图像进行了缩放就不能适用
//-----------------------------------------------------------------------------------------
void Blacking()
{
    for(uint8 y=(UVC_HEIGHT-1); y>10; y--)      //赛道边界及中线
    {
        ips200_draw_point(l_line_x[y], y, RGB565_GREEN);
        ips200_draw_point(r_line_x[y], y, RGB565_YELLOW);
        ips200_draw_point( m_line_x[y], y, RGB565_BLUE);

    }

    for(uint8 i =5; i<UVC_WIDTH-5; i++)
    {
        ips200_draw_point(i, CONTROLL_H_MAX, RGB565_BLUE);
        ips200_draw_point(i, CONTROLL_H_MIN, RGB565_RED);
        ips200_draw_point(i, 20, RGB565_RED);
    }
    for(uint8 i=10; i<UVC_HEIGHT-1; i++)                        //图像中线
    {
        ips200_draw_point(UVC_WIDTH/2, i, RGB565_RED);
        ips200_draw_point(5, i, RGB565_RED);
        ips200_draw_point(10, i, RGB565_RED);
        ips200_draw_point(15, i, RGB565_RED);
        ips200_draw_point(25, i, RGB565_RED);
        ips200_draw_point(30, i, RGB565_RED);
        ips200_draw_point(UVC_WIDTH-1-5, i, RGB565_RED);
        ips200_draw_point(UVC_WIDTH-1-10, i, RGB565_RED);
        ips200_draw_point(UVC_WIDTH-1-15, i, RGB565_RED);
        ips200_draw_point(UVC_WIDTH-1-25, i, RGB565_RED);
        ips200_draw_point(UVC_WIDTH-1-30, i, RGB565_RED);
    }

   // ips200_show_string(0,115,"Speed_pwm");
   // ips200_show_int(150,115,Speed_pwm,4);
   /// ips200_show_string(0,130,"Inloop_pwm");
   // ips200_show_int(150,130,Inloop_pwm,4);
/*
    tft180_show_string(0,145,"imu660ra_gyro_z");
    tft180_show_int(150,145,imu660ra_gyro_z,4);
    tft180_show_string(0,160,"offset");
    tft180_show_float(150,160, offset, 5,2);
*/                    //图像误差，注释了
/*
    ips200_show_string(0,175,"Allpwm_left");
    ips200_show_int(150, 175, Allpwm_left, 5);           //左推动风扇转速
    ips200_show_string(0,190,"Allpwm_right");
    ips200_show_int(150, 190, Allpwm_right, 5);          //右推动风扇转速

    ips200_show_string(0,205,"Turn_Allpwm_left");
    ips200_show_int(150, 205, Turn_Allpwm_left+5000, 5);        //左辅助风扇转速
    ips200_show_string(0,220,"Turn_Allpwm_right");
    ips200_show_int(150, 220, Turn_Allpwm_right+5000, 5);       //右辅助风扇转速

    ips200_show_uint(190, 10, Right_CirqueIntegration, 5);      //角速度积分
*/



}

//出界保护
//uint8 flag_stop = 0;
//void Outside_protect(void)
//{
//    uint8 j=0;
//    for(int16 x = l_line_x[UVC_HEIGHT-1]; x < r_line_x[UVC_HEIGHT-1]; x++)
//    {
//        if(image_01[UVC_HEIGHT-1][x] == 0)
//        {
//            j++;
//            if(j > 120)
//            {
//                flag_stop = 1;
//            }
//        }
//    }
//}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      元素识别处理
//  @param      void
//  @return     void
//  Sample usage:   Element_Test();
//-------------------------------------------------------------------------------------------------------------------
void Element_Test()
{
    //imu660ra_get_acc();
    Check_bianxian(1);
    Check_bianxian(2);

    QianZhan(1, UVC_HEIGHT-1, 1);
    Have_Count(1, controll_h_max, controll_h_max -40);
    Have_Count(2, controll_h_max, controll_h_max -40);
    Have_Count(1, 40, 0);
    Have_Count(2, 40, 0);
    Test_guai (1, controll_h_max, 30);
    Test_guai (2, controll_h_max, 30);

    Cross_Test();

    if(!road_type.Cross && !road_type.L_RoadBlock && !road_type.R_RoadBlock && !road_type.LeftCirque && !road_type.RightCirque )
    {
        if(num.LeftCirque == 0 || num.RightCirque == 0)
        {
            Ring_Test();                //判断是否为环岛
        }
        if(num.L_RoadBlock == 0 && num.R_RoadBlock == 0)
        {
            Roadblock_Test();
        }
    }
}


void  Element_Handle(void)              //元素处理
{
    if(road_type.Cross)
    {
        Handle_Cross();
    }
    else if(road_type.LeftCirque)
    {
        Handle_Left_Cirque();
    }
    else if(road_type.RightCirque)
    {
        Handle_Right_Cirque();
    }

    else if(road_type.L_RoadBlock || road_type.R_RoadBlock)
    {
        Handle_Roadblock();
    }
}

//void Element_Show()
//{
//Show_Right_Cirque();
//Show_Roadbolck();
//}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      摄像头判断是否出界
//  @param      void
//  @return     void
//  Sample usage:    Outside_protect(100);
//-------------------------------------------------------------------------------------------------------------------
uint8 flag_stop;
void Outside_protect(uint8 value)
{
    uint8 j = 0;
    for(int16 x = 0; x < UVC_WIDTH - 1; x++)
    {
        if(image_01[UVC_HEIGHT -2][x] == 0 )  //&& 0 == road_type.barrier &&  Element == nothing
        {
            j++;
            if(j > value )  //摄像头识别出界
            {
                flag_stop = 1;
                //SysFlag.Stop_Run = ON;
            }
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief     获取赛道宽度
//  @param      void
//  @return     void
//  Sample usage:    Get_Width_Road();
//  explain：  通过蓝牙传回来，使用逐飞默认串口 在cpu_main.c中的debug_init()声明了默认串口;
//-------------------------------------------------------------------------------------------------------------------
void Get_Width_Road(void)
{
    //打印赛道宽度
    for(uint8 y = UVC_HEIGHT-1; y > search_line_end; y--)
    {
        if(y == UVC_HEIGHT-1)
        {
            printf("%d",  1022);
            printf(",");
        }
        if(y%10 == 0)
        {
            printf("%d", r_line_x[y] - l_line_x[y]);
            printf(",");
            printf(", \n");
        }
        else
        {
            printf("%d", r_line_x[y] - l_line_x[y]);
            printf(",");
        }
    }
}


void Search_Ring(void)
{
    //1.找中线白点
    //2.找左右丢线点
}


uint8 Qianzhan_Middle, Qianzhan_Left, Qianzhan_Right;
void QianZhan(uint8 type,uint8 start_line,uint8 end_line)  //type:0,中线白点，1左边，2右边
{
   if(type == 1)
   {
      Qianzhan_Middle = 0;
      for(uint8 x = start_line; x > end_line; x--)
      {
          if(image_01[x][UVC_WIDTH/2] != 0)
          {
              Qianzhan_Middle++;
          }
      }
   }
   else if(type == 2)
   {
       Qianzhan_Left=0;
     for(uint8 x=start_line;x>end_line;x--)
        {
           if(image_01[x][UVC_WIDTH/2-30]!=0)
          {
            Qianzhan_Left++;
           }
        }
   }
   else if(type == 3)
   {
     Qianzhan_Right = 0;
     for(uint8 x=start_line;x>end_line;x--)
    {
       if(image_01[x][UVC_WIDTH/2+30]!=0)
      {
        Qianzhan_Right++;
       }
    }
   }
}


uint8 r_haveline, l_haveline;
void Have_Count(uint8 type, uint8 start_line, uint8 end_line)
{
    if(type == 1)
    {
       l_haveline = 0;
       for(uint8 i = start_line; i > end_line; i--)
       {
           if(l_search_flag[i] == 1)
           {
               l_haveline++;
           }
       }
    }
    else if(type == 2)
   {
      r_haveline = 0;
      for(uint8 i = start_line; i > end_line; i--)
      {
          if(r_search_flag[i] == 1)
          {
              r_haveline++;
          }
      }
   }
}


/********************用于检测圆环拐点****************/
uint8 left_guai, right_guai;
void Test_guai(uint8 type, uint8 start_line, uint8 end_line)
{
    if(type == 1)
    {
       left_guai = 0;
       for(uint8 i = start_line; i > end_line; i--)
       {
           if(l_search_flag[i+1] == 1 && l_search_flag[i] == 1 && l_search_flag[i -1] == 0 && l_search_flag[i -2] == 0)
           {
               left_guai = i;
               break;
           }
       }
    }
    else if(type == 2)
    {
        right_guai = 0;
        for(uint8 i = start_line; i > end_line; i--)
        {
            if(r_search_flag[i +1] == 1 && r_search_flag[i] == 1 && r_search_flag[i -1] == 0 && r_search_flag[i -2] == 0)
            {
                right_guai = i;
                break;
            }
        }
    }
}
/********************Test_guai_end****************/

/********************用于检测拐点周围点****************/
uint8 Point_Guai_right,Point_Guai_left;
void Count_Point(uint8 type,uint8 start_line)
{
    if(type==1&&start_line!=0)
    {
        Point_Guai_left=0;
        for(uint8 i=start_line-2;i>start_line-7;i--)
          {
              for(uint8 j=l_line_x[start_line+2];j<l_line_x[start_line+2]-10;j--)
              {
                  if(image_01[i][j]==1)
                  {
                      Point_Guai_left++;
                  }
              }
          }
    }
    else if(type==2&&start_line!=0)
    {
        Point_Guai_right=0;
   for(uint8 i=start_line-2;i>start_line-7;i--)
   {
       for(uint8 j=r_line_x[start_line+2];j<r_line_x[start_line-2]+10;j++)
       {
           if(image_01[i][j]==1)
           {
               Point_Guai_right++;
           }
       }
   }
  }
}
/********************Count_Point_end****************/

/********************用于检测环岛****************/
void Ring_Test(void)
{
    if(Qianzhan_Middle > 90 && left_guai != 0 && right_guai == 0 && r_haveline >= 38 && l_haveline < 10 && !road_type.RightCirque)
    {
        road_type.LeftCirque = 1;
        num.LeftCirque++;
    }
    if(Qianzhan_Middle > 90 && left_guai == 0 && right_guai != 0 && r_haveline < 10 && l_haveline >= 38 && !road_type.LeftCirque)
    {
        road_type.RightCirque = 1;
        num.RightCirque++;
    }
}
/********************Ring_Test_end****************/

/********************用于检测障碍物****************/
void Roadblock_Test(void)
{
    if(l_line_x[50] - l_line_x[60] > 15 && abs(l_line_x[45] - l_line_x[50]) < 5 && r_lose_value < 10&& l_lose_value < 10)  //检测左边是否有路障
    {
        //gpio_set_level(E9,0);
        road_type.L_RoadBlock = 1;
        num.L_RoadBlock++;
    }
    else if(r_line_x[60] - r_line_x[50] > 15  && abs(r_line_x[45] - r_line_x[50]) < 5 && r_lose_value < 10 && l_lose_value < 10)  //检测右边是否有路障
    {
        //gpio_set_level(E9,0);
        road_type.R_RoadBlock = 1;
        num.R_RoadBlock++;
    }
}

/********************用于检测十字路口****************/
void Cross_Test(void)
{
    if(Qianzhan_Middle > 90 && l_lose_value > 38 && r_lose_value > 38)
    {
        //Find_Up_Point(5,80);
        //if(Left_Up_Find!=0 && Right_Up_Find!=0)
            road_type.Cross = 1;
    }
}
/********************Cross_Test_end****************/

/********************用于处理左环岛****************/
uint8 L_integration_flag = 0, R_integration_flag = 0;
void Handle_Left_Cirque(void)
{
    if(road_type.LeftCirque)
    {
        switch(stage.LeftCirque)
        {
            case 0:
                if(road_width[50] > 140)  //中间行
                {
//                    if( r_haveline<=38)
//                    {
//                       road_type.LeftCirque = 0;
//                       num.LeftCirque=0;
//                       //break;
//                    }
                    //BUZZER_ON;
                    //gpio_set_level(E9,0);
                    stage.LeftCirque = 1;
                }
            break;
            case 1:
                if(road_width[50] < 130)
                {
                    //BUZZER_OFF;
                    //gpio_set_level(E9,1);
                    stage.LeftCirque = 2;
                }
            break;
            case 2:
                if(road_width[50] > 135)
                {
                    //BUZZER_ON;
                    //gpio_set_level(E9,0);
                    stage.LeftCirque = 3;
                }
            break;
            case 3:
                if(Left_CirqueIntegration > 90) //进环
                {
                    //BUZZER_OFF;
                    //gpio_set_level(E9,1);
                    stage.LeftCirque = 4;
                }
            break;
            case 4:
                if(Left_CirqueIntegration >600 ) //环内
                {
                    //BUZZER_ON;
                    //gpio_set_level(E9,0);
                    stage.LeftCirque = 5;
                }
            break;
            case 5:
                if(Left_CirqueIntegration > 1000) //出环
                {
                    //BUZZER_OFF;
                    //gpio_set_level(E9,1);
                    stage.LeftCirque=6;
                }
            break;
            case 6:
                if(Left_CirqueIntegration > 1200)  //直行
                {
                    //BUZZER_ON;
                    stage.LeftCirque = 7;
                    //gpio_set_level(E9,0);
                }
            break;
        }
        if(stage.LeftCirque == 0 || stage.LeftCirque == 1 || stage.LeftCirque == 2)  //直行
        {
            if( r_haveline<=38)
            {
                road_type.LeftCirque = 0;
                num.LeftCirque=0;
            }
            for(uint8 i = UVC_HEIGHT-1; i > 10; i--)
            {
                l_line_x[i] = r_line_x[i] - kuandu_saidao[UVC_HEIGHT-1 - i]+10;
            }

        }
        if(stage.LeftCirque == 3)  //进环
        {
            L_integration_flag = 1;
            for(uint8 i = UVC_HEIGHT-1; i > 10; i--)
            {
                r_line_x[i] = l_line_x[i] + kuandu_saidao[UVC_HEIGHT-1 -i]-20;
            }
        }
        if(stage.LeftCirque ==4 )  //环内
        {

        }
        if(stage.LeftCirque == 5)  //出环
        {

            for(uint8 i = UVC_HEIGHT-1; i > 10; i--)
            {
                r_line_x[i] = l_line_x[i] + kuandu_saidao[UVC_HEIGHT -1 -i] -20;
            }
        }
        if(stage.LeftCirque == 6)  //直行
        {
            //BUZZER_OFF;
//            Search_Line();
            for(uint8 i = UVC_HEIGHT-1; i > 10; i--)
            {
                l_line_x[i] = r_line_x[i] - kuandu_saidao[UVC_HEIGHT -1 -i]+15;
            }
        }
        if(stage.LeftCirque == 7)
        {
            road_type.LeftCirque = 0;
            stage.LeftCirque = 0;
            L_integration_flag = 0;
            Left_CirqueIntegration = 0;
            //num.LeftCirque = 0;
        }
    }
}
/********************Handle_Left_Cirque_end****************/

/********************用于处理右环岛****************/
void Handle_Right_Cirque(void)
{
    if(road_type.RightCirque)
    {
        switch(stage.RightCirque)
        {
            case 0:
                if(road_width[60] > 140)
                {
                    //gpio_set_level(E9,0);
                    stage.RightCirque = 1;
                }
            break;
            case 1:
                if(road_width[60] < 135)
                {
                    //gpio_set_level(E9,1);
                    stage.RightCirque = 2;
                }
            break;
            case 2:
                if(road_width[60] > 136)
                {
                    //gpio_set_level(E9,0);
                    stage.RightCirque = 3;
                }
            break;
            case 3:
                if(Right_CirqueIntegration > 90)
                {
                    //gpio_set_level(E9,1);
                    stage.RightCirque = 4;
                }
            break;
            case 4:
                if(Right_CirqueIntegration > 600)   //2300
                {
                    //gpio_set_level(E9,0);
                    stage.RightCirque = 5;
                }
            break;
            case 5:
                if(Right_CirqueIntegration > 1000)
                {
                    //gpio_set_level(E9,1);
                    stage.RightCirque = 6;
                }
            break;
            case 6:
                if(Right_CirqueIntegration > 1200)
                {
                    //gpio_set_level(E9,0);
                    stage.RightCirque = 7;
                }
            break;
        }
        if(stage.RightCirque == 0||stage.RightCirque == 1||stage.RightCirque == 2)  //直行
        {
            if(l_haveline<=38)
            {
                road_type.RightCirque = 0;
                num.RightCirque=0;
                //break;
            }
            for(uint8 i = UVC_HEIGHT -1; i > 10; i--)
            {
                r_line_x[i] = l_line_x[i] + kuandu_saidao[UVC_HEIGHT -1 -i]-10;
            }
        }
        if(stage.RightCirque == 3) //进环
        {
            //BUZZER_ON;
            R_integration_flag = 1;
            for(uint8 i = UVC_HEIGHT -1; i > 10; i--)
            {
                l_line_x[i] = r_line_x[i] - kuandu_saidao[UVC_HEIGHT -1 -i] +20;
            }
        }
        if(stage.RightCirque == 4) //环内
        {
            //BUZZER_OFF;
        }
        if(stage.RightCirque == 5)  //出环
        {
            //BUZZER_ON;
            for(uint8 i = UVC_HEIGHT -1; i > 10; i--)
            {
                l_line_x[i] = r_line_x[i] - kuandu_saidao[UVC_HEIGHT -1 -i] +25;
            }
        }
        if(stage.RightCirque == 6)  //直行
        {
            //BUZZER_OFF;
            for(uint8 i = UVC_HEIGHT -1; i > 10; i--)
            {
                r_line_x[i] = l_line_x[i] + kuandu_saidao[UVC_HEIGHT -1 -i]-15;
            }
        }
        if(stage.RightCirque == 7)
        {
            stage.RightCirque = 0;
            R_integration_flag = 0;
            road_type.RightCirque = 0;
            Right_CirqueIntegration = 0;
            //num.RightCirque = 0;
        }
    }
}

/********************Handle_Left_Cirque_end****************/

/********************用于处理障碍物****************/
uint16 Roadblock_lent;
void Handle_Roadblock(void)
{
    //BUZZER_ON;
    if (distance.L_RoadBlock > 1500)
    {
        road_type.L_RoadBlock = 0;
        distance.L_RoadBlock  = 0;
        //gpio_set_level(E9,1);
        //BUZZER_OFF;
    }
    else if(distance.R_RoadBlock > 1500)
    {
        road_type.R_RoadBlock = 0;
        distance.R_RoadBlock  =0;
        //gpio_set_level(E9,1);
        //BUZZER_OFF;
    }
}
/********************Handle_Roadblock_end****************/

/***************************用于处理十字***********************/

///*正入十字*/
///********************用于检查十字的四个角****************/
//uint8 Row_l_1, Col_l_1, Row_l_2, Col_l_2;
//uint8 Row_r_1, Col_r_1, Row_r_2, Col_r_2;
//void Check_Corss_Point(uint8 type)
//{
//    if (type == 1)
//    {
//        Row_l_1 = Row_l_2 = 0;
//        for (uint8 i = UVC_HEIGHT - 5; i > 20; i--)
//        {
//            if (Row_l_1 == 0 && l_line_x[i] != 0 && l_line_x[i - 5] == 0)  //找左下拐点
//            {
//                if (l_line_x[i] - l_line_x[i - 5] > 15)
//                {
//                    Row_l_1 = i;
//                    Col_l_1 = l_line_x[i];
//                }
//            }
//            if(Row_l_1 != 0)                                               //找左上拐点
//            {
//                for (uint8 i = Row_l_1; i > 20; i--)
//                {
//                    if (l_line_x[i] == 0 && l_line_x[i - 5] != 0)
//                    {
//                        if (l_line_x[i - 5] - l_line_x[i] > 15)
//                        {
//                            Row_l_2 = i;
//                            Col_l_2 = l_line_x[i];
//                            break;
//                        }
//                    }
//                }
//            }
//        }
//    }
//    else if (type == 2)
//    {
//        Row_r_1 = Row_r_2 = 0;
//        for (uint8 i = UVC_HEIGHT - 1; i > 20; i--)
//        {
//            if (Row_r_1 == 0 && r_line_x[i] != UVC_WIDTH -1 && r_line_x[i - 5] == UVC_WIDTH - 1)  //找右下拐点
//            {
//                if (r_line_x[i-5] - r_line_x[i] > 15)
//                {
//                    Row_r_1 = i;
//                    Col_r_1 = r_line_x[i];
//                }
//            }
//            if (Row_r_1 != 0)                                                                     //找右上拐点
//            {
//                for (uint8 i = Row_r_1; i > 20; i--)
//                {
//                    if (r_line_x[i] == 0 && r_line_x[i - 5] != 0)
//                    {
//                        if (r_line_x[i - 5] - r_line_x[i] > 15)
//                        {
//                            Row_r_2 = i;
//                            Col_r_2 = r_line_x[i];
//                            break;
//                        }
//                    }
//                }
//            }
//        }
//    }
//}


/*******************寻找十字上面的两个点*****************/
uint8 Left_Up_Find=0;
uint8 Right_Up_Find=0;
void Find_Up_Point(uint8 start,uint8 end)
{
    for(uint8 i=start;i>=end;i--)
    {

        if(abs(l_line_x[i]-l_line_x[i-2])<5&&
           abs(l_line_x[i-2]-l_line_x[i-4])<5&&
           abs(l_line_x[i-4]-l_line_x[i-6])<5&&
              (l_line_x[i]-l_line_x[i+2])>15&&
              (l_line_x[i]-l_line_x[i+4])>25&&
              (l_line_x[i]-l_line_x[i+6])>  25)
        {
            Left_Up_Find=i;//获取行数即可
        }

        if(abs(r_line_x[i]-r_line_x[i-1])<=5&&
           abs(r_line_x[i-1]-r_line_x[i-2])<=5&&
           abs(r_line_x[i-2]-r_line_x[i-3])<=5&&
              (r_line_x[i]-r_line_x[i+2])<=-8&&
              (r_line_x[i]-r_line_x[i+3])<=-15&&
              (r_line_x[i]-r_line_x[i+4])<=-15)
        {
            Right_Up_Find=i;//获取行数即可
        }
//        if(Left_Up_Find!=0&&Right_Up_Find!=0)//下面两个找到就出去
//        {
//            return;
//        }
    }
}

//    if(abs(Right_Up_Find-Left_Up_Find)>=80)//纵向撕裂过大，视为误判
//    {
//        Right_Up_Find=0;
//        Left_Up_Find=0;
//    }



/*-------------------------------------------------------------------------------------------------------------------
  @brief     找下面的两个拐点，供十字使用
  @param     搜索的范围起点，终点
  @return    修改两个全局变量
             Right_Down_Find=0;
             Left_Down_Find=0;
  Sample     Find_Down_Point(int start,int end)
  @note      运行完之后查看对应的变量，注意，没找到时对应变量将是0
-------------------------------------------------------------------------------------------------------------------*/
uint8 Right_Down_Find=0;
uint8 Left_Down_Find=0;
void Find_Down_Point(uint8 start,uint8 end)                 //用于判别正入或者斜入十字
{
//    if(start>=UVC_HEIGHT-1-5)//起始点位置校正，排除数组越界的可能
//        start=UVC_HEIGHT-1-5;
//    else if(start<=5)
//        start=5;
//    if(end>=UVC_HEIGHT-1-5)
//        end=UVC_HEIGHT-1-5;
//    else if(end<=5)
//        end=5;
    for(uint8 i=start;i>=end;i--)
    {
        if(//只找第一个符合条件的点
           abs(l_line_x[i]-l_line_x[i+1])<=5&&//角点的阈值可以更改
           abs(l_line_x[i+1]-l_line_x[i+2])<=5&&
           abs(l_line_x[i+2]-l_line_x[i+3])<=5&&
           (l_line_x[i]-l_line_x[i-2])>=8&&
           (l_line_x[i]-l_line_x[i-3])>=15&&
           (l_line_x[i]-l_line_x[i-4])>=15)
        {
            Left_Down_Find=i;//获取行数即可
        }
        if(//只找第一个符合条件的点
           abs(r_line_x[i]-r_line_x[i+1])<=5&&//角点的阈值可以更改
           abs(r_line_x[i+1]-r_line_x[i+2])<=5&&
           abs(r_line_x[i+2]-r_line_x[i+3])<=5&&
              (r_line_x[i]-r_line_x[i-2])<=-8&&
              (r_line_x[i]-r_line_x[i-3])<=-15&&
              (r_line_x[i]-r_line_x[i-4])<=-15)
        {
            Right_Down_Find=i;
        }
//        if(Left_Down_Find!=0&&Right_Down_Find!=0)//两个找到就退出
//        {
//            return;
//        }
    }

}

/********************正入十字补线****************/
void Add_Line(uint8 type,uint8 x1,int16 y1,uint8 x2,int16 y2)//左补线,补的是边界          左下（x1,y1）  左上（x2，y2）
{
    int hy;
    if(x1>=UVC_HEIGHT-1)//起始点位置校正，排除数组越界的可能
       x1=UVC_HEIGHT-1;
    else if(x1<=10)
        x1=10;
    if(y1>=UVC_WIDTH-1)
    y1=UVC_WIDTH-1;
    else if(y1<=10)
    y1=10;
    if(x2>=UVC_HEIGHT-1)
    x2=UVC_HEIGHT-1;
    else if(x2<=10)
         x2=10;
    if(y2>=UVC_WIDTH-1)
    y2=UVC_WIDTH-1;
    else if(y2<=10)
         y2=10;

    if(type == 1)//左补线,补的是边界          左下（x2,y2）  左上（x1，y1）
    {
        for(uint8 i=x2;i>=x1;i--)//根据斜率补线即可
        {
            hy=(i-x2)*(y2-y1)/(x2-x1)+y2;
            if(hy>=UVC_WIDTH-1)
                hy=UVC_WIDTH;
            else if(hy<=0)
                hy=0;
            l_line_x[i]=hy;
        }
    }
    else if(type == 2)//右补线，补的是边界         右下（x2,y2）  左上（x1，y1）
    {
        for(uint8 i=x2;i>=x1;i--)//根据斜率补线即可
        {
            hy=(i-x2)*(y2-y1)/(x2-x1)+y2;
            if(hy>=UVC_WIDTH-1)
                hy=UVC_WIDTH;
            else if(hy<=0)
                hy=0;
            r_line_x[i]=hy;
        }
    }
}
/********************斜入十字补线****************/
/*-------------------------------------------------------------------------------------------------------------------
  @brief     右左边界延长
  @param     延长起始行数，延长到某行
  Sample     Lengthen_Right_Boundry(int start,int end)；
  @note      从起始点向上找3个点，算出斜率，向下延长，直至结束点
-------------------------------------------------------------------------------------------------------------------*/
void Lengthen_Boundry(uint8 type,uint8 start,uint8 end)
{
    float k=0;
    if(start>=UVC_HEIGHT-1-5)//起始点位置校正，排除数组越界的可能
        start=UVC_HEIGHT-1-5;
    else if(start<=5)
        start=5;
    if(end>=UVC_HEIGHT-1-5)
        end=UVC_HEIGHT-1-5;
    else if(end<=5)
        end=5;
    if(type == 1)
    {
        k=(float)(l_line_x[start]-l_line_x[start-4])/5.0;//这里的k是斜率
        for(uint8 i=start;i<=end;i++)
        {
            l_line_x[i]=(int)(i-start)*k+l_line_x[start];//(x=(y-y1)*k+x1),点斜式变形
            if(l_line_x[i]>=UVC_WIDTH-1)
            {
                l_line_x[i]=UVC_WIDTH-1;
            }
            else if(l_line_x[i]<=0)
            {
                l_line_x[i]=0;
            }
        }
    }
    if(type == 2)
    {
        k=(float)(r_line_x[start]-r_line_x[start-4])/5.0;//这里的k是斜率
        for(uint8 i=start;i<=end;i++)
        {
            r_line_x[i]=(int)(i-start)*k+r_line_x[start];//(x=(y-y1)*k+x1),点斜式变形
            if(r_line_x[i]>=UVC_WIDTH-1)
            {
                r_line_x[i]=UVC_WIDTH-1;
            }
            else if(r_line_x[i]<=0)
            {
                r_line_x[i]=0;
            }
        }
    }
}
/*******************十字处理函数*****************/
void Handle_Cross()
{
    int down_search_start=0;//下角点搜索开始行

    if(l_lose_value > 38 && r_lose_value > 38)//十字必定有双边丢线，在有双边丢线的情况下再开始找角点
    {
        Find_Up_Point( UVC_HEIGHT-1-5, 20);
        if(Left_Up_Find==0&&Right_Up_Find==0)//只要没有同时找到两个上点，直接结束
        {
            return;
        }
    }
    if(Left_Up_Find!=0&&Right_Up_Find!=0)//找到两个上点，就认为找到十字了
    {

        down_search_start=Left_Up_Find>Right_Up_Find?Left_Up_Find:Right_Up_Find;//用两个上拐点坐标靠下者作为下点的搜索上限
        Find_Down_Point(UVC_HEIGHT-5,down_search_start+2);//在上拐点下2行作为下角点的截止行
        if(Left_Down_Find<=Left_Up_Find)
        {
            Left_Down_Find=0;//下点不可能比上点还靠上
        }
        if(Right_Down_Find<=Right_Up_Find)
        {
            Right_Down_Find=0;//下点不可能比上点还靠上
        }
        Lengthen_Boundry(1,Left_Up_Find,UVC_HEIGHT-1);
        Lengthen_Boundry(2,Right_Up_Find,UVC_HEIGHT-1);
//        if(Left_Down_Find!=0&&Right_Down_Find!=0)
//        {//四个点都在，无脑连线，这种情况显然很少
//            Add_Line(1,Left_Down_Find,l_line_x[Left_Down_Find+5],Left_Up_Find,l_line_x[Left_Up_Find-5]);
//            Add_Line(2,Right_Down_Find,r_line_x[Right_Down_Find+5],Right_Up_Find,r_line_x[Right_Up_Find-5]);
//        }
//        else if(Left_Down_Find==0&&Right_Down_Find!=0)//11//这里使用的是斜率补线
//        {//三个点                                     //01
//            Lengthen_Boundry(1,Left_Up_Find,UVC_HEIGHT-1);
//            Add_Line(2,Right_Down_Find,r_line_x[Right_Down_Find],Right_Up_Find,r_line_x[Right_Up_Find]);
//        }
//        else if(Left_Down_Find!=0&&Right_Down_Find==0)//11
//        {//三个点                                      //10
//            Add_Line(1,Left_Down_Find,l_line_x[Left_Down_Find],Left_Up_Find,l_line_x[Left_Up_Find]);
//            Lengthen_Boundry(2,Right_Up_Find,UVC_HEIGHT-1);
//        }
//        else if(Left_Down_Find==0&&Right_Down_Find==0)//11
//        {//就俩上点                                    //00
//            Lengthen_Boundry(1,Left_Up_Find,UVC_HEIGHT-1);
//            Lengthen_Boundry(2,Right_Up_Find,UVC_HEIGHT-1);
//        }
        ips200_show_string(190,25,"Left_Up_Find");
        ips200_show_uint(190, 40, Left_Up_Find, 5);           //左推动风扇转速
        ips200_show_string(190,55,"Right_Up_Find");
        ips200_show_uint(190, 70, Right_Up_Find, 5);          //右推动风扇转速

        ips200_show_string(190,85,"Left_Down_Find");
        ips200_show_uint(190, 100, Left_Down_Find, 5);        //左辅助风扇转速
        ips200_show_string(190,115,"Right_Down_Find");
        ips200_show_uint(190, 130, Right_Down_Find, 5);       //右辅助风扇转速
    }
    else {
        road_type.Cross=0;
    }

        if(r_haveline >= 38 && l_haveline >= 38)
        {
            road_type.Cross=0;
            Left_Down_Find=0;
            Right_Down_Find=0;
        }


//    Find_Up_Point(UVC_HEIGHT-1 - 30,20);
//    Find_Down_Point(Left_Up_Find,UVC_HEIGHT-1 - 5);
//    if(Left_Down_Find!=0 && Right_Down_Find!=0)
//    {
//        Add_Line(1,Left_Down_Find,l_line_x[Left_Down_Find],Left_Up_Find,l_line_x[Left_Up_Find]);
//        Add_Line(1,Right_Down_Find,r_line_x[Right_Down_Find],Right_Up_Find,r_line_x[Right_Up_Find]);
//    }
//    else if(Left_Down_Find==0 && Right_Down_Find==0)
//    {
//        Lengthen_Boundry(1,Left_Up_Find,UVC_HEIGHT-1);
//        Lengthen_Boundry(2,Right_Up_Find,UVC_HEIGHT-1);
//    }
//    if(r_haveline >= 38 && l_haveline >= 38)
//    {
//        road_type.Cross=0;
//    }
}

/********************用于角速度积分****************/

uint32 Left_CirqueIntegration = 0, Right_CirqueIntegration = 0;
void Integration(void)
{
  /* 
  if(L_integration_flag == 1)
   {
       Left_CirqueIntegration += fabs(imu660ra_gyro_z*0.01);
   }
   if(R_integration_flag == 1)
   {
       Right_CirqueIntegration += fabs(imu660ra_gyro_z*0.01);
   }
   if(road_type.L_RoadBlock)
   {
       distance.L_RoadBlock  += fabs(int16real_speed*0.1);
   }
   if(road_type.R_RoadBlock)
   {
       distance.R_RoadBlock  += fabs(int16real_speed*0.1);
   }
*/
}

/********************Test_Element_first_end****************/


void Datasend_Side_line(uint8 type)
{
    if(type == 1)
    {
        for (uint8 i = 0; i < UVC_HEIGHT - 2; i++)
        {
           if(i == 0)
           {
               printf("start");
               printf("\n");
           }
           else
           {
               printf("%d", l_line_x[i]);
               printf(",");
               if(i%10 == 0)
               {
                   printf("\n");
               }
           }
        }
    }
    else if(type == 2)
    {
        for (uint8 i = 0; i < UVC_HEIGHT - 2; i++)
        {
           if(i == 0)
           {
               printf("start");
               printf("\n");
           }
           else
           {
               printf("%d", r_line_x[i]);
               printf(",");
               if(i%10 == 0)
               {
                   printf("\n");
               }
           }
        }
    }
    else if(type == 3)
    {
        for (uint8 i = UVC_HEIGHT - 1; i > 2; i--)
        {
           if(i == UVC_HEIGHT - 1)
           {
               printf("start");
               printf("\n");
           }
           else
           {
               printf("%d", r_line_x[i] - l_line_x[i]);
               printf(",");
               if(i%10 == 0)
               {
                   printf("\n");
               }
           }
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     在一个点画十字
// 参数说明     void
// 返回参数     uint32 电压
// 使用示例     Voltage_Detection(ADC_VOLTAGE);
//-------------------------------------------------------------------------------------------------------------------
void Draw_Cross(uint16 x_point, uint16 y_point, const uint16 color, uint8 num)
{
    for (uint8 x = x_point -num; x < x_point +num; x++)
    {
        ips200_draw_point(x, y_point, color);
    }
    for (uint8 y = y_point -num; y < y_point +num; y++)
    {
        ips200_draw_point(x_point, y, color);
    }
}


int clip(int x, int low, int up)
{
    return x > up ? up : x < low ? low : x;
}


float fclip(float x, float low, float up)
{
    return x > up ? up : x < low ? low : x;
}


//中线滑动平均值滤波
int16 Sum1;
void HDPJ_lvbo(int16 data[], uint8 N, uint8 size)
{//data[] 要滤波的数组   N 滤波强度   size 滤波起始点
    Sum1 = 0;
    for(uint8 j = 0; j < size; j++)
    {
        if(j < N /2)
        {
            for(uint8 k = 0; k < N; k++)
            {
                Sum1 += data[j +k];
            }
            data[j] = Sum1 /N;
        }
        else if(j < size - N/2)
        {
            for(uint8 k = 0; k < N/2; k++)
            {
                Sum1 += (data[j +k] +data[j -k]);
            }
            data[j] = Sum1 /N;
        }
        else
        {
            for(uint8 k = 0; k < size -j; k++)
            {
                Sum1 += data[j +k];
            }
            for(uint8 k =0; k <(N -size +j); k++)
            {
                Sum1 += data[j -k];
            }
            data[j] = Sum1 /N;
        }
        Sum1 = 0;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief     识别斑马线
//  @param      start_point：识别的y起点   end_point:识别的y终点  qiangdu：识别的强度
//  @return     void
//  Sample usage:      Check_Starting_Line(65, 55, 1);
//  explain：  qiangdu越小越容易判断成功，且不会误判
//             该函数后面几句代码进行了车库方向的判断
//-------------------------------------------------------------------------------------------------------------------
uint8 flag_starting_line;
uint8 black_blocks_l, black_blocks_r;
uint8 cursor_l, cursor_r;
void Check_Starting_Line(uint8 start_point, uint8 end_point, uint8 qiangdu)
{
    uint8 times = 0;
    flag_starting_line = 0;
    for(uint8 y = start_point; y >= end_point; y--)  //判断斑马线
    {
        black_blocks_l = black_blocks_r = cursor_l = cursor_r = 0;
        for(uint8 width_l = UVC_WIDTH/2,width_r = UVC_WIDTH/2; width_l > 1 && width_r < UVC_WIDTH-2; width_l--,width_r++)
        {
            if(image_01[y][width_l] == 0)
            {
                if(cursor_l > 16)
                {
                    break;    //当黑色元素超过栈长度的操作
                }
                else
                {
                    cursor_l++;
                }
            }
            else
            {
                if(cursor_l >= qiangdu && cursor_l <= qiangdu+6)
                {
                    black_blocks_l++;
                    cursor_l = 0;
                }
                else
                {
                    cursor_l = 0;
                }
            }

        if(image_01[y][width_r] == 0)
        {
            if (cursor_r > 16)
            {
                break;    //当黑色元素超过栈长度的操作
            }
            else
            {
                cursor_r++;
            }
        }
        else
        {
            if(cursor_r >= qiangdu && cursor_r <= qiangdu + 6)
            {
                black_blocks_r++;
                cursor_r = 0;
            }
            else
            {
                cursor_r = 0;
            }
        }
    }

    if((black_blocks_l + black_blocks_r) >= 4)
    {
        times++;
    }

    if(times >= (start_point -end_point -5))
    {
        flag_starting_line = 1;
    }
    else
    {
        flag_starting_line = 0;
    }
  }
}