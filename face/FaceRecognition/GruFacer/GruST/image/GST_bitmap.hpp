#pragma once
/*
	1��λͼ�ӿڣ�Ӧ����������
	2���������漰�κε���ͼ�ο�
*/
#include <vector>
#include <string>
#include <stdint.h>
#include "../GST_def.h"
#include "../util/GST_datashape.hpp"
#include "../basic/GST_fno.hpp"
#ifdef WIN32		//BMP_DRAW�������
	#include <windows.h>
#endif

#define RGB2GRAY(r,g,b) (((b)*117 + (g)*601 + (r)*306) >> 10)
//���ɫ��ͨ��
typedef BIT_32 CCCx;
#define CCC3(r,g,b)   ( (CCCx)(((BIT_8)(r)|((CCCx)((BIT_8)(g))<<8))|(((CCCx)(BIT_8)(b))<<16)) )
#define CCC_1(rgb)      (BIT_8 (rgb) )
#define CCC_2(rgb)      (BIT_8 (((CCCx)(rgb)) >> 8) )
#define CCC_3(rgb)      (BIT_8 ((CCCx)(rgb)>>16) )

namespace Grusoft{

typedef void* BMP_HANDLE; 
//On FreeImage:	
class GST_BMP{
	
public:
	typedef enum{
		GRAY,R,G,B,L,U,V,G_NORMAL
	}CHANEL;

	struct INITIAL{
		int wMax,hMax,bpp,wth_0,hei_0;
		double sW,sH;		//zoom scale for wth and height
		INITIAL( ) : wMax(-1),hMax(-1),sW(1.0),sH(1.0){;}
		INITIAL( int wM,int hM) : wMax(wM),hMax(hM),sW(1.0),sH(1.0){;}
	};
	INITIAL init;

	class ColorSpace{
		public:
			typedef enum{
				CF_0=0,RGB,LUV,
			}FORMAT;		
		private:
			FORMAT fomat;
			BIT_8 *ccc;	//sizeȡ����si,��һ��map����gray(���庬�����ʽ���)
			GST_BMP *hBmp;
			void ToRGB( int flag=0x0 );
			void ToLUV( int flag=0x0 );
		protected:
			ColorSpace( GST_BMP *bmp):hBmp(bmp),fomat(CF_0),ccc(nullptr)	{	
				GST_VERIFY(bmp!=nullptr,"ColorSpace bmp is 0" );
			}
			virtual ~ColorSpace( ){	
				if( ccc!=nullptr )	delete[] ccc;	
			}
			bool To( FORMAT format,int flag );
			BIT_8 *Channel( CHANEL chan,int flag=0 );
		friend class GST_BMP;
	};
	
	typedef enum{
		HISTO_EQUAL=1,
		CONTRAST=2,
	}NORMAL;
protected:
	typedef enum{
		NONE=0x0,
		//T_GRAY,
		T_GRADIENT,
	}TRANS;
	struct STATE{
		vector<TRANS> trans;
		int code;
		bool isOK( )	{	return true;	}
		bool isTransed( TRANS t1,... )	{	return isOK();	}
	};
	STATE state;
	ColorSpace space;
	BMP_HANDLE hib;
	SHAPE_IMAGE si;
	//void InitCCC( int flag=0x0 );
	GST_BMP* HistoEqual( BIT_8 *gray,INT_32 flag=0x0 );	
	GST_BMP* Contrast( BIT_8 *gray,INT_32 flag=0x0 );	
	GST_BMP* Illumi( BIT_8 *cc,INT_32 flag=0x0 );
	GST_BMP* Guide( BIT_8 *gray,INT_32 bo,float epsi,float *S_TEMP,INT_32 flag=0x0 );
public:
	static std::string sDumpFolder;
	int no,nColors;
	string desc;
	typedef enum{
		QUAD=1,
		C_RGB=10
	}PIXEL_INFO;

	GST_BMP( const string &sPath,INITIAL init=INITIAL(),int flag=0x0 );
	GST_BMP( const wstring &sPath,INITIAL init=INITIAL(),int flag=0x0 );
	GST_BMP( GST_BMP *hSrc,int trans=0x0,RoCLS* sub=NULL,int flag=0x0 );		//sub always in hSrc
	GST_BMP( SHAPE_IMAGE *si,int type=0x0,int flag=0x0 );
	GST_BMP( BIT_8 *gray,SHAPE_IMAGE *sin,int flag=0x0 );
	virtual ~GST_BMP( );
	virtual bool Save( const string &sPath,int flag=0x0 );
	virtual bool Load( const string &sPath,int flag=0x0 );
	//static bool SaveF( const string &sPath,float *data,SHAPE_IMAGE si,int flag=0x0 );

