#include "../PM_type.h"

void	fconv_1( const Mat &A, const Mat &F, Mat &R );

void PM_type::fconv( const vector<Mat> &Feats, const Mat &Filter, vector<Mat> &Response, int lev1, int lev2, vector<INTS_2> &ScoreSz )
// The filter response of Filter on HOG feature map Feats
{
	Response.resize( Feats.size() );
	for( int i=lev1; i<lev2; i++ ){
		Response[i].create( ScoreSz[i][0], ScoreSz[i][1], CV_32FC1 );
		Response[i] = Scalar(-FLOAT_INF);
		fconv_1( Feats[i], Filter, Response[i] );
	}
}

void	fconv_1( const Mat &A, const Mat &F, Mat &R )
{
	int		RowA = A.rows, ColA = A.cols, NumFeatures = A.channels();
	int		RowF = F.rows, ColF = F.cols, ChnF = F.channels();
	if( NumFeatures!=ChnF )
		throw runtime_error("");

	int    RowR = RowA - RowF + 1, ColR = ColA - ColF + 1;

	float *Rpt = (float*)R.data;
	int Rstep = R.step1();

	for( int r=0; r!=RowR; r++ ){
		float *pt = Rpt + r*Rstep;
		for( int c=0; c!=ColR; c++ ){
			Mat	Asub = A( Rect(c,r,ColF,RowF) );
			*(pt++) = (float)( F.dot( Asub ) );
		}
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
