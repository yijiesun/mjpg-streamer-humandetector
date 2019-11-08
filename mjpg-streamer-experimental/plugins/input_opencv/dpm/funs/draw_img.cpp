#include "../FastDPM.h"
#include <stdio.h>

void  FastDPM::draw_once(Mat &img, float ElapsedTime, int frameCnt)
{
	const Scalar drawColor = CV_RGB(255, 0, 0);
	const Scalar fontColor = CV_RGB(0, 255, 0);
	const float fontScale = font_size;
	const int fontFace = CV_FONT_HERSHEY_PLAIN;
	char buf[100];

	//char buf3[4] = { '0' };
	//sprintf_s(buf3, 4, "%d", frameCnt);
	//string msgFrame(buf3);
	//Point frameOrig(0, 10);
	//putText(img, msgFrame, frameOrig, fontFace, fontScale, drawColor);

	if (ElapsedTime>0) {
		/*printf("Elapsed time for total program = %gs\n", ElapsedTime);*/
		memset(buf, 0, 100);
		snprintf(buf, sizeof(buf), "%gs", ElapsedTime);
		string msg(buf);
		int	 baseline = 0;
		Size textSize = getTextSize(msg, fontFace, fontScale, 1, &baseline);
		Point txtOrig(0, 50);
		putText(img, msg, txtOrig, fontFace, fontScale, fontColor);
	}
}
void  FastDPM::draw_img( Mat &img, vector<FLOATS_7> &detections, Scalar &drawColor,bool drawFont)
{
	char buf[100];
	const Scalar fontColor = CV_RGB(0,255,0);
	const float fontScale = font_size;
	const int fontFace = CV_FONT_HERSHEY_PLAIN;

	//printf(">>> %-9s %-9s %-9s %-9s %-5s %-9s %-9s\n","x1","y1","x2","y2","level","component","score");
	drawCnt = 0;
	for( int i=0; i<detections.size(); i++){
		float		x1 = detections[i][0], y1 = detections[i][1], x2 = detections[i][2], y2 = detections[i][3];
		float		level = detections[i][4];
		float		component = detections[i][5];		
		float		score = detections[i][6];
		//x1 = x1 / zoom_value + x0;
		//y1 = y1 / zoom_value + y0;
		//x2 = x2 / zoom_value + x0;
		//y2 = y2 / zoom_value + y0;

		if (x2 - x1 <= tooSmalltoDrop || y2 - y1 <= tooSmalltoDrop)
			drawColor = CV_RGB(0, 0, 255);
		
		Point2f		UL( x1, y1), BR( x2, y2);
		if (drawFont == 0)
			rectangle( img, UL, BR, drawColor, 4 );
		else
			rectangle(img, UL, BR, drawColor, 2);
		//printf("    %-9.3f %-9.3f %-9.3f %-9.3f %-5g %-9g %-9g\n", x1, y1, x2, y2, level, component,score );

		memset( buf, 0, 100 );
		snprintf( buf, sizeof(buf), "%-2d %-3.0f %-3.0f %-3.0f %-3.0f  %-2.0f %-2.0f %-6.3f", drawCnt,x1,y1,x2,y2,level,component,score );
		string msg(buf);
		int	 baseline = 0;
		Size textSize = getTextSize( msg, fontFace, fontScale, 1, &baseline );
		Point txtOrig( 50, (drawCnt +1)*(textSize.height+3) );
		if(drawFont)
			putText( img, msg, txtOrig, fontFace, fontScale, fontColor );

		char buf2[4] = { '0' };
		snprintf( buf2, 4, "%d", drawCnt);
		msg = buf2;
		Point   textOrg2( int(x1)+3, int(y1)+textSize.height+5 );
		if (drawFont)
			putText( img, msg, textOrg2, fontFace, fontScale, drawColor );
		drawCnt++;

	}
		
}