	virtual INT_32 Width( int flag=0 )		{	return si.wth;	}
	virtual INT_32 Height( int flag=0 )		{	return si.hei;	}
	virtual INT_32 nPixel( int flag=0 )		{	return si.hei*si.wth;	}
	template<typename T>
	void Channel_( CHANEL chan,T *target,int flag=0 ){
		int ld=si.Pixel();
		BIT_8 *src=space.Channel(chan,flag );		
		GST_VERIFY( target!=nullptr && src!=nullptr,"GST_BMP::Channel is ..." );	
		for( int i=0;i<ld;i++ )		target[i]=src[i];
	}

	BIT_8 *Gray( )	{ return space.Channel(GST_BMP::GRAY);	 }	//inline Gray Channel

	void* GetHBitmap( )			{	return hib;	}
	SHAPE_IMAGE* spIMAGE( )		{	return &si;}

	BIT_8 GetPixelGray( unsigned int c, unsigned int r );
	bool GetPixelColor( unsigned int c, unsigned int r, BIT_8 *cr,BIT_8 *cg,BIT_8 *cb,PIXEL_INFO type=C_RGB );
	bool GetPixelX( unsigned int c, unsigned int r,void *,PIXEL_INFO type=QUAD );
	//bool SetPixelColor( unsigned int c, unsigned int r, void *value);
	virtual bool SetPixelColor( unsigned int c, unsigned int r, BIT_8 cr,BIT_8 cg,BIT_8 cb,void *value=nullptr );
	virtual bool SetPixelColor( int c, int r, BIT_8 cr,BIT_8 cg,BIT_8 cb,void *value=nullptr ){
		unsigned int ic=(unsigned int)(c),ir=(unsigned int)(r);
		return SetPixelColor( ic,ir,cr,cg,cb,value );
	}
	virtual bool SetPixelColor( double c, double r, BIT_8 cr,BIT_8 cg,BIT_8 cb,void *value=nullptr ){
		unsigned int ic=(unsigned int)(c+0.5),ir=(unsigned int)(r+0.5);
		return SetPixelColor( ic,ir,cr,cg,cb,value );
	}
	virtual bool SetPixelColor( float c, float r, BIT_8 cr,BIT_8 cg,BIT_8 cb,void *value=nullptr ){
		unsigned int ic=(unsigned int)(c+0.5f),ir=(unsigned int)(r+0.5f);
		return SetPixelColor( ic,ir,cr,cg,cb,value );
	}
	template<typename T> GST_BMP* Set( T *data,int nMap,int flag=0x0 ){
		//assert( nMap==1 );
		unsigned int r,c,nz=0;
		BIT_8 cr,cg,cb;
		T *d1=data;
		for( r=0; r<si.hei;r++ ){
		for( c=0; c<si.wth;c++ ){
			if( nMap==1 ){
				cr = data[nz++];		cg=cb=cr;
			}
			SetPixelColor( c,r,cr,cg,cb );
		}
		}
		return this;
	}

	GST_BMP* FlipHorizontal( );
	GST_BMP* FlipVertical( );

	GST_BMP *ToColorSpace( ColorSpace::FORMAT format,int flag=0x0 )	{	space.To(format,flag);	return this;	}
	GST_BMP *Median( BIT_8 *gray=nullptr,int radius=1,int flag=0x0 );
	GST_BMP *Normalise( NORMAL alg,int chan=-1,int flag=0x0 );

	virtual bool ToSize( int wth,int hei,bool isKeepRatio=true,int flag=0x0 );
	virtual bool Rescale( double sW,double sH,int flag=0x0 );
	virtual void SetModified( int flag=0x0 );		//ͼƬ���������޸�(�����ŵ�)

};

struct STRUC_ELE{
	typedef enum {
		NU,RECTANGLE,CIRCLE,
		UNCHANGED=-100,
	}TYPE;
	INT_32 W_0,W_1,H_0,H_1,ldw,M_nz;
	TYPE type;
	BIT_8 *mask;

