#include "../FastDPM.h"

void FastDPM::togetherAllBox(bool is_video)
{
	for (int i = 0; i<detections.size(); i++) {
		float		x1 = detections[i][0], y1 = detections[i][1], x2 = detections[i][2], y2 = detections[i][3];
		float		level = detections[i][4];
		float		component = detections[i][5];
		float		score = detections[i][6];
		if (is_video)
		{
			detections[i][0] = x1 / zoom_value + x0;
			detections[i][1] = y1 / zoom_value + y0;
			detections[i][2] = x2 / zoom_value + x0;
			detections[i][3] = y2 / zoom_value + y0;
		}
		detections_full_current.push_back(detections[i]);

	}
}

float FastDPM::calcDis(FLOATS_7 &last, FLOATS_7&curr)
{
	float x1 = last[0] + (last[2] - last[0]) / 2.0;
	float y1 = last[1] + (last[3] - last[1]) / 2.0;
	float x2 = curr[0] + (curr[2] - curr[0]) / 2.0;
	float y2 = curr[1] + (curr[3] - curr[1]) / 2.0;
	float tmp = (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1);
	tmp = sqrt(tmp);
	return tmp;
}

void FastDPM::use_2_box_calc_vec()
{
	vector<VEC_BOX>      P_T;
	vector<FLOATS_7>     A;//last single box
	vector<FLOATS_7>     B;//current single box
	find_single_box(detections_full_last, A);
	find_single_box(detections_full_current, B);
	//AB配对的，或者只有A中有的单点向量集
	for (int i = 0; i < A.size(); i++) {
		float dis = 0;
		for (int j = 0; j < B.size(); j++) {
			dis = calcDis(A[i], B[j]);
			if (dis <= box_track_distance)
			{
				VEC_BOX tmp;
				tmp.A = A[i];
				tmp.B = B[j];
				tmp.C = 0;
				P_T.push_back(tmp);
				break;
			}
		}
		if (dis > box_track_distance)
		{
			VEC_BOX tmp;
			tmp.A = A[i];
			tmp.B = A[i];
			tmp.C = 1;
			P_T.push_back(tmp);
		}
	}
	//AB不配对，且只有B中有的单点向量集
	for (int i = 0; i < B.size(); i++) {
		float dis = 0;
		for (int j = 0; j < A.size(); j++) {
			dis = calcDis(A[i], B[j]);
		}
		if (dis > box_track_distance)
		{
			VEC_BOX tmp;
			tmp.A = B[i];
			tmp.B = B[i];
			tmp.C = 0;
			P_T.push_back(tmp);
		}
	}
	find_single_box_vec(P_T, P);

}
void FastDPM::use_vec_box_calc_vec()
{
	vector<FLOATS_7>     B;//last single box
	find_single_box(detections_full_current, B);
	cout << "\n[" << frame_cnt << "] " << "detections_full_current " << detections_full_current.size() << "  B " << B.size() << endl;
	fprintf(stream, "\n[%d] detections_full_current %d  single box B %d\n", frame_cnt, detections_full_current.size(), B.size());
	//用detections_full_current和P来更新P
	//当P是空的，B有值
	if (P.size() == 0 && B.size() != 0)
	{
		for (int j = 0; j < B.size(); j++) {
			VEC_BOX tmp;
			tmp.A = B[j];
			tmp.B = B[j];
			tmp.C = 0;
			tmp.D = 0;
			P.push_back(tmp);
		}
	}
	else
	{
		//B是当前box，P是预测box
		//BP配对的，或者只有P中有的单点向量集
		for (int i = 0; i < P.size(); i++) {
			float dis = 0;
			for (int j = 0; j < B.size(); j++) {
				dis = calcDis(P[i].B, B[j]);
				if (dis <= box_track_distance)
				{
					P[i].A = P[i].B;
					P[i].B = B[j];
					P[i].C = 0;
					P[i].D = dis;
					break;
				}
			}
			if (dis > box_track_distance)
			{
				P[i].C++;
			}
		}
		//BP不配对，且只有B中有单点向量集
		for (int i = 0; i < B.size(); i++) {
			float dis = 0;
			bool noQuat = true;
			for (int j = 0; j < P.size(); j++) {
				dis = calcDis(P[j].B, B[i]);
				if (dis <= box_track_distance)
					noQuat = false;
			}
			if (noQuat)
			{
				VEC_BOX tmp;
				tmp.A = B[i];
				tmp.B = B[i];
				tmp.C = 5;
				tmp.D = 0;
				P.push_back(tmp);
			}
		}

		//BP不配对，且B为空
		if (P.size() != 0 && B.size() == 0)
		{
			for (int j = 0; j < P.size(); j++) {
				P[j].C++;
			}
		}
	}

	vector<VEC_BOX>      P_T;
	P_T.assign(P.begin(), P.end());
	P.clear();
	find_single_box_vec(P_T, P);
	//cout << "[" << frame_cnt << "] " << "P_T " << P_T.size() << "  P " << P.size() << endl;
	//fprintf(stream, "[%d] 填入矢量集P_T %d  矢量集中single box P 数量 %d\n", frame_cnt, P_T.size(), P.size());
}
void FastDPM::find_single_box(vector<FLOATS_7> &L, vector<FLOATS_7> &A)
{
	for (int i = 0; i < L.size(); i++) {
		bool isSingleBox = true;
		for (int j = 0; j < L.size(); j++) {
			if (i == j)
				continue;

			float dis = calcDis(L[i], L[j]);
			if (dis <= box_track_distance)
			{
				isSingleBox = false;
				break;
			}
		}
		if (isSingleBox)
			A.push_back(L[i]);
	}

}

