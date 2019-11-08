/**
    Example C++ OpenCV filter plugin that doesn't do anything. Copy/paste this
    to create your own awesome filter plugins for mjpg-streamer.
    
    At the moment, only the input_opencv.so plugin supports filter plugins.
*/

#include"fun.h"
#include <opencv2/core/core.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/highgui/highgui.hpp>  

using namespace cv;
using namespace std;

/**
    Initializes the filter. If you return something, it will be passed to the
    filter_process function, and should be freed by the filter_free function
*/
bool filter_init(const char * args, void** filter_ctx) {
    return true;
}

/**
    Called by the OpenCV plugin upon each frame
*/
void filter_process(void* filter_ctx, Mat &src, Mat &dst) {
    // TODO insert your filter code here
    int x0 = src.cols / 4;
    int x1 = src.cols * 3 / 4;
    int y0 = src.rows / 4;
    int y1 = src.rows * 3 / 4;
	cv::Point p0 = cv::Point(x0,y0);
	cv::Point p1 = cv::Point(x1, y1);
	//cv::line(src, p0, p1, cv::Scalar(0, 0, 255), 3, 4);

	//Mat M(4,4,CV_32FC3,Scalar(1,2,3));

	//cvtColor(src, dst, CV_BGR2GRAY);
    dst = src;
}

/**
    Called when the input plugin is cleaning up
*/
void filter_free(void* filter_ctx) {
    // empty
}