	STRUC_ELE( ) : mask(nullptr)	{	}
	virtual ~STRUC_ELE( ){
		if( mask!=nullptr )	delete[] mask;
	}
};
struct SE_CIRCLE : public STRUC_ELE{
	SE_CIRCLE( INT_32 W_1,INT_32 H_1,INT_32 flag=0x0 );
};


//image processing
class BMPP : public GST_BMP{
protected:
	float *g_I,*g_rad,g_I_0,g_I_1;
	BIT_8 gray_0,gray_1;//*gray;
	int Init( int flag );

public:	
	union{
		float weight;
		float confi;
	};
	BIT_8 *mask,*tmp8;
	typedef enum{
		MASK_3_3=0x100,					//3*3 operator
		PREWITT_R=0x100,PREWITT_C,SOBEL_R,SOBEL_C,SOBEL_45,SOBEL_135,MEAN,
		MASK_5_5=0x200,
		FIVE_R=MASK_5_5,FIVE_C,SPOT_R,SPOT_C,
	}MASK;
	typedef enum {
		ADJACENT_4=0,ADJACENT_8,ADJACENT_M
	}ADJACENT;
	typedef enum{
		GRAD_INTENS=0x100,GRAD_ANGLE,	
		MASK_0,MASK_1,MASK_2,
	}INFO_TYPE;
	typedef enum{
		MSK_RED_GRN=1,MSK_BLACK,MSK_BORDER,						
	}BMP_OUTPUT;

	BMPP( SHAPE_IMAGE *si,int type=0x0,int flag=0x0 ): GST_BMP(si,type,flag),g_I(nullptr),g_rad(nullptr),mask(nullptr)
	{		Init(flag);		}
	//roi always sub-area in hSrc
	BMPP( BMPP *hSrc,int trans=0x0,RoCLS* sub=NULL,int flag=0x0 ): GST_BMP(hSrc,trans,sub,flag),g_I(nullptr),g_rad(nullptr),mask(nullptr)
	{		Init(flag);			}
	BMPP( const string &sPath,INITIAL init=INITIAL(),int flag=0x0 ) : GST_BMP(sPath,init,flag),g_I(nullptr),g_rad(nullptr),mask(nullptr)
	{		Init(flag);		}
	BMPP( const wstring &sPath,INITIAL init=INITIAL(),int flag=0x0 ) : GST_BMP(sPath,init,flag),g_I(nullptr),g_rad(nullptr),mask(nullptr)
	{		Init(flag);		}
	BMPP( BIT_8 *gray,SHAPE_IMAGE *si,int flag=0x0 ) : GST_BMP(gray,si,flag),g_I(nullptr),g_rad(nullptr),mask(nullptr)
	{		Init(flag);		}
	virtual ~BMPP( );
	virtual BMPP* Shrink( double sW,double sH,int flag=0x0 );		//��Ϣ����ʧ!
	virtual BMPP* SaveMask( const string &sPath,BIT_8 *ground,BMP_OUTPUT type,int flag=0x0 );

	BMPP* GetInfo( INFO_TYPE type,void*pinfo, void **ppinfo=nullptr,int flag=0x0 );
	//BMPP* Gray( int flag=0x0 );
	BMPP* Gradient( int flag=0x0 );
	BMPP* EdgeCanny( float I_1,float I_0,float T_g0,float T_g1,int flag=0x0 );

	virtual	BMPP* Inner_( BIT_8 *ground,BIT_8 *tmp,INT_32 loop=1,INT_32 flag=0x0 );
	virtual	BMPP* Erose_( STRUC_ELE *hSE,BIT_8 *ground,BIT_8 *tmp,INT_32 loop=1,INT_32 flag=0x0 );
	virtual BMPP* Dilate_( STRUC_ELE *hSE,BIT_8 *ground,BIT_8 *tmp,INT_32 loop=1,INT_32 flag=0x0 );
	virtual BMPP* Open_( STRUC_ELE *hSE,BIT_8 *ground,BIT_8 *tmp,INT_32 loop,INT_32 flag=0x0 );
	virtual BMPP* Close_( STRUC_ELE *hSE,BIT_8 *ground,BIT_8 *tmp,INT_32 loop,INT_32 flag=0x0 );

