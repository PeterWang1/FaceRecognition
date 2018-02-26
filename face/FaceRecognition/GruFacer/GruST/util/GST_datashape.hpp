#pragma once

#include <memory>
#include <string>
#include "../GST_def.h"
#include "../basic/GST_fno.hpp"
#include "Serial.hpp"
using namespace std;

typedef enum{
	MNWH,NMWH
}TENSOR_FORMAT;

class DATA_SHAPE;
typedef shared_ptr<DATA_SHAPE> hSHAPE;
template<typename T>
T* TO(const hSHAPE hS ) 	{
	T* ts=dynamic_cast<T*>(hS.get( ) );
	if( ts==nullptr )
		throw "TO hSHAPE hS is XXX";	//bad_cast("TO hSHAPE"); 
	return ts; 
}

//��ݵ�һЩͨ��
typedef enum {
	GST_PIXEL_3=0x100,
}GST_DATA_MAPs;

class DATA_SHAPE{
public:
	unsigned int x;
//In general the leading dimension lda is equal to the number of elements in the major dimension. It is also
//equal to the distance in elements between two neighboring elements in a line in the minor dimension.
	typedef enum{
		UNKNOWN,SAMPLE_MAJOR=100,MAP_MAJOR=110,
		NMWH=200,	MNWH,
	}LAYOUT;
	LAYOUT layout;
	DATA_SHAPE( )
	{	x=0;	layout=UNKNOWN;	}
//elements in tensor	(all maps,all frames...)
	virtual void Init( const string &sParam )	{	throw "DATA_SHAPE::Init( const string &sParam ) unimplemented!!!";	}
	virtual unsigned int Count( )	const					
	{	throw "DATA_SHAPE::Count unimplemented!!!";	}
};

//�㼯
class SHAPE_PtSet : public DATA_SHAPE {
public:
	unsigned int nPt,dim;	//dim=2,3,...

	SHAPE_PtSet( ):DATA_SHAPE( ),nPt(0),dim(0)	{}
	SHAPE_PtSet( int n,int d,LAYOUT l=UNKNOWN ):DATA_SHAPE( ),nPt(n),dim(d)		{	layout=l;	/*GST_VERIFY(n>0 && d>0,"SHAPE_PtSet is X" );*/	}
	SHAPE_PtSet( const SHAPE_PtSet &im ):nPt(im.nPt),dim(im.dim)				{	layout=im.layout;		x=im.x;	}
	SHAPE_PtSet& operator=(const SHAPE_PtSet& si )	
	{	nPt=si.nPt,dim=si.dim,	layout=si.layout;	x=si.x;		return *this;	}
	//virtual void Init( int w,int h,int c=1,LAYOUT l=UNKNOWN )			{	nPt=w,		dim=h,		layout=l;	}
	virtual unsigned int Count( )	const		{	return nPt*dim;	}
};
//typedef SHAPE_PtSet* hSHAPt;

class SHAPE_IMAGE : public DATA_SHAPE {
public:
	unsigned int wth,hei;
	union{
		unsigned int chanel;
		unsigned int maps;
	};
	unsigned int bpp;
//	unsigned int x;
	SHAPE_IMAGE( ):DATA_SHAPE( ),wth(0),hei(0),chanel(0)	{}
	SHAPE_IMAGE( int w,int h,int c=1,LAYOUT l=UNKNOWN ):DATA_SHAPE( ),wth(w),hei(h),chanel(c)		{	layout=l;	}
	SHAPE_IMAGE( const SHAPE_IMAGE &im ):wth(im.wth),hei(im.hei),chanel(im.chanel)	{	layout=im.layout;	x=im.x;		}
	SHAPE_IMAGE& operator=(const SHAPE_IMAGE& si )	
	{	wth=si.wth,hei=si.hei,chanel=si.chanel,layout=si.layout;	bpp=si.bpp;x=si.x;	return *this;	}

	virtual void Init( const string &sParam ){
		int n0,n1,n2,n3;
		int n=sscanf(sParam.c_str(),"%d %d %d %d",&n0,&n1,&n2,&n3);
		if( n>=3 )	{			wth=n0,	hei=n1,	chanel=n2;		}
		if( n>=4 )	{			x=n3;		}
	}
	virtual void Init( int w,int h,int c=1,LAYOUT l=UNKNOWN )		
	{	wth=w,		hei=h,		chanel=c,		layout=l;	}
	virtual bool canConv( const SHAPE_IMAGE *sia,const SHAPE_IMAGE *sib,int flag )	const	{
	//	��ʱȡ��chanel���		1/31/2015		cys
	//	if( chanel!=sia->chanel || chanel!=sib->chanel )
	//		return false;
		if( sia->wth-sib->wth!=wth-1 )
			return false;
		if( sia->hei-sib->hei!=hei-1 )
			return false;
		return true;
	}

//pixels in all maps
	virtual unsigned int Count( )	const	
	{	return wth*hei*chanel;	}
	virtual unsigned int Pixel( )	const	{	return wth*hei;	}

	virtual void PosInfo( int pos_0,int &no,int &r,int &c,int flag=0x0  ){
//		assert( chanel==1 );
		int ld=Count( ),pos=pos_0%ld;
		no=pos_0/ld;
		r=pos/wth;		c=pos%wth;;
	}

