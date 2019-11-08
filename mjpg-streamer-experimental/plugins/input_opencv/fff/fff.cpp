/**
    Example C++ OpenCV filter plugin that doesn't do anything. Copy/paste this
    to create your own awesome filter plugins for mjpg-streamer.
    
    At the moment, only the input_opencv.so plugin supports filter plugins.
*/

#include"fff.h"

using namespace cv;
using namespace std;


void fff_process(Mat &src, Mat &dst) {
	int x0 = src.cols / 4;
	   int x1 = src.cols * 3 / 4;
	   int y0 = src.rows / 4;
	   int y1 = src.rows * 3 / 4;
	   cv::Point p0 = cv::Point(x0,y0);
	   cv::Point p1 = cv::Point(x1, y1);
	   cv::line(src, p0, p1, cv::Scalar(0, 0, 255), 3, 4);
	   dst = src;
}