	virtual BMPP* MASK_Polygon( const POLYGON&poly,RoCLS &roi,BIT_8 b8,bool isFill=true,int flag=0x0 );
	virtual BMPP* TagBorder( BIT_8 *tag,BIT_8 *border,INT_32 flag=0x0 );
};
typedef BMPP* hBMPP;

/*Bitmap drawing*/
struct BMP_DRAW : public GST_BMP {
#ifdef WIN32
	BITMAPINFO *bmi,*pbase;
	BYTE *bits;
	HDC pDC,tmpDC;
	HBITMAP tmpBmp;
	HFONT hFont;
#endif
protected:
	void CrateFont( int cH=12, int cW=0 );

public:	
	BMP_DRAW( GST_BMP *hSrc,RoCLS*sub=NULL,int type=0x0,int flag=0x0 );
	BMP_DRAW( SHAPE_IMAGE *si,int type=0x0,int flag=0x0 );
	virtual ~BMP_DRAW( );
	virtual void ClearBack( int flag=0x0 );
	virtual bool SetPixelColor( unsigned int c, unsigned int r, BIT_8 cr,BIT_8 cg,BIT_8 cb,void *value=nullptr );
	virtual void BITBlt( const GST_BMP *hbmp,const RoCLS &box,int flag=0x0 );
	virtual void Text( int x, int y, const char* text, CCCx color=0,int flag=0x0 );
	virtual void Line( int32_t StartX, int32_t EndX, int32_t StartY, int32_t EndY, CCCx color=0,int flag=0x0);
	virtual void Rect( int32_t StartX, int32_t EndX, int32_t StartY, int32_t EndY, CCCx color=0,int flag=0x0);
	virtual bool Save( const char*sPath,bool isX=true,int flag=0x0 );
	virtual bool Flush( );		//cancas->bitmap

	bool SaveText( const wchar_t* sPath,const wchar_t *sText, CCCx color=0,int flag=0x0 );

	static bool Plot_N( string sPath,vector<GST_BMP*>&bmps,SHAPE_IMAGE si,int ld,int flag=0x0 );
;
};
typedef BMP_DRAW *hBMPD;

class Hough	{
public:
	Hough( hBMPP hBMPP,int flag=0x0 ){;}
	virtual ~Hough( )	{;}
};
class Hough_Parabola : public Hough	{
public:
	Hough_Parabola( hBMPP hBMPP,int flag=0x0 );
	virtual ~Hough_Parabola( )	{;}
};

class PICS_N{
protected:
//	void Trans_Bach( float * pdata,int flag );
	int SaveF( const char *sPath,float * pdata,int flag );
public:
	typedef enum{
		NONE,
	}TRANS;
	TRANS trans;
	vector<GST_BMP*> nails;

//	void *hDIB;		//pointer to dib
	int wth,hei,maps;
	static int gridX;
	static void ON( int x,int flag=0x0 ){	gridX=x;	}
	static void OFF( ){	gridX=-1;}
	BMP_DRAW *hBMP;
	int nBmp,nWTH,nHEI,magi_w,magi_h,i_1,j_1;
	std::vector<std::string> arrInfo;
	std::string sTitle;

	PICS_N( int nBmp,int wth, int hei,int map,int x,int margi_,int flag=0 );
	PICS_N( ):hBMP(nullptr),nBmp(0),wth(-1),hei(-1),magi_w(-1),magi_h(-1){;}	
	virtual ~PICS_N( );	
//	void CCC_P( BIT_8 cr,BIT_8 cg,BIT_8 cb,int r,int c,int flag=0x0 );	
//	void Gray_P( BIT_8 gray,int r,int c,int flag=0x0 );
//	virtual void Save( const char *sPath,int flag );	
	bool PlotNails( string sPath,SHAPE_IMAGE si,int ld,bool isClear=false,int flag=0x0 );
	void SetText( int no,const char *format,... );

	template <typename T>
	int SaveT( const char *sPath,T * pdat_,TRANS tr_=NONE,int flag=0x0 )	{	
		float *temp=nullptr,*pdata=(float *)pdat_;
		int i,dim=nBmp*wth*hei*maps,iRet=-1;
		if( sizeof(T)==4 ){
		}else if( sizeof(T)==1 ) {
			temp = new float[dim];
			for( i=0;i<dim; i++)	temp[i]=pdat_[i];
			pdata=temp;
		}else{
			throw "PICS_N::SaveT T is XXX";
		}		
		switch( trans ){
		default:
			break;
		}
		iRet=SaveF( sPath,(float*)pdata,flag );	
		if( temp!=nullptr )		delete[] temp;
		return iRet;
	}
};
}







