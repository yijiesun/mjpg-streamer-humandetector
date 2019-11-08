#ifndef CONFIG_H
#define CONFIG_H

#include "FastDPM.h"
#define NUM_THREADS     

class config_
{
public:
	void open_video(char *videoName, int wid, int hgt, int fps);

	VideoCapture v_capture;
	int IMG_WID_;
	int IMG_HGT_;
};


#endif