	int Export( FILE *fp,int type,int flag ){
		if( fwrite( &wth,sizeof(unsigned int),1,fp)!=1 )			{		return -100;		}
		if( fwrite( &hei,sizeof(unsigned int),1,fp)!=1 )			{		return -101;		}
		if( fwrite( &maps,sizeof(unsigned int),1,fp)!=1 )			{		return -102;		}
		if( fwrite( &x,sizeof(unsigned int),1,fp)!=1 )				{		return -103;		}
		return 0x0;
	}
	int Import( FILE *fp,int type,int flag ){
		if( fread( &wth,sizeof(unsigned int),1,fp)!=1 )				{		return -100;		}
		if( fread( &hei,sizeof(unsigned int),1,fp)!=1 )				{		return -101;		}
		if( fread( &maps,sizeof(unsigned int),1,fp)!=1 )			{		return -102;		}
		if( fread( &x,sizeof(unsigned int),1,fp)!=1 )				{		return -103;		}
		return 0x0;
	}
	int Import( char* &pb,int type,int flag );

	template<typename T>
	void Sub( T *X,int r0,int c0,int r1,int c1,T *Z,int flag=0x0 ){
		assert( r0>=0 && r1<hei && c0>=0 && c1<wth );		
		int r,c,pos,nz=0,map;
		T *mX=X;
		for( map=0; map<maps; map++,mX+=wth*hei )	{
			for( r=r0; r<r1; r++ ){
			for( c=c0; c<c1; c++ ){
				pos=r*wth+c;		Z[nz++] = mX[pos];
			}
			}
		}

	}
};

//Image Augamentation
struct IMAGE_AUG{
	typedef enum{
		ALIGN_OFF=0x0,
		IMAG_CENTER=0x10,IMAG_MAX_CENTER,IMAG_FLOAT,IMAG_MAXFLOAT,IMAG_ALL,		//IMAGE
		IMAG_BACKGROUND,RAND_SUB,
		SAMP_4=0x20,SAMP_5,
	}TYPE;
	TYPE align;
	static TYPE Str2Align( string token,int flag=0x0 );
	//tW<=wMap,tH<=hMap
	int wMap,hMap,tW,tH,ang;
	double param[8],sZoom;
	//Pick area in image source
	int L,R,T,B;

	IMAGE_AUG( int wM,int hM,TYPE t,int flag=0x0 );
	~IMAGE_AUG( )	{		}
	void Augment( int wth_0,int hei_0,float xita=0.0,int flag=0x0 );

	bool isRandom( )	{
		return align==IMAGE_AUG::IMAG_FLOAT || align==IMAGE_AUG::IMAG_MAXFLOAT;
	}
};

#define ROCLS_MOST	8
struct RoCLS{	//Region of class
	float p;
	int clsno,flag;
	int c_0,r_0,wth,hei;
	void *hX;

	RoCLS():c_0(-1),r_0(-1),wth(-1),hei(-1),clsno(-1),flag(-1),p(0.),hX(nullptr){}
	RoCLS( int c_,int r_,int w_,int h_,int n_=0,double p_=0):c_0(c_),r_0(r_),wth(w_),hei(h_),clsno(n_),p(float(p_)),flag(0x0),hX(nullptr){}
	RoCLS& operator=(const RoCLS& x )	{	
		c_0=x.c_0,r_0=x.r_0,wth=x.wth,hei=x.hei,clsno=x.clsno,p=x.p,flag=x.flag,hX=x.hX;	return *this;	}
	static bool isBig( const RoCLS &l,const RoCLS &r)			{	return l.p>r.p ;	}

	void Serial( SERIAL &seral ){
		seral.T(c_0).T(r_0).T(wth).T(hei).T(p);
	}
	int Margin( int width,int height){
		int dr=MIN(r_0,height-r_0-hei),dc=MIN(c_0,width-c_0-wth),margin=MIN(dr,dc);
		return MAX(0,margin);
	}
	void Expand( double s ){
		int dc=(int)(wth*s),dr=(int)(hei*s);
		c_0-=dc;		wth+=2*dc;
		r_0-=dr;		hei+=2*dr;
	}

	//(-1:1,-1:1)=>ROI
	void Map1_f( float x0,float y0,int &x1,int &y1 ){
		double sX=wth/2.0,sY=hei/2.0,oX=c_0+wth/2.0,oY=r_0+hei/2.0;
		assert( sX>=32 );
		x1 = (int)(oX+x0*sX+0.5);		
		y1 = (int)(oY+y0*sY+0.5);
	}
	void UnMap1_f( int x1,int y1,float &x0,float &y0 ){
		double sX=wth/2.0,sY=hei/2.0,oX=c_0+wth/2.0,oY=r_0+hei/2.0;
		assert( sX>=32 );
		x0=(x1-oX)/sX;			y0=(y1-oY)/sY;
		//x1 = (int)(oX+x0*sX+0.5);		
		//y1 = (int)(oY+y0*sY+0.5);
	}

    void Map_Poly( POLYGON &poly,int nPt,int *pts,float*sx,float*sy ){
        int i,no,c,r;
        for( i=0;i <nPt; i++ ){
            no=pts[i];
            Map1_f( sx[no],sy[no],c,r );
            poly.push_back( F4NO(-1.0,i,c,r) );
        }
    }
    void Map_Poly( POLYGON &poly,int nPt,float*sx,float*sy ){
        int i,no,c,r;
        for( i=0;i <nPt; i++ ){
            no=i;
            Map1_f( sx[no],sy[no],c,r );
            poly.push_back( F4NO(-1.0,i,c,r) );
        }
    }
};

