#pragma once

#include "GruST/image/GST_bitmap.hpp"
#include "GruST/basic/GST_fno.hpp"

#define _FACE_MARK_0_		68
#define _FACE_MARK_			51

#define _BIT_MOVE_
#ifdef  _BIT_MOVE_
	typedef signed char T_MOVE;
#else
	typedef float T_MOVE;
#endif

//for NDK
typedef float* NDKvFACE;	
#define NDKvF_ld			(_FACE_MARK_*2+10)
#define NDKvF_interoc(vF)	(vF[_FACE_MARK_*2])
#define NDKvF_r0(vF)	(vF[_FACE_MARK_*2+1])
#define NDKvF_c0(vF)	(vF[_FACE_MARK_*2+2])
#define NDKvF_wth(vF)	(vF[_FACE_MARK_*2+3])
#define NDKvF_hei(vF)	(vF[_FACE_MARK_*2+4])
namespace Grusoft{
	class Makeup	{
	public:
		typedef arrFNO PIXELS;
		typedef enum{
			L_EYE,R_EYE,NOISE,MOUTH
		}TYPE;

	protected:
		int type;		
		int confi;		//The confidence score from 0 to 100 of the image coordinates.
		RoCLS roi;
		hBMPP organ,shadow;
		//void PolyMask( void *hX,int nPoly,int (*polys)[4],COLORREF crFore,int flag ); 
	public:
		static int nFrame;
		static bool isSaveFrame;
		static int ClsColor( int cls,int flag=0x0 );
		enum{
			LEFT=0x10000,RIGHT=0x20000
		};
		bool isRefined;
		bool isType( int flag )	{	return BIT_TEST(type,flag);	}
		//poly���Ǳպ�
        Makeup( int wth,int hei,const POLYGON&poly,BIT_8*mask,int nDialte,int flag=0x0 );
		Makeup( const hBMPP hSrc,const POLYGON &poly,PIXELS &pixs,TYPE tp,int flag=0x0 );
		virtual ~Makeup( ){
			//pixs.clear( );
			if(organ!=nullptr)	delete organ;
			if(shadow!=nullptr)	delete shadow;
			//if(alpha!=nullptr)	delete[] alpha;
		}
		
	/*	void* GetHBitmap( )	{	
			if(organ==nullptr)	
				throw "GetHBitmap is XXX";	
			return organ->GetHBitmap( ); 
		}*/
		virtual bool isConfi( )	{
			return confi>30;		
		}
		virtual bool isOK( )	{
			return organ!=nullptr;		
		}
	};
	typedef vector<Makeup*> arrMakeup;

	template<typename T>
	bool File2Array( const char *sPath,size_t *nnz,T **arr,int flag=0x0 ){
		try{
			*nnz=-1;
			FILE *fpD=fopen( sPath,"rb" );
			if( fpD==nullptr )		GST_THROW( "FaceAligner::File2Array: fpD is 0" );
			fseek( fpD,0,SEEK_END );
			int nz=ftell(fpD)/sizeof(T);
			*arr=new T[nz];
			fseek(fpD,0,SEEK_SET );
			if( fread( *arr,sizeof(T),nz,fpD)!=nz )
			{	delete[] *arr;	return false;	}
			fclose( fpD );
			*nnz=nz;
			printf( "\n********* ToArray: %s=>%d x %d\n",sPath,nz,sizeof(T) );
			return true;
		}catch(...){
			GST_THROW( "FaceAligner::File2Array is X" );
//	return false;
		}
	}

	class FaceAligner	{

	SHAPE_IMAGE siFace;
	float *meanFace,*lastFace;

	int *trees,nCas;
	T_MOVE *moves;
	size_t nnzMove,nnzTree;
protected:
	bool LoadMeanFace( const char *sPath,int flag );
public:
    static int face_Blink[8],face_lE[7],face_rE[7];
    static int face_58pts[12],face_all[12],face_half[12];
    static FaceAligner* Instance( );

	NDKvFACE face000;
	int nMorph;
	int becomNO,becomWth,becomHei;
	BIT_8* becomGray;
    double maxBlink;

	Makeup::PIXELS pixs;		//make-up pixels in the face
	static SHAPE_PtSet spFace0,spFace;
	static int wMax,hMax;
	struct PARAM{
		string pMeanFace,pRF_D,pRF_T;
		PARAM( );
	};
	PARAM param;
	RoCLS roi,roiLast;

	typedef enum{
		OK=0x0,
		NO_FACE=-1,
	}CODE;

	FaceAligner( const char *pD,const char *pT,int flag );
	FaceAligner( int flag=0x0 );
	~FaceAligner( ){
		SetBecome( -1,nullptr,-1,-1 );
		if( meanFace!=nullptr )		delete[] meanFace;
		if( lastFace!=nullptr )		delete[] lastFace;
		if( moves!=nullptr )		delete[] moves;
		if( trees!=nullptr )		delete[] trees;
	}
    void SetBecome( int nTo,BIT_8*gray,int wth,int hei,int flag=0x0 );
	void SetData( size_t nzM,T_MOVE*mov,size_t nzT,int *tre,int flag=0x0 );
	bool Detect_( GST_BMP *hSrc,BIT_8 *gray,int wth,int hei,int mode,int flag=0x0 );

	CODE Align( GST_BMP *hbmp,float*shape,int flag=0x0 );
	CODE Align( BIT_8 *gray,int wth,int hei,float*shape,int flag=0x0 );

	bool isStatic( BIT_8 *gray,int wth,int hei,float*shape,int flag=0x0 );
	int CopyMeanFace( float **mf,int flag=0x0 );
#ifdef  _FACE_MORPH_

    void Morph( const hBMPP hbmp,float*src,int flag=0x0 );
    int Morph( BIT_8 *gray,int wth,int hei,float*src,int *nos,int flag );
#endif
};


}
void FaceMorph_( Grusoft::FaceAligner *hAlign,float *face,BIT_8* frame,BIT_8* fU,BIT_8* fV,int fWth,int fHei,int *morphs,int *info,int flag );
int NDK_FaceAlign_( BIT_8 *gray,BIT_8 *fU,BIT_8 *fV,float *fMark,int *info,int wth,int hei,int *morphs,int flag );
#ifdef __cplusplus
extern "C" {
#endif
	int FaceAlign_init( size_t nzM,T_MOVE*mov,size_t nzT,int *tre,size_t nzFace,float *face000,int flag );
	int FaceAlign_clear( );
    void FaceAlign_ToMark0_( float *marks,int flag );
    
	int NDK_FaceBecome_( int nTo,int *pixels,int wth,int hei,int *info,int flag );
	//int NDK_FaceMorph_( int *pixels,int wth,int hei,int nTo,int *info,int flag );
#ifdef __cplusplus
}
#endif
