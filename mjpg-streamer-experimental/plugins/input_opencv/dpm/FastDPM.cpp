#include "FastDPM.h"

FastDPM::FastDPM(string config_file)
{
	frame_cnt = 0;
	setup(config_file);
	tooSmalltoDrop = knn_conf[9] / 2;
	load_model( model_file, model );
	analyze_model( model );
}
void FastDPM::init()
{
	drawCnt = 0;
	Elapsed_Time_One_Frame = 0;
	Elapsed_Time_ROI = 0;

	if (img_frame.depth() != CV_8U || (img_frame.channels() != 1 && img_frame.channels() != 3)) {
		printf("Function prepareImg() only takes as input an image of 1 or 3 channels of uint8 type!");
		throw	runtime_error("");
	}
	if (img_frame.channels() == 1)
		cvtColor(img_frame, img_color, CV_GRAY2RGB);
	else if (img_frame.channels() == 3)
		img_color = img_frame.clone();
}

Mat FastDPM::prepareImg( Mat &img_uint8)
// Make a uint8 type image into a float type mat with 3 channels
{
	Mat	imgTmp;
	if( img_uint8.depth()!=CV_8U || (img_uint8.channels()!=1 && img_uint8.channels()!=3) ){
		printf("Function prepareImg() only takes as input an image of 1 or 3 channels of uint8 type!");
		throw	runtime_error("");
	}
	if( img_uint8.channels()==1 )
		cvtColor( img_uint8, imgTmp, CV_GRAY2RGB );
	else if( img_uint8.channels()==3 )
		imgTmp = img_uint8;
	Mat	img;
	imgTmp.convertTo( img, CV_32FC3 );

	return	img;

}

void	loadPyramid( const string FileName, FEATURE_PYRAMID &pyra );
void FastDPM::detect( Mat &img, float score_thresh /* = -FLOAT_INF */, bool show_hints /* = true */ )
{
	detections.clear();
	prag_start = yuGetCurrentTime('M');
	// 1. Feature pyramid <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<	
	if(show_hints){ printf("Calculating feature pyramid ...\t"); start_clock = prag_start; }
	/* method 1. */

	featpyramid( img, model, pyra );
	//loadPyramid( "pyramid.txt", pyra );
	/* method 2. */
	//featpyramid2( img, model, pyra );	
	if(show_hints){	end_clock = yuGetCurrentTime('M');
		printf("_featpyramid takes %gs\n",(end_clock-start_clock)/1000.f);
		fprintf(stream, "\n_featpyramid takes %gs\n", (end_clock - start_clock) / 1000.f);
	}

	update_ruleData( model, pyra.num_levels );

	// 2. Filter responses & Deformation rules & Structural rules<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	if(show_hints){	printf("Applying rules ...\t"); start_clock = end_clock; }
	/* method 1. */
	//apply_rules( model, pyra );
	/* method 2. */
	/*apply_rules2( model, pyra );*/
	/* method 3. */
	apply_rules3( model, pyra );
	if(show_hints){	end_clock = yuGetCurrentTime('M');
		printf("_apply_rules takes %gs\n",(end_clock-start_clock)/1000.f);
		fprintf(stream, "_apply_rules takes %gs\n", (end_clock - start_clock) / 1000.f);
	}

	// 3. Parse detections <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	if( hints ){	/*printf("Parsing detections ...\t");*/ start_clock = end_clock; }
	detections = parse_detections( model, pyra, score_thresh );
	/*if( detections.empty() ) { printf("NO DETECTIONS!\n"); }*/
	if(show_hints){	end_clock = yuGetCurrentTime('M');
		printf("_parse_detections takes %gs\n",(end_clock-start_clock)/1000.f); 
		fprintf(stream, "_parse_detections takes %gs\n", (end_clock - start_clock) / 1000.f);
	}

	// 4. Show results <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	Elapsed_Time_ROI = 0;
	prag_end = yuGetCurrentTime('M');
	Elapsed_Time_ROI = (prag_end-prag_start)/1000.f;
	Elapsed_Time_One_Frame += Elapsed_Time_ROI;
	
}

