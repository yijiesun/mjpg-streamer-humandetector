/*******************************************************************************
#                                                                              #
# OpenCV input plugin                                                          #
# Copyright (C) 2016 Dustin Spicuzza                                           #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dlfcn.h>
#include <pthread.h>

#include "input_opencv.h"

#include "opencv2/opencv.hpp"
#include "fff/fff.h"

#include "dpm/config.h"
#include <fstream>
#include <dirent.h>


using namespace cv;
using namespace std;

int IMG_WID_;
int IMG_HGT_;
Mat src, dst;

int frame_cnt;
float Elapsed_Time_One_Frame;
bool quit;
vector<FLOATS_7>	detections_full_current;

pthread_mutex_t mutex;
VideoCapture v_capture;
VideoWriter outputVideo;
FastDPM	PM("/home/pi/work/mjpg-streamer-master/mjpg-streamer-experimental/plugins/input_opencv/dpm/config.txt");


/* private functions and variables to this plugin */
static globals     *pglobal;

typedef struct {
    char *filter_args;
    int fps_set, fps,
        quality_set, quality,
        co_set, co,
        br_set, br,
        sa_set, sa,
        gain_set, gain,
        ex_set, ex;
} context_settings;

// filter functions
typedef bool (*filter_init_fn)(const char * args, void** filter_ctx);
typedef Mat (*filter_init_frame_fn)(void* filter_ctx);
typedef void (*filter_process_fn)(Mat &src, Mat &dst);
typedef void (*filter_free_fn)(void* filter_ctx);


typedef struct {
    pthread_t   worker;
    VideoCapture capture;
    
    context_settings *init_settings;
    
    void* filter_handle;
    void* filter_ctx;
    
    filter_init_fn filter_init;
    filter_init_frame_fn filter_init_frame;
    filter_process_fn filter_process;
    filter_free_fn filter_free;
    
} context;

void *dpm_thread(void *threadarg);


void *worker_thread(void *);
void worker_cleanup(void *);

#define INPUT_PLUGIN_NAME "OpenCV Input plugin"
static char plugin_name[] = INPUT_PLUGIN_NAME;

static void null_filter(void* filter_ctx, Mat &src, Mat &dst) {
    dst = src;
}

static void help() {
    
    fprintf(stderr,
    " ---------------------------------------------------------------\n" \
    " Help for input plugin..: "INPUT_PLUGIN_NAME"\n" \
    " ---------------------------------------------------------------\n" \
    " The following parameters can be passed to this plugin:\n\n" \
    " [-d | --device ].......: video device to open (your camera)\n" \
    " [-r | --resolution ]...: the resolution of the video device,\n" \
    "                          can be one of the following strings:\n" \
    "                          ");
    
    resolutions_help("                          ");
    
    fprintf(stderr,
    " [-f | --fps ]..........: frames per second\n" \
    " [-q | --quality ] .....: set quality of JPEG encoding\n" \
    " ---------------------------------------------------------------\n" \
    " Optional parameters (may not be supported by all cameras):\n\n"
    " [-br ].................: Set image brightness (integer)\n"\
    " [-co ].................: Set image contrast (integer)\n"\
    " [-sh ].................: Set image sharpness (integer)\n"\
    " [-sa ].................: Set image saturation (integer)\n"\
    " [-ex ].................: Set exposure (off, or integer)\n"\
    " [-gain ]...............: Set gain (integer)\n"
    " ---------------------------------------------------------------\n" \
    " Optional filter plugin:\n" \
    " [ -filter ]............: filter plugin .so\n" \
    " [ -fargs ].............: filter plugin arguments\n" \
    " ---------------------------------------------------------------\n\n"\
    );
}

static context_settings* init_settings() {
    context_settings *settings;
    
    settings = (context_settings*)calloc(1, sizeof(context_settings));
    if (settings == NULL) {
        IPRINT("error allocating context");
        exit(EXIT_FAILURE);
    }
    
    settings->quality = 80;
    return settings;
}

/*** plugin interface functions ***/

/******************************************************************************
Description.: parse input parameters
Input Value.: param contains the command line string and a pointer to globals
Return Value: 0 if everything is ok
******************************************************************************/


