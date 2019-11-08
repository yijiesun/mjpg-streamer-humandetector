#ifndef INPUT_OPENCV_H_
#define INPUT_OPENCV_H_
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#include "../../mjpg_streamer.h"
#include "../../utils.h"

void errno_exit(const char *s);
int xioctl(int fh, int request, void *arg);
void process_image(const void *p, int size,Mat &out);
int read_frame(Mat &out);
void mainloop(void);
void stop_capturing(void);
void start_capturing(void);
void uninit_device(void);
void init_mmap(void);
void init_device(void);
void open_device(char *dev_name_,int wid,int hgt);

int getTimesSecf();
void printTimesSecf();

void yuyv2yuv(unsigned char *yuv, unsigned char *yuyv, int width, int height);
int yuv422toyuv420(unsigned char *out, const unsigned char *in, unsigned int width, unsigned int height);

void yuyv_to_bgr(unsigned char* yuv,unsigned char* rgb);


int input_init(input_parameter* param, int id);
int input_stop(int id);
int input_run(int id);
int input_cmd(int plugin, unsigned int control_id, unsigned int typecode, int value);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_OPENCV_H_ */
