#include <iostream>
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <opencv2/video.hpp>
#include "opencv2/opencv.hpp"  
#include "input_opencv.h"
using namespace cv;
using namespace std;

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
	void *start;
	size_t length;
};

char dev_name[200];
int width_;
int height_;


static int fd = -1;        //DEVICE NUMBER
struct buffer *buffers;
static unsigned int n_buffers = 4;
char bufSec[100];
unsigned int count_=0;
static int frame_count = 100;
                              //FILE POINTOR
char fileDir[100];
#if 1

int getTimesSecf()
{
	time_t lastTime;
	time(&lastTime);
	time_t tt;
	time(&tt);
	tm* t;
	lastTime = tt;
	tt = tt + 8 * 3600;  // transform the time zone
	t = gmtime(&tt);

	struct  timeb   stTimeb;
	ftime(&stTimeb);
	return t->tm_hour*60*60+t->tm_min*60+t->tm_sec;
}

void printTimesSecf()
{
	time_t lastTime;
	time(&lastTime);
	time_t tt;
	time(&tt);
	tm* t;
	lastTime = tt;
	tt = tt + 8 * 3600;  // transform the time zone
	t = gmtime(&tt);

	struct  timeb   stTimeb;
	ftime(&stTimeb);
	
	printf("%d-%02d-%02d-%02d-%02d-%02d-%03d\n",
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec,
		stTimeb.millitm);
}


void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

int xioctl(int fh, int request, void *arg)
{
	int r;
	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);
	return r;
}
//处理函数
void process_image(const void *p, int size,Mat &rgb)
{

	yuyv_to_bgr((unsigned char*) p,rgb.data);

}

int read_frame(Mat &out)
{
	struct v4l2_buffer buf;
	CLEAR(buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
		errno_exit("VIDIOC_DQBUF");
	
	process_image(buffers[buf.index].start, buf.bytesused,out);

	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
		errno_exit("VIDIOC_QBUF");

	return 1;
}

void stop_capturing(void)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
		errno_exit("VIDIOC_STREAMOFF");
}

void start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
		errno_exit("VIDIOC_STREAMON");
}

void uninit_device(void)
{
	unsigned int i;

	for (i = 0; i < n_buffers; ++i)
		if (-1 == munmap(buffers[i].start, buffers[i].length))
			errno_exit("munmap");

	free(buffers);
}



void init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support "
				"memory mapping\n", dev_name);
			exit(EXIT_FAILURE);
		}
		else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n",
			dev_name);
		exit(EXIT_FAILURE);
	}

	buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < req.count; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		buffers[i].length = buf.length;
		buffers[i].start =
			mmap(NULL /* start anywhere */,
				buf.length,
				PROT_READ | PROT_WRITE /* required */,
				MAP_SHARED /* recommended */,
				fd, buf.m.offset);

		if (MAP_FAILED == buffers[i].start)
			errno_exit("mmap");
	}
}

void init_device(void)
{
	
	struct v4l2_capability cap;
	struct v4l2_format fmt;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {        //测试参数
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n",
				dev_name);
			exit(EXIT_FAILURE);
		}
		else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n",
			dev_name);
		exit(EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "%s does not support streaming i/o\n",
			dev_name);
		exit(EXIT_FAILURE);
	}

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width_;
	fmt.fmt.pix.height = height_;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))        //设置格式
		errno_exit("VIDIOC_S_FMT");
	init_mmap();
}

void open_device(char *dev_name_,int wid,int hgt)
{
	strcpy(dev_name,dev_name_);
	printf("open %s\n",dev_name_);
	printf("w:%d,h:%d\n",wid,hgt);
	dev_name[strlen(dev_name)-1]=0;
	width_ = wid;
	height_ = hgt;
	//加上O_NONBLOCK会出现如下错误
	//VIDIOC_DQBUF error 11, Resource temporarily unavailable
	fd = open(dev_name, O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
			dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void close_device(void)
{
	if (-1 == close(fd))
		errno_exit("close");

	fd = -1;
}
#endif

void yuyv_to_bgr(unsigned char* yuv,unsigned char* rgb)
{
    unsigned int i;
    unsigned char* y0 = yuv + 0;   
    unsigned char* u0 = yuv + 1;
    unsigned char* y1 = yuv + 2;
    unsigned char* v0 = yuv + 3;
 
    unsigned  char* b0 = rgb + 0;
    unsigned  char* g0 = rgb + 1;
    unsigned  char* r0 = rgb + 2;
    unsigned  char* b1 = rgb + 3;
    unsigned  char* g1 = rgb + 4;
    unsigned  char* r1 = rgb + 5;
   
    float rt0 = 0, gt0 = 0, bt0 = 0, rt1 = 0, gt1 = 0, bt1 = 0;
 
    for(i = 0; i <= (width_ * height_) / 2 ;i++)
    {
        bt0 = 1.164 * (*y0 - 16) + 2.018 * (*u0 - 128); 
        gt0 = 1.164 * (*y0 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128); 
        rt0 = 1.164 * (*y0 - 16) + 1.596 * (*v0 - 128); 
   
    	bt1 = 1.164 * (*y1 - 16) + 2.018 * (*u0 - 128); 
        gt1 = 1.164 * (*y1 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128); 
        rt1 = 1.164 * (*y1 - 16) + 1.596 * (*v0 - 128); 
    
      
       	        if(rt0 > 250)  	rt0 = 255;
		if(rt0< 0)    	rt0 = 0;	
 
		if(gt0 > 250) 	gt0 = 255;
		if(gt0 < 0)	gt0 = 0;	
 
		if(bt0 > 250)	bt0 = 255;
		if(bt0 < 0)	bt0 = 0;	
 
		if(rt1 > 250)	rt1 = 255;
		if(rt1 < 0)	rt1 = 0;	
 
		if(gt1 > 250)	gt1 = 255;
		if(gt1 < 0)	gt1 = 0;	
 
		if(bt1 > 250)	bt1 = 255;
		if(bt1 < 0)	bt1 = 0;	
					
		*r0 = (unsigned char)rt0;
		*g0 = (unsigned char)gt0;
		*b0 = (unsigned char)bt0;
	
		*r1 = (unsigned char)rt1;
		*g1 = (unsigned char)gt1;
		*b1 = (unsigned char)bt1;
 
        yuv = yuv + 4;
        rgb = rgb + 6;
        if(yuv == NULL)
          break;
 
        y0 = yuv;
        u0 = yuv + 1;
        y1 = yuv + 2;
        v0 = yuv + 3;
  
        b0 = rgb + 0;
        g0 = rgb + 1;
        r0 = rgb + 2;
        b1 = rgb + 3;
        g1 = rgb + 4;
        r1 = rgb + 5;
    }   
}