int input_init(input_parameter *param, int plugin_no)
{
    const char * device = "default";
    const char *filter = NULL, *filter_args = "";
    int width = 640, height = 480, i, device_idx;
    
    input * in;
    context *pctx;
    context_settings *settings;
    
    pctx = new context();
    
    settings = pctx->init_settings = init_settings();
    pglobal = param->global;
    in = &pglobal->in[plugin_no];
    in->context = pctx;

    param->argv[0] = plugin_name;

    /* show all parameters for DBG purposes */
    for(i = 0; i < param->argc; i++) {
        DBG("argv[%d]=%s\n", i, param->argv[i]);
    }

    /* parse the parameters */
    reset_getopt();
    while(1) {
        int option_index = 0, c = 0;
        static struct option long_options[] = {
            {"h", no_argument, 0, 0},
            {"help", no_argument, 0, 0},
            {"d", required_argument, 0, 0},
            {"device", required_argument, 0, 0},
            {"r", required_argument, 0, 0},
            {"resolution", required_argument, 0, 0},
            {"f", required_argument, 0, 0},
            {"fps", required_argument, 0, 0},
            {"q", required_argument, 0, 0},
            {"quality", required_argument, 0, 0},
            {"co", required_argument, 0, 0},
            {"br", required_argument, 0, 0},
            {"sa", required_argument, 0, 0},
            {"gain", required_argument, 0, 0},
            {"ex", required_argument, 0, 0},
            {"filter", required_argument, 0, 0},
            {"fargs", required_argument, 0, 0},
            {0, 0, 0, 0}
        };
    
        /* parsing all parameters according to the list above is sufficent */
        c = getopt_long_only(param->argc, param->argv, "", long_options, &option_index);

        /* no more options to parse */
        if(c == -1) break;

        /* unrecognized option */
        if(c == '?') {
            help();
            return 1;
        }

        /* dispatch the given options */
        switch(option_index) {
        /* h, help */
        case 0:
        case 1:
            help();
            return 1;
        /* d, device */
        case 2:
        case 3:
            device = optarg;
            break;
        /* r, resolution */
        case 4:
        case 5:
            DBG("case 4,5\n");
            parse_resolution_opt(optarg, &width, &height);
            break;
        /* f, fps */
        case 6:
        OPTION_INT(7, fps)
            break;
        /* q, quality */
        case 8:
        OPTION_INT(9, quality)
            settings->quality = MIN(MAX(settings->quality, 0), 100);
            break;
        OPTION_INT(10, co)
            break;
        OPTION_INT(11, br)
            break;
        OPTION_INT(12, sa)
            break;
        OPTION_INT(13, gain)
            break;
        OPTION_INT(14, ex)
            break;
            
        /* filter */
        case 15:
            filter = optarg;
            break;
            
        /* fargs */
        case 16:
            filter_args = optarg;
            break;
            
        default:
            help();
            return 1;
        }
    }

    IPRINT("device........... : %s\n", device);
    IPRINT("Desired Resolution: %i x %i\n", width, height);
    IMG_WID_ = width;
	IMG_HGT_ = height;
    // need to allocate a VideoCapture object: default device is 0
    try {
        if (!strcasecmp(device, "default")) {
            pctx->capture.open(0);
        } else if (sscanf(device, "%d", &device_idx) == 1) {
            pctx->capture.open(device_idx);
        } else {
            pctx->capture.open(device);
        }
    } catch (Exception e) {
        IPRINT("VideoCapture::open() failed: %s\n", e.what());
        goto fatal_error;
    }
    
    // validate that isOpened is true
    if (!pctx->capture.isOpened()) {
        IPRINT("VideoCapture::open() failed\n");
        goto fatal_error;
    }
    
    pctx->capture.set(CV_CAP_PROP_FRAME_WIDTH, width);
    pctx->capture.set(CV_CAP_PROP_FRAME_HEIGHT, height);
    
    if (settings->fps_set)
        pctx->capture.set(CV_CAP_PROP_FPS, settings->fps);
    
    /* filter stuff goes here */
    if (filter != NULL) {
        
        IPRINT("filter........... : %s\n", filter);
        IPRINT("filter args ..... : %s\n", filter_args);
        
        pctx->filter_handle = dlopen(filter, RTLD_LAZY | RTLD_GLOBAL);
        if(!pctx->filter_handle) {
            LOG("ERROR: could not find input plugin\n");
            LOG("       Perhaps you want to adjust the search path with:\n");
            LOG("       # export LD_LIBRARY_PATH=/path/to/plugin/folder\n");
            LOG("       dlopen: %s\n", dlerror());
            goto fatal_error;
        }
        
        pctx->filter_init = (filter_init_fn)dlsym(pctx->filter_handle, "filter_init");
        if (pctx->filter_init == NULL) {
            LOG("ERROR: %s\n", dlerror());
            goto fatal_error;
        }
        
        pctx->filter_process = (filter_process_fn)dlsym(pctx->filter_handle, "filter_process");
        if (pctx->filter_process == NULL) {
            LOG("ERROR: %s\n", dlerror());
            goto fatal_error;
        }
        
        pctx->filter_free = (filter_free_fn)dlsym(pctx->filter_handle, "filter_free");
        if (pctx->filter_free == NULL) {
            LOG("ERROR: %s\n", dlerror());
            goto fatal_error;
        }
        
        // optional functions
        pctx->filter_init_frame = (filter_init_frame_fn)dlsym(pctx->filter_handle, "filter_init_frame");
        
        // initialize it
        if (!pctx->filter_init(filter_args, &pctx->filter_ctx)) {
            goto fatal_error;
        }
        
    } else {
        pctx->filter_handle = NULL;
        pctx->filter_ctx = NULL;
        pctx->filter_process = fff_process;
        pctx->filter_free = NULL;
    }
    
    return 0;
    
fatal_error:
    worker_cleanup(in);
    closelog();
    exit(EXIT_FAILURE);
}