void FastDPM::find_single_box_vec(vector<VEC_BOX> &L, vector<VEC_BOX> &A)
{
	for (int i = 0; i < L.size(); i++) {
		bool isSingleBox = true;
		for (int j = 0; j < L.size(); j++) {
			if (i == j)
				continue;
			float dis = calcDis(L[i].B, L[j].B);
			if (dis <= box_track_distance)
			{
				isSingleBox = false;
				break;
			}
		}
		if (isSingleBox)
			A.push_back(L[i]);
	}

}

void FastDPM::predict_box_by_vec()
{
	detections_full_predict.clear();
	for (int i = 0; i < P.size(); i++) {
		if (0< P[i].C && P[i].C<predict_dis)
		{
			float x1 = P[i].B[0] + (P[i].B[2] - P[i].B[0]) / 2.0;
			float y1 = P[i].B[1] + (P[i].B[3] - P[i].B[1]) / 2.0;
			float x2 = P[i].A[0] + (P[i].A[2] - P[i].A[0]) / 2.0;
			float y2 = P[i].A[1] + (P[i].A[3] - P[i].A[1]) / 2.0;

			float x_center = x1 + x1 - x2;
			float y_center = y1 + y1 - y2;

			float wid = (P[i].B[2] - P[i].B[0] + P[i].A[2] - P[i].A[0]) / 2.0;
			float hgt = (P[i].B[3] - P[i].B[1] + P[i].A[3] - P[i].A[1]) / 2.0;

			FLOATS_7 tmp;
			tmp[0] = x_center - wid / 2.0;
			tmp[1] = y_center - hgt / 2.0;
			tmp[2] = x_center + wid / 2.0;
			tmp[3] = y_center + hgt / 2.0;
			tmp[4] = P[i].B[4];
			tmp[5] = P[i].B[5];
			tmp[6] = P[i].B[6];
			detections_full_predict.push_back(tmp);

			P[i].A = P[i].B;
			P[i].B = tmp;

		}
		cout << "[" << frame_cnt << "] " << "P[" << i << "].A " << P[i].A[0] << "," << P[i].A[1] << "," << P[i].A[2] << "," << P[i].A[3] << "," << P[i].A[4] << ",  C:" << P[i].C << endl;
		cout << "[" << frame_cnt << "] " << "P[" << i << "].B " << P[i].B[0] << "," << P[i].B[1] << "," << P[i].B[2] << "," << P[i].B[3] << "," << P[i].B[4] << ",  D:" << P[i].D << endl;
		fprintf(stream, "[%d] P[%d].A %f, %f, %f, %f, %f	C: %d\n", frame_cnt, i, P[i].A[0], P[i].A[1], P[i].A[2], P[i].A[3], P[i].A[4], P[i].C);
		fprintf(stream, "[%d] P[%d].B %f, %f, %f, %f, %f	D: %f\n", frame_cnt, i, P[i].B[0], P[i].B[1], P[i].B[2], P[i].B[3], P[i].B[4], P[i].D);
	}
	cout << "[" << frame_cnt << "] " << "predict " << detections_full_predict.size() << endl;
	fprintf(stream, "[%d] predict %d\n", frame_cnt, detections_full_predict.size());
	for (int i = 0; i < detections_full_predict.size(); i++) {
		cout << "[" << frame_cnt << "] " << "predict[" << i << "] " << detections_full_predict[i][0] << "," << detections_full_predict[i][1] <<
			"," << detections_full_predict[i][2] << "," << detections_full_predict[i][3] << "," << detections_full_predict[i][4] << endl;
		fprintf(stream, "[%d] predict[%d] %f, %f, %f, %f, %f\n", frame_cnt, i, detections_full_predict[i][0], detections_full_predict[i][1],
			detections_full_predict[i][2], detections_full_predict[i][3], detections_full_predict[i][4]);
	}
}

void FastDPM::clear_bad_vec()
{
	vector<VEC_BOX>      P_T;
	P_T.assign(P.begin(), P.end());
	P.clear();

	for (int i = 0; i < P_T.size(); i++) {
		if (P_T[i].C <= predict_dis)
			P.push_back(P_T[i]);

	}
	cout << "[" << frame_cnt << "] " << "P " << P.size() << " after clear" << endl;
	fprintf(stream, "[%d] P after clear %d\n", frame_cnt, P.size());
}

void FastDPM::clear_bad_box()
{
	vector<FLOATS_7>      P_T;
	P_T.assign(detections_full_current.begin(), detections_full_current.end());
	detections_full_current.clear();
	for (int i = 0; i < P_T.size(); i++) {
		float		x1 = P_T[i][0], y1 = P_T[i][1], x2 = P_T[i][2], y2 = P_T[i][3];

		if (x2 - x1 <= tooSmalltoDrop || y2 - y1 <= tooSmalltoDrop)
			continue;

		detections_full_current.push_back(P_T[i]);

	}
}