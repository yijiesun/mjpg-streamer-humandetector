
# TODO: which components do I need?
# To fix the error: "undefined symbol: _ZN2cv12VideoCaptureC1Ev"
find_package(OpenCV COMPONENTS core imgproc highgui videoio)

MJPG_STREAMER_PLUGIN_OPTION(input_opencv "OpenCV input plugin"
                            ONLYIF OpenCV_FOUND)

if (PLUGIN_INPUT_OPENCV)
    enable_language(CXX)
    include_directories(${OpenCV_INCLUDE_DIRS})

    MJPG_STREAMER_PLUGIN_COMPILE(input_opencv input_opencv.cpp v4l_test.cpp dpm/FastDPM.cpp
	dpm/loadPyramid.cpp
	dpm/auxiliary/cvPrint.cpp
	dpm/auxiliary/getCurrentTime.cpp
	dpm/auxiliary/yuSaveMat.cpp
	dpm/funs/analyze_model.cpp
	dpm/funs/apply_rules.cpp
	dpm/funs/apply_rules2.cpp
	dpm/funs/apply_rules3.cpp
	dpm/funs/bboxpred_get.cpp
	dpm/funs/bboxpred_input.cpp
	dpm/funs/bounded_dt.cpp
	dpm/funs/bounded_dt2.cpp
	dpm/funs/clipboxes.cpp
	dpm/funs/config.cpp
	dpm/funs/cvPermutate.cpp
	dpm/funs/detections_trees2.cpp
	dpm/funs/draw_img.cpp
	dpm/funs/fconv.cpp
	dpm/funs/featpyramid.cpp
	dpm/funs/featpyramid2.cpp
	dpm/funs/features.cpp
	dpm/funs/features14_2.cpp
	dpm/funs/flipfeat.cpp
	dpm/funs/get_score_size.cpp
	dpm/funs/load_model.cpp
	dpm/funs/loc_feat.cpp
	dpm/funs/model_get_block.cpp
	dpm/funs/nms.cpp
	dpm/funs/parse_detections.cpp
	dpm/funs/pDollar_nms.cpp
	dpm/funs/predict_box.cpp
	dpm/funs/reduceboxes.cpp
	dpm/funs/sum_score.cpp
	dpm/funs/yuInd2Sub.cpp)
    
    target_link_libraries(input_opencv ${OpenCV_LIBS})

    
endif()