/******************************************************************************
Description.: stops the execution of the worker thread
Input Value.: -
Return Value: 0
******************************************************************************/
int input_stop(int id)
{
    input * in = &pglobal->in[id];
    context *pctx = (context*)in->context;
    
    if (pctx != NULL) {
        DBG("will cancel input thread\n");
        pthread_cancel(pctx->worker);
    }
    return 0;
}

/******************************************************************************
Description.: starts the worker thread and allocates memory
Input Value.: -
Return Value: 0
******************************************************************************/
int input_run(int id)
{
    input * in = &pglobal->in[id];
    context *pctx = (context*)in->context;
    
    in->buf = NULL;
    in->size = 0;


	pthread_mutex_init(&mutex, NULL);

	
    if(pthread_create(&pctx->worker, 0, worker_thread, in) != 0) {
        worker_cleanup(in);
        fprintf(stderr, "could not start worker thread\n");
        exit(EXIT_FAILURE);
    }

	pthread_t threads_dpm;
	int rc = pthread_create(&threads_dpm, NULL, dpm_thread, NULL);
	
    pthread_detach(pctx->worker);
	v_capture.release();

    return 0;
}

void *worker_thread(void *arg)
{


    input * in = (input*)arg;
    context *pctx = (context*)in->context;
    context_settings *settings = (context_settings*)pctx->init_settings;
    
    /* set cleanup handler to cleanup allocated resources */
    pthread_cleanup_push(worker_cleanup, arg);

    /* set VideoCapture options */
    #define CVOPT_OPT(prop, var, desc) \
        if (!pctx->capture.set(prop, settings->var)) {\
            IPRINT("%-18s: %d\n", desc, settings->var); \
        } else {\
            fprintf(stderr, "Failed to set " desc "\n"); \
        }
    
    #define CVOPT_SET(prop, var, desc) \
        if (settings->var##_set) { \
            CVOPT_OPT(prop, var,desc) \
        }
    
    CVOPT_SET(CV_CAP_PROP_FPS, fps, "frames per second")
    CVOPT_SET(CV_CAP_PROP_BRIGHTNESS, co, "contrast")
    CVOPT_SET(CV_CAP_PROP_CONTRAST, br, "brightness")
    CVOPT_SET(CV_CAP_PROP_SATURATION, sa, "saturation")
    CVOPT_SET(CV_CAP_PROP_GAIN, gain, "gain")
    CVOPT_SET(CV_CAP_PROP_EXPOSURE, ex, "exposure")
    
    /* setup imencode options */
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(settings->quality); // 1-100
    
    free(settings);
    pctx->init_settings = NULL;
    settings = NULL;
    
  
    vector<uchar> jpeg_buffer;
    
    // this exists so that the numpy allocator can assign a custom allocator to
    // the mat, so that it doesn't need to copy the data each time
    //if (pctx->filter_init_frame != NULL)
    //    src = pctx->filter_init_frame(pctx->filter_ctx);
    
	Elapsed_Time_One_Frame = 0.0;
	frame_cnt = 0;
	Scalar drawColor = CV_RGB(255, 0, 0);
	if (PM.is_save_result_video)
	{
		Size sWH = Size(IMG_WID_, IMG_HGT_);
		bool ret = outputVideo.open(PM.result_video_file, CV_FOURCC('M', 'P', '4', '2'), PM.camera_fps, sWH);
	}
	
    while (!pglobal->stop) {
		pthread_mutex_lock(&mutex);
		if (!pctx->capture.read(src))
            break; // TODO
		
		PM.draw_img(src, detections_full_current, drawColor, 1);
		PM.draw_once(src, Elapsed_Time_One_Frame, frame_cnt);

		pthread_mutex_unlock(&mutex);

	
        // call the filter function
        //pctx->filter_process(src, dst);
		//fff_process(src, dst);

        /* copy JPG picture to global buffer */
        pthread_mutex_lock(&in->db);
        
        // take whatever Mat it returns, and write it to jpeg buffer
        imencode(".jpg", src, jpeg_buffer, compression_params);
        
        // TODO: what to do if imencode returns an error?
        
        // std::vector is guaranteed to be contiguous
        in->buf = &jpeg_buffer[0];
        in->size = jpeg_buffer.size();
        
        /* signal fresh_frame */
        pthread_cond_broadcast(&in->db_update);
        pthread_mutex_unlock(&in->db);
    }
    
    IPRINT("leaving input thread, calling cleanup function now\n");
    pthread_cleanup_pop(1);

    return NULL;
}

