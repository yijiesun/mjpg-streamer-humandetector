#ifndef FAST_DPM_H
#define FAST_DPM_H
#include "opencv2/opencv.hpp"  
#include "PM_type.h"
using namespace PM_type;

struct VEC_BOX
{
	FLOATS_7 A;//ʸ�����
	FLOATS_7 B;//ʸ���յ�
	int C;//��¼Ԥ��·��
	float D;//ABʸ������
};

class FastDPM
{
public:
	FastDPM( string config_file);
	void init();
	Mat		prepareImg( Mat &img_uint8); // returns a float type image and set "img_color" with appropriate value	

	void		detect( Mat &img, float score_thresh = -FLOAT_INF, bool show_hints = true );
	vector<VEC_BOX>             P;

	vector<FLOATS_7>	detections; //����ÿ��knnǰ��box�е�pedestrian
	vector<FLOATS_7>	detections_full_last;//��������img�е�pedestrian����һ֡��
	vector<FLOATS_7>	detections_full_predict;//��������img��Ԥ���pedestrian����ǰ֡��
	vector<FLOATS_7>	detections_full_current;//��������img�е�pedestrian����ǰ֡��
	void find_single_box(vector<FLOATS_7> &L, vector<FLOATS_7> &A);//��L,�ҳ���Ӧ�ĵ��㼯A
	void find_single_box_vec(vector<VEC_BOX> &L, vector<VEC_BOX> &A);
	float calcDis(FLOATS_7 &last, FLOATS_7&curr);
	void togetherAllBox(bool is_video);
	void  draw_img(Mat &img, vector<FLOATS_7> &detections, Scalar &drawColor, bool drawFont);
	void use_2_box_calc_vec();
	void use_vec_box_calc_vec();
	void  draw_once(Mat &img, float ElapsedTime, int frameCnt);
	void predict_box_by_vec();
	void clear_bad_vec();
	void clear_bad_box();
	void setup(string config_file);
	//void	show( Mat &the_img, bool specification=false );

	/* All useful stuff have been declared above. */
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/* Below are not supposed to be used. */

public:
	MODEL		model;	
	FEATURE_PYRAMID		pyra;

	Mat		img_color;
	double tooSmalltoDrop;
	double      zoom_value;
	int drawCnt;
	bool		hints;
	int			start_clock;
	int			end_clock;
	int			prag_start;
	int			prag_end;
	int         frame_cnt;
	float Elapsed_Time_ROI;
	float Elapsed_Time_One_Frame;
	float box_track_distance;//��֡��С�ڴ˾����2��boxΪͬһĿ��box
	int predict_dis;//������ʧ��ô��֮֡��������ٴ�box
	Mat	img_frame;
	Mat	img_roi;
	Mat	img_roi_big;
	Mat	img_input;
	int x0, y0, w0, h0;
	char model_file[400];
	char log_file[200];
	char input_video_file[200];
	char knn_roi_tmp_file[200];
	char result_video_file[200];
	char img_dir[200];
	char result_img_dir[200];
	FILE *stream;
	FILE *read_setup;
	int knn_conf[10];
	float knn_over_percent;
	float score_thresh;
	bool open_pic_not_video;
	bool is_video;
	bool is_predict_box;
	bool is_use_knn;
	float font_size;
	int camera_width;
	int camera_height;
	int camera_fps;
	bool is_save_result_video;
};

#endif