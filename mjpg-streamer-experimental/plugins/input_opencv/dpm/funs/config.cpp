#include "../FastDPM.h"
bool _str_cmp(char* a, char *b)
{	
	int sum = 0;
	for (int i = 0; b[i] != '\0'; i++)
		sum++;
	char tmp[200] = {""};
	strncpy(tmp, a + 0, sum);
	for (int i = 0; a[i] != '\0'; i++)
	{
		if (a[i] == '\n')
			a[i] = NULL;
	}
	return !strcmp(tmp,b);
}

void FastDPM::setup(string config_file)
{
	if ((read_setup = fopen(config_file.c_str(), "r")) == NULL) {
		puts("Fail to open config.txt!");
		exit(0);
	}
	char str[400];
	while (fgets(str, 400, read_setup) != NULL) {
		//printf("%s\n", str);
		if (_str_cmp(str, "model_file"))
		{
			strncpy(model_file, str + 11, 400);
			model_file[strlen(model_file)-1]=0;
		}
		else if (_str_cmp(str, "log_file"))
		{
			strncpy(log_file, str + 9, 200);
			stream = fopen(log_file, "w");
		}
		else if (_str_cmp(str, "input_video_file"))
		{
			strncpy(input_video_file, str + 17, 200);
		}
		else if (_str_cmp(str, "knn_roi_tmp_file"))
		{
			strncpy(knn_roi_tmp_file, str + 17, 200);
		}
		else if (_str_cmp(str, "result_video_file"))
		{
			strncpy(result_video_file, str + 18, 200);
		}
		else if (_str_cmp(str, "img_dir"))
		{
			strncpy(img_dir, str + 8, 200);
		}
		else if (_str_cmp(str, "result_img_dir"))
		{
			strncpy(result_img_dir, str + 15, 200);
		}
		else if (_str_cmp(str, "knn_bgs_set"))
		{
			char knn_bgs_set[200];
			strncpy(knn_bgs_set, str + 12, 200);
			const char * split = ",";
			char *p = strtok(knn_bgs_set, split);
			int a;
			int i = 0;
			while (p != NULL)
			{
				sscanf(p, "%d", &a);
				knn_conf[i++] = a;
				p = strtok(NULL, split);
			}
			
		}
		else if (_str_cmp(str, "score_thresh"))
		{
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%f", &score_thresh);
		}
		else if (_str_cmp(str, "knn_over_percent"))
		{
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%f", &knn_over_percent);
		}
		else if (_str_cmp(str, "open_pic_not_video"))
		{
			int tmp;
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%d", &tmp);
			open_pic_not_video = bool(tmp);
		}
		else if (_str_cmp(str, "is_use_knn"))
		{
			int tmp;
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%d", &tmp);
			is_use_knn = bool(tmp);
		}
		else if (_str_cmp(str, "is_save_result_video"))
		{
			int tmp;
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%d", &tmp);
			is_save_result_video = bool(tmp);
		}
		else if (_str_cmp(str, "box_track_distance"))
		{
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%f", &box_track_distance);
		}
		else if (_str_cmp(str, "font_size"))
		{
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%f", &font_size);
		}
		else if (_str_cmp(str, "predict_dis"))
		{
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%d", &predict_dis);
		}
		else if (_str_cmp(str, "camera_width"))
		{
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%d", &camera_width);
		}
		else if (_str_cmp(str, "camera_height"))
		{
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%d", &camera_height);
		}
		else if (_str_cmp(str, "camera_fps"))
		{
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%d", &camera_fps);
		}
		else if (_str_cmp(str, "is_predict_box"))
		{
			int tmp;
			const char * split = " ";
			char *p = strtok(str, split);
			p = strtok(NULL, split);
			sscanf(p, "%d", &tmp);
			is_predict_box = bool(tmp);
		}
	}
	printf("---------config---------\n");
	printf("model_file: %s\n", model_file);
	printf("log_file: %s\n", log_file);
	printf("score_thresh: %f\n", score_thresh);
	printf("font_size: %f\n", font_size);

	printf("\nopen_pic_not_video: %d\n", open_pic_not_video);
	if (open_pic_not_video)
	{
		printf("img_dir: %s\n", img_dir);
		printf("result_img_dir: %s\n\n", result_img_dir);
	}
	else
	{
		printf("\ninput_video_file: %s\n", input_video_file);
		printf("camera_width: %d\n", camera_width);
		printf("camera_height: %d\n", camera_height);
		printf("camera_fps: %d\n", camera_fps);
		printf("is_use_knn: %d\n", is_use_knn);
		printf("is_save_result_video: %d\n", is_save_result_video);
		printf("result_video_file: %s\n", result_video_file);
		if (is_use_knn)
		{
			printf("knn_roi_tmp_file: %s\n", knn_roi_tmp_file);
			printf("knn_over_percent: %f\n", knn_over_percent);
			printf("knn_bgs_set: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\n", knn_conf[0], knn_conf[1],
				knn_conf[2], knn_conf[3], knn_conf[4], knn_conf[5], knn_conf[6], knn_conf[7], knn_conf[8], knn_conf[9]);
		}
	}
	printf("\nis_predict_box: %d\n", is_predict_box);
	if (is_predict_box)
	{
		printf("box_track_distance: %f\n", box_track_distance);
		printf("predict_dis: %d\n\n", predict_dis);
	}
	printf("---------conend---------\n");





	fprintf( stream, "---------config---------\n");
	fprintf( stream, "model_file: %s\n", model_file);
	fprintf( stream, "log_file: %s\n", log_file);
	fprintf( stream, "score_thresh: %f\n", score_thresh);
	fprintf( stream, "font_size: %f\n", font_size);

	fprintf( stream, "\nopen_pic_not_video: %d\n", open_pic_not_video);
	if (open_pic_not_video)
	{
		fprintf( stream, "img_dir: %s\n", img_dir);
		fprintf( stream, "result_img_dir: %s\n\n", result_img_dir);
	}
	else
	{
		fprintf( stream, "\ninput_video_file: %s\n", input_video_file);
		fprintf( stream, "camera_width: %d\n", camera_width);
		fprintf( stream, "camera_height: %d\n", camera_height);
		fprintf( stream, "camera_fps: %d\n", camera_fps);
		fprintf( stream, "is_use_knn: %d\n", is_use_knn);
		fprintf( stream, "is_save_result_video: %d\n", is_save_result_video);
		fprintf( stream, "result_video_file: %s\n", result_video_file);
		if (is_use_knn)
		{
			fprintf( stream, "knn_roi_tmp_file: %s\n", knn_roi_tmp_file);
			fprintf( stream, "knn_over_percent: %f\n", knn_over_percent);
			fprintf( stream, "knn_bgs_set: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\n", knn_conf[0], knn_conf[1],
				knn_conf[2], knn_conf[3], knn_conf[4], knn_conf[5], knn_conf[6], knn_conf[7], knn_conf[8], knn_conf[9]);
		}
	}
	fprintf( stream, "\nis_predict_box: %d\n", is_predict_box);
	if (is_predict_box)
	{
		fprintf( stream, "box_track_distance: %f\n", box_track_distance);
		fprintf( stream, "predict_dis: %d\n\n", predict_dis);
	}
	fprintf( stream, "---------conend---------\n");
}