/******************************************************************************
Description.: this functions cleans up allocated resources
Input Value.: arg is unused
Return Value: -
******************************************************************************/
void worker_cleanup(void *arg)
{
    input * in = (input*)arg;
    if (in->context != NULL) {
        context *pctx = (context*)in->context;
        
        if (pctx->filter_free != NULL && pctx->filter_ctx != NULL) {
            pctx->filter_free(pctx->filter_ctx);
            pctx->filter_free = NULL;
        }
        
        if (pctx->filter_handle != NULL) {
            dlclose(pctx->filter_handle);
            pctx->filter_handle = NULL;
        }
        
        delete pctx;
        in->context = NULL;
    }
}

void *dpm_thread(void *threadarg)
{
	while (1)
	{
	printf("hhhh\n");
	//waitKey(1000);

		pthread_mutex_lock(&mutex);
		if(src.empty())
		{
			pthread_mutex_unlock(&mutex);
			continue;
			waitKey(30);
		}
			
		PM.img_frame = src;
		pthread_mutex_unlock(&mutex);


		PM.init();
		PM.detections_full_current.clear();
		PM.img_input = PM.prepareImg(PM.img_frame);
		
		PM.detect(PM.img_input, PM.score_thresh, false);
	
		PM.togetherAllBox(0);
		PM.frame_cnt++;
		PM.clear_bad_box();



		pthread_mutex_lock(&mutex);
		detections_full_current.clear();
		detections_full_current.assign(PM.detections_full_current.begin(), PM.detections_full_current.end());
		frame_cnt = PM.frame_cnt;
		Elapsed_Time_One_Frame = PM.Elapsed_Time_One_Frame;
		pthread_mutex_unlock(&mutex);
		if (quit)
			pthread_exit(NULL);

	
	}
}


