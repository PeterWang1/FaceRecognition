#include "GST_bitmap.hpp"
#include "../util/GST_util.hpp"
#include <random>
#include <time.h>
#include <thread>

#ifdef WIN32
	#include <windows.h>
	#include <assert.h>
	#include <math.h>
	#include "FreeImage.h"
#if defined _WIN64
	#pragma comment( lib, "F:\\GiPack\\lib\\x64\\Release\\FreeImage.lib" )
#else
	#pragma comment( lib, "F:\\GiPack\\lib\\Release\\FreeImage.lib" )
#endif
#else
    #include "FreeImage.h"
#endif

using namespace Grusoft;
#define ASSERT assert
static char sDumpPath[500];
string GST_BMP::sDumpFolder="D:\\trace\\";

static double _MASK_3_w_sum[7]={
	6,6,8,8,8,8,9
};
static double GIP_MASK_3_3[7][9]={
	{-1,-1,-1,0,0,0,1,1,1},		//PREWITT_X
	{-1,0,1,-1,0,1,-1,0,1},		//PREWITT_Y
	{-1,-2,-1,0,0,0,1,2,1},		//SOBEL_R
	{-1,0,1,-2,0,2,-1,0,1},		//SOBEL_C
	{-2,-1,0,-1,0,1,0,1,2},		//SOBEL_45
	{0,-1,-2,1,0,-1,2,1,0},		//SOBEL_135
	{1,1,1,1,1,1,1,1,1},		//G_MASK_MEAN
};
double G_Mask_3_( int pos,int M,int N,int ldu,BIT_8 *u,BMPP::MASK type,int flag )	{
	int i,r,c,supp=9,pos_1;
	int GIP_SUPP_r[]={-1,-1,-1,0,0,0,1,1,1},GIP_SUPP_c[]={-1,0,1,-1,0,1,-1,0,1};
	double a,*w;								//{-1,1,-ldu,ldu}

	a = 0.0;
	w=GIP_MASK_3_3[type-BMPP::MASK_3_3];
	for( i = 0; i < supp; i++ )	{
		r=G_POS2R(pos,ldu),		c=G_POS2C(pos,ldu);
		r+=GIP_SUPP_r[i];			c+=GIP_SUPP_c[i];
		if( r==-1 )	r=1;
		if( r==M )	r=M-2;
		if( c==-1 )	c=1;
		if( c==N )	c=N-2;
		ASSERT( r>=0 && r<M && c>=0 && c<N );
		pos_1=G_RC2POS( r,c,ldu );
		a += w[i]*u[pos_1];
	}
	a /= _MASK_3_w_sum[type-BMPP::MASK_3_3];

	return a;
}

template<typename T>
void BMPP_save( SHAPE_IMAGE *si,int nMap,T *data,int no,int x=0 ){
	BMPP gmp(si);		gmp.no=no;	
	char sPath[1000];
	sprintf( sPath,"%s_$%d_.bmp",GST_BMP::sDumpFolder.c_str(),no );
	gmp.Set(data,nMap)->Save( sPath,0 );
}

//教训啊，NDK不支持wchar_t!
FIBITMAP *FreeImage_Open_( const char *sPath,GST_BMP::INITIAL &init,int flag ){
	try{
		FIBITMAP *fib=nullptr;
		FREE_IMAGE_FORMAT fif=FIF_UNKNOWN;
		fif = FreeImage_GetFileType( sPath );
		if( fif==FIF_UNKNOWN )		fif=FIF_JPEG;
		fib=FreeImage_Load( fif,sPath );
		if( fib==nullptr ){
			for( fif=FIF_BMP;fif<=FIF_RAW;fif=(FREE_IMAGE_FORMAT)(fif+1) ){
				fib=FreeImage_Load( fif,sPath );
				if(fib!=nullptr)
					break;
			}
		}
		if( fib==nullptr )
			GST_THROW ("GST_BMP failed to load from sPath!");
		int bpp=FreeImage_GetBPP( fib ),wth=FreeImage_GetWidth(fib),hei=FreeImage_GetHeight(fib);
		init.bpp=bpp;		init.wth_0=wth;			init.hei_0=hei;
		if( init.wMax>0 && init.hMax>0 && (wth>init.wMax || hei>init.hMax) ){
			double sW=init.wMax*1.0/wth,sH=init.hMax*1.0/hei,s=MIN(sW,sH);
			wth=(int)(s*wth+0.5),hei=(int)(s*hei+0.5);
			FIBITMAP *dib=FreeImage_Rescale( fib, wth, hei, FILTER_CATMULLROM );
			if( dib!=NULL ){
				FreeImage_Unload(fib);			fib=dib;
			}else{
				GST_THROW( "GST_BMP- init size failed" );
			}
			init.sH=s;			init.sW=s;
			bpp=FreeImage_GetBPP( fib ),wth=FreeImage_GetWidth(fib),hei=FreeImage_GetHeight(fib);
		}
		if( bpp>32 ){
			FIBITMAP *fib32 = FreeImage_ConvertTo32Bits( fib );
			FreeImage_Unload( fib );		fib=fib32;
		}
		if( 0 ){
			FIBITMAP *fib8 = FreeImage_ConvertToGreyscale( fib );
			FreeImage_Unload( fib );		fib=fib8;
		}else{

		}
		return fib;

	}catch( ... ){
		GST_THROW( "GST_BMP failed load from sPath ..." );

	}
}
FIBITMAP *FreeImage_Open_( const wchar_t *sPath,GST_BMP::INITIAL &init,int flag ){
try{
	FIBITMAP *fib=nullptr;
	FREE_IMAGE_FORMAT fif=FIF_UNKNOWN;
	fif = FreeImage_GetFileTypeU( sPath );
	if( fif==FIF_UNKNOWN )		fif=FIF_JPEG;
	fib=FreeImage_LoadU( fif,sPath );
	if( fib==nullptr ){
		for( fif=FIF_BMP;fif<=FIF_RAW;fif=(FREE_IMAGE_FORMAT)(fif+1) ){
			fib=FreeImage_LoadU( fif,sPath );
			if(fib!=nullptr)	
				break;
		}
	}
	if( fib==nullptr )
		GST_THROW ("GST_BMP failed to load from sPath!");
	int bpp=FreeImage_GetBPP( fib ),wth=FreeImage_GetWidth(fib),hei=FreeImage_GetHeight(fib);
	init.bpp=bpp;		init.wth_0=wth;			init.hei_0=hei;
	if( init.wMax>0 && init.hMax>0 && (wth>init.wMax || hei>init.hMax) ){
		double sW=init.wMax*1.0/wth,sH=init.hMax*1.0/hei,s=MIN(sW,sH);
		wth=(int)(s*wth+0.5),hei=(int)(s*hei+0.5);
		FIBITMAP *dib=FreeImage_Rescale( fib, wth, hei, FILTER_CATMULLROM );
		if( dib!=NULL ){
			FreeImage_Unload(fib);			fib=dib;
		}else{
			GST_THROW( "GST_BMP- init size failed" );
		}
		init.sH=s;			init.sW=s;			
		bpp=FreeImage_GetBPP( fib ),wth=FreeImage_GetWidth(fib),hei=FreeImage_GetHeight(fib);
	}
	if( bpp>32 ){
		FIBITMAP *fib32 = FreeImage_ConvertTo32Bits( fib );
		FreeImage_Unload( fib );		fib=fib32;
	} 	
	if( 0 ){
		FIBITMAP *fib8 = FreeImage_ConvertToGreyscale( fib );
		FreeImage_Unload( fib );		fib=fib8;
	}else{
		
	}
	return fib;

}catch( ... ){
	GST_THROW( "GST_BMP failed load from sPath ..." );

}
}
GST_BMP::GST_BMP( const wstring &sPath,INITIAL ini_,int flag ):space(this),init(ini_){
	FIBITMAP *fib=FreeImage_Open_( sPath.c_str(),init,flag );
	hib=fib;
	int bpp=FreeImage_GetBPP( fib ),wth=FreeImage_GetWidth(fib),hei=FreeImage_GetHeight(fib);
	si.Init( wth,hei,bpp<=8 ? 1 : 3 );		si.bpp=bpp;
}
GST_BMP::GST_BMP( const string &sPath,INITIAL ini_,int flag ) : space(this),init(ini_) {
	FIBITMAP *fib=FreeImage_Open_( sPath.c_str(),init,flag );
	hib=fib;
	int bpp=FreeImage_GetBPP( fib ),wth=FreeImage_GetWidth(fib),hei=FreeImage_GetHeight(fib);
	si.Init( wth,hei,bpp<=8 ? 1 : 3 );		si.bpp=bpp;
}


GST_BMP::GST_BMP( GST_BMP *hSrc,int trans,RoCLS* roi,int flag ) :hib(nullptr),space(this){
	FIBITMAP *fib=(FIBITMAP *)(hSrc->GetHBitmap( ));
	int left=0,r0=0,right=FreeImage_GetWidth(fib),r1=FreeImage_GetHeight(fib),r,c,nz=0;
	if( roi!=NULL ){
		left=MAX(0,roi->c_0),	right=MIN(right,roi->c_0+roi->wth);
		r0=MAX(0,roi->r_0),		r1=MIN(r1,roi->r_0+roi->hei);
	}
//The coordinate system uses usual graphics conventions. �����ˣ�		cys		12/18/2015
	FIBITMAP *sub=FreeImage_Copy( fib,left,r0,right,r1 );		
	if( sub==NULL )			throw "GST_BMP::sub is 0";
	RGBQUAD quad;
	for(r=r0;r<r1;r++){
		for(c=left;c<right;c++,nz++){
			FreeImage_GetPixelColor( fib,c,r,&quad );
			FreeImage_SetPixelColor( sub,c-left,r-r0,&quad );
		}
	}
//	FreeImage_Save(FIF_BMP,fib,"D:\\trace\\fib.bmp");
//	FreeImage_Save(FIF_BMP,sub,"D:\\trace\\sub.bmp");
	si=hSrc->si;			hib=sub;		
	si.wth=FreeImage_GetWidth(sub);			si.hei=FreeImage_GetHeight(sub);
}
GST_BMP::GST_BMP( SHAPE_IMAGE *sin,int type,int flag ):space(this){
	GST_VERIFY( sin->bpp==8 || sin->bpp==16 || sin->bpp==24 || sin->bpp==32,"GST_BMP bpp is X" );
	hib=FreeImage_Allocate(sin->wth,sin->hei,sin->bpp );	
	if( hib==NULL )
		throw "GST_BMP::hib is 0";
	si= *sin;
}

GST_BMP::GST_BMP( BIT_8 *gray,SHAPE_IMAGE *sin,int flag ):space(this){
	si= *sin;		si.bpp=8;
	//The last parameter, topdown, will store the bitmap top-left pixelfirst when it is TRUE or bottom-left pixel first when it is FALSE.
	hib= FreeImage_ConvertFromRawBits(gray, si.wth,si.hei,si.wth, 8, 0, 0, 0, false);
	if( hib==NULL )
		throw "GST_BMP::hib is 0";
	if( 0 )	{
		FreeImage_Save(FIF_BMP,(FIBITMAP *)hib,"D:\\trace\\gray.bmp");
	}
}
/*void GST_BMP::InitCCC( int flag ){
//	this->Save( sDumpFolder+"1.bmp" );
	if( ccc!=nullptr )
		delete[] ccc;
	ccc=new BIT_8[si.Count()]();
	ToLUV( );	
}*/

GST_BMP::~GST_BMP( ){
	if( hib!=nullptr ){
		FreeImage_Unload( (FIBITMAP*)hib );		hib=nullptr;
	}
}

bool GST_BMP::Save( const string &sPath,int flag ){
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename( sPath.c_str() );
	//fif=FIF_BMP;
	return  (bool)FreeImage_Save(fif,(FIBITMAP*)hib,sPath.c_str() );
}
bool GST_BMP::Load( const string &sPath,int flag ){
	wchar_t wPath[1000];
	GST_util::CHAR2W( sPath.c_str(),wPath,1000 );
	INITIAL initial;
	FIBITMAP *dib=FreeImage_Open_( wPath,initial,flag );
	if( dib!=nullptr ){
		FreeImage_Unload((FIBITMAP*)hib);
		hib=dib;
		SetModified( );	
		return true;
	}
	return  false;
}

void GST_BMP::SetModified( int flag ){
	si.wth=FreeImage_GetWidth((FIBITMAP*)hib);		si.hei=FreeImage_GetHeight((FIBITMAP*)hib);
	ToColorSpace( ColorSpace::CF_0 );
}
bool GST_BMP::ToSize( int wth,int hei,bool isKeepRatio,int flag ){
	GST_VERIFY( wth>0 && hei>0, "GST_BMP::ToSize must >0" );
	if( isKeepRatio ){
		double sW=wth*1.0/Width(),sH=hei*1.0/Height(),s=MIN(sW,sH);
		wth=(int)(s*Width()+0.5),hei=(int)(s*Height()+0.5);
	}
	if( wth==Width( ) && hei==Height( ) )
		return true;
	FIBITMAP *dib=FreeImage_Rescale( (FIBITMAP*)hib, wth, hei, FILTER_CATMULLROM );
	if( dib!=NULL ){
		FreeImage_Unload((FIBITMAP*)hib);
		hib=dib;
		SetModified( );	//ToColorSpace( ColorSpace::CF_0 );
		
		return true;
	}
	return  false;
}
bool GST_BMP::Rescale( double sW,double sH,int flag ){
	int wth0=Width(),hei0=Height(),wth=(int)(sW*wth0+0.5),hei=(int)(sH*hei0+0.5);
	return ToSize( wth,hei,false );
	/*FIBITMAP *dib=FreeImage_Rescale( (FIBITMAP*)hib, wth, hei, FILTER_CATMULLROM );
	if( dib!=NULL ){
		FreeImage_Unload((FIBITMAP*)hib);
		hib=dib;
		SetModified( );	//ToColorSpace( ColorSpace::CF_0 );
		
		return true;
	}
	return false;*/
}
GST_BMP* GST_BMP::FlipHorizontal( ){
	if( FreeImage_FlipHorizontal((FIBITMAP*)hib)==TRUE ){
		SetModified( );
	}
	return this;
}
GST_BMP* GST_BMP::FlipVertical( ){
	if( FreeImage_FlipVertical((FIBITMAP*)hib)==TRUE ){
		SetModified( );
	}
	return this;
}

bool GST_BMP::GetPixelColor( unsigned int c, unsigned int r, BIT_8 *c1,BIT_8 *c2,BIT_8 *c3,PIXEL_INFO type ){
/*	if(  FreeImage_GetPixelColor( dib,c,(aug.tH-1-r),&cr ) )	{
				Xr[nz] = cr.rgbRed;		Xg[nz] = cr.rgbGreen;			Xb[nz] = cr.rgbBlue;
			}else if( FreeImage_GetPixelIndex( dib,c,(aug.tH-1-r),&index ) ){
				if( pallet!=NULL ){
					Xr[nz]=pallet[index].rgbRed;	Xg[nz]=pallet[index].rgbGreen;	Xb[nz]=pallet[index].rgbBlue;
				}	else			{
					if( r==0&&c==0 )		printf( "ImportImage: %s is GRAY!!!\n",sPath);
					Xr[nz] = index;			Xg[nz] = index;					Xb[nz] = index;
				}
			}else{
				throw "DATASET_cr::ImportImage:GetPixelColor is failed!";
			}*/
	RGBQUAD value;
	if( FreeImage_GetPixelColor( (FIBITMAP*)hib, c, r,&value) ){
		*c1=value.rgbRed,	*c2=value.rgbGreen,	*c3=value.rgbBlue;
		return true;
	}else{
		return false;
	}
}
bool GST_BMP::GetPixelX( unsigned int x, unsigned int y, void *value,PIXEL_INFO type){
	switch( type ){
	case QUAD:
		return (bool)FreeImage_GetPixelColor( (FIBITMAP*)hib, x, y,(RGBQUAD *)value);
	default:
		GST_THROW( "GST_BMP::GetPixelX is ..." );
	}
}
BIT_8 GST_BMP::GetPixelGray( unsigned int c, unsigned int r ){
	BIT_8 cr,cg,cb;	//RGBQUAD quad;
	GetPixelColor( c,r,&cr,&cg,&cb );
	BIT_8 gray=(BIT_8)(cr*0.299+cg*0.587+cb*0.114+0.5);
	return gray;
}

//bool GST_BMP::SetPixelColor( unsigned int x, unsigned int y, void *value){}
bool GST_BMP::SetPixelColor( unsigned int c, unsigned int r, BIT_8 cr,BIT_8 cg,BIT_8 cb,void *value ){
	if( !G_RC_VALID(c,0,si.wth-1) || !G_RC_VALID(r,0,si.hei-1) )
		return false;
	if( value!=nullptr )
		return (bool)FreeImage_SetPixelColor( (FIBITMAP*)hib, c, r,(RGBQUAD *)value );
	else{
		RGBQUAD quad;
		quad.rgbRed=cr;			quad.rgbGreen=cg;			quad.rgbBlue=cb;
		return (bool)FreeImage_SetPixelColor( (FIBITMAP*)hib, c, r,&quad );
	}
}
/*
BMPP* BMPP::Gray( int flag )	{
	if( !state.isOK( ) )		return this;

	BIT_8 g;
	INT_32 r,c,i,pos=0,pos_0=0,MN=0,g_0=256,g_1=0,width=si.wth,height=si.hei;

//	no = no;
	MN = width*height;
	gray = new BIT_8[MN]();			
	//mask = new BIT_8[MN]();		
	memset( mask,0x0,sizeof(BIT_8)*MN );
	width=width,		height=height;		pos=0;
	for( r = 0; r < height; r++ )	{
		for( c = 0; c < width; c++ )	{
			pos_0 = G_RC2POS( r,c,width );
			gray[pos] = g = GetPixelGray(c,r);
			g_0=MIN(g_0,g);					g_1=MAX(g_1,g);
			mask[pos]=0;			
			pos++;
		}
	}
	ASSERT( pos==MN );
	gray_0 = g_0;				gray_1 = g_1;	
	BMPP_save( &si,1,gray,11 );
	state.trans.push_back( T_GRAY );
	return this;
}*/

BMPP* BMPP::GetInfo( INFO_TYPE type,void*pinfo,void **ppinfo,int flag ){
	int i,nPixel=si.Pixel( );
	switch( type ){
	case MASK_0:		case MASK_1:	case MASK_2:{
		vector<int>*vPos=(vector<int>*)(pinfo);
		for( i=0;i<nPixel;i++ ){
			if( mask[i]==type-MASK_0 )	
				vPos->push_back(i);
		}
		}		break;
	case GRAD_INTENS:
		*ppinfo=g_I;
		break;
	case GRAD_ANGLE:
		*ppinfo=g_rad;
		break;
	default:
		GST_THROW("BMPP::GetInfo type is ...");
	}
	return this;
}
BMPP* BMPP::Gradient( int flag )	{
	if( !state.isOK( ) )		return this;

	INT_32 r,c,i,pos=0,pos_0=0,M=Height(),N=Width();
	if( 1 ){
		g_I = (float*)malloc( sizeof(float)*M*N );		g_rad = (float*)malloc( sizeof(float)*M*N );	
	}
	g_I_0=FLT_MAX,g_I_1=-FLT_MAX;
	double gr,gc;
	BIT_8 *gray=nullptr;
	Channel_( GRAY,gray );
	for( r = 0; r < M; r++ )	{
	for( c = 0; c < N; c++ )	{
		pos=G_RC2POS( r,c,N );
		gr = -G_Mask_3_( pos,M,N,N,gray,SOBEL_R,0x0 );
		gc = -G_Mask_3_( pos,M,N,N,gray,SOBEL_C,0x0 );
		g_I[pos] = (sqrt( gr*gr+gc*gc ));
		g_rad[pos] =  (atan2( gr,gc ));
		g_I_0=MIN(g_I_0,g_I[pos]);		g_I_1=MAX(g_I_1,g_I[pos]);
	}
	}
	state.trans.push_back( T_GRADIENT );
	return this;
}
	

INT_32 G_adjacent_U8(INT_32 M,INT_32 N,INT_32 ldu,BIT_8 *u,INT_32 r,INT_32 c,INT_32 *adj,BMPP::ADJACENT type  )	{
	INT_32 i,nAdj=0,r_1,c_1,pos_1,pos_2,pos_3,N4=4,N8=8,target;
	INT_32 GIP_SUPP_r[]={0,0,-1,1,-1,-1,1,1},GIP_SUPP_c[]={-1,1,0,0,-1,1,-1,1};

	pos_1=G_RC2POS( r,c,ldu );
	target=u[pos_1];
	for( i = 0; i < N8; i++ )	{
		r_1=r+GIP_SUPP_r[i];			c_1=c+GIP_SUPP_c[i];
		if( !G_RC_VALID(r_1,0,M-1) || !G_RC_VALID(c_1,0,N-1) )
			continue;
		if( i>=N4 && type==BMPP::ADJACENT_4 )		//GIP_ADJACENT_4
			break;
		pos_1=G_RC2POS( r_1,c_1,ldu );
		if( i>N4 && type==BMPP::ADJACENT_M )	{	//GIP_ADJACENT_M
			pos_2 = G_RC2POS( r,c_1,ldu );
			pos_3 = G_RC2POS( r_1,c,ldu );
			if( u[pos_1]==target && u[pos_2]!=target && u[pos_3]!=target)	{
				adj[nAdj++]=pos_1;
			}
		}else	{								//GIP_ADJACENT_8
			if( u[pos_1]==target )
			{	adj[nAdj++]=pos_1;			}
		}
	}
	return nAdj;
}

int BMPP::Init( int flag ){
	int nPixel=si.Pixel();
	if( mask!=nullptr )		
		delete[] mask;
	mask=new BIT_8[nPixel]();
	return 0x0;	
}
BMPP::~BMPP( ){
	if( g_I!=nullptr )			delete[] g_I;
	if( g_rad!=nullptr )		delete[] g_rad;
//	if( gray!=nullptr )			delete[] gray;
	if( mask!=nullptr )			delete[] mask;
	//if( tmp8!=nullptr )			delete[] tmp8;
}

BMPP* BMPP::Shrink( double sW,double sH,int flag ){
	GST_VERIFY( sW<1.0&& sH<1.0,"BMPP::Shrink s is XXX" );
	if( BMPP::Rescale( sW,sH ) )	{
		Init( 0x0 );
	}
	return this;
}

BMPP* BMPP::EdgeCanny( float s_I0,float s_I1,float T_g0,float T_g1,int flag ){
//	if( !state.isTransed( T_GRAY ) )		return this;
	assert(0);
	INT_32 pos,M=si.hei ,N=si.wth,r,c,dgr,nz,adj[8],e_nz;
	INT_32 GIP_SUPP_r[]={0,0,-1,1,-1,-1,1,1},GIP_SUPP_c[]={-1,1,0,0,-1,1,-1,1},i;
	float gr,gc,w,g_t0=0,g_t1=1;
	float g_I_0=FLT_MAX,g_I_1=-FLT_MAX;
	BIT_8 I_1=gray_1*s_I1,I_0=gray_1*s_I0,*gray=nullptr;
	
	Channel_(GRAY,gray);
	assert( s_I0<s_I1 && g_t0<g_t1 );
	for( r = 0; r < M; r++ )	{
	for( c = 0; c < N; c++ )	{
		pos=G_RC2POS( r,c,N );
		mask[pos]=0;
		if( gray[pos]>I_1 || gray[pos]<I_0 )
		{	mask[pos]=-1;			}
		gr = -G_Mask_3_( pos,M,N,N,gray,SOBEL_R,0x0 );
		gc = -G_Mask_3_( pos,M,N,N,gray,SOBEL_C,0x0 );
		g_I[pos] = (sqrt( gr*gr+gc*gc ));
		g_rad[pos] =  (atan2( gr,gc ));
	//	if( (g_rad[pos]>PI*0.35 &&  g_rad[pos]<PI*0.65) || (g_rad[pos]>-PI*0.65 &&  g_rad[pos]<-PI*0.35) )
	//		g_I[pos] = 0.f;
		g_I_0=MIN(g_I_0,g_I[pos]);		g_I_1=MAX(g_I_1,g_I[pos]);
	}
	}
//	BMPP_save( &si,1,g_I,13 );

	g_t1 = g_I_1*T_g1;	g_t0 = g_t1*T_g0;
	e_nz=0;
	for( r = 1; r < M-1; r++ )	{
	for( c = 1; c < N-1; c++ )	{
		pos=G_RC2POS( r,c,N );
		if( mask[pos]<0 )
			continue;
		if( g_I[pos] < g_t0 )
			continue;
		w = fabs(g_rad[pos])*57.295779513082;
		dgr = (22.5<w&&w<=67.5) ? 1 : (67.5<w&&w<=112.5) ? 2 : (112.5<w&&w<=157.5) ? 3 : 0;
		if( dgr==0 && (g_I[pos]<g_I[pos-1] || g_I[pos]<g_I[pos+1]) )		//nonmaximum suppression
			continue;
		if( dgr==3 && (g_I[pos]<g_I[pos-N-1] || g_I[pos]<g_I[pos+N+1]) )
			continue;
		if( dgr==2 && (g_I[pos]<g_I[pos-N] || g_I[pos]<g_I[pos+N]) )
			continue;
		if( dgr==1 && (g_I[pos]<g_I[pos-N+1] || g_I[pos]<g_I[pos+N-1]) )
			continue;
		if( g_I[pos] >= g_t1 )	{
			mask[pos] = 1;			e_nz++;
		}else
			mask[pos] = 10;
	}
	}

	do{
		nz = 0;
		for( r = 1; r < M-1; r++ )	{
		for( c = 1; c < N-1; c++ )	{
			pos=G_RC2POS( r,c,N );
			if( mask[pos]!=10 )		continue;
			mask[pos]=1;
			if( G_adjacent_U8( M, N,N,mask,r,c,adj,ADJACENT_M )>0 )	{
				mask[pos]=1;		nz++;
			}else
				mask[pos]=10;
		}
		}
		e_nz += nz;
	}while( nz>0 );
	BMPP_save( &si,1,mask,no+12 );
/*	_DUMP_IF_( 0607,LOGP( "GE_IMAGE_canny{I=(%d,%d)} g=(%g,%g)(%g,%g) %d",I_0,I_1,g_I_0,g_I_1,g_t0,g_t1,e_nz ));
	if( 0 )	{	//mask����˱ߵ���Ϣ����Բ����޸�mask��ֵ��
		BIT_8 *tmp8=(BIT_8*)malloc(sizeof(BIT_8)*M*N);
		for( i =0; i<M*N; i++ ){	tmp8[i]=mask[i]==1 ? 0 : 255;	}
		BMPP_save( &si,1,tmp8,no+12 );
		free(tmp8);
	}*/

	return this;;
}

BMPP* BMPP::SaveMask( const string &sPath_0,BIT_8 *ground,BMP_OUTPUT type,int flag ){
	string sPath=sDumpFolder+sPath_0;
	if( ground==nullptr )		ground=mask;

	BMPP bmp(this);
	BIT_8 gray,cr,cg,cb;
	unsigned int r,c,pos=0,wth=si.wth,hei=si.hei;
//	for( r = 0; r < hei; r++ )	{
//	for( c = 0; c < wth; c++,pos++ )	{	
	for( r = 1; r < hei-1; r++ )	{
	for( c = 1; c < wth-1; c++ )	{	
		pos=G_RC2POS( r,c,wth );
		gray=GetPixelGray(c,r);		
		switch( type ){
		case MSK_RED_GRN:
			if( ground[pos]==1 ){
				cr=255;	cg=0;	cb=0;
			}else if( ground[pos]==2 ){
				cr=0;	cg=255;	cb=0;
			}else{
				cr=gray,cg=gray,cb=gray;
			}
			break;
		case MSK_BLACK:
			if( ground[pos]==1 ){	cr=0;	cg=0;	cb=0;	}
			else
				continue;
			break;
		case MSK_BORDER:
			if( ground[pos]!=1 )	continue;
			if( ground[pos-1]==0 || ground[pos+1]==0 || ground[pos+wth]==0 || ground[pos-wth]==0 )
			{	cr=0;	cg=0;	cb=0;			}
			else
				continue;
			break;
		default:
			GST_THROW( "BMPP::SaveMask is ...");
		}
		bmp.SetPixelColor( c,r,cr,cg,cb );
	}
	}
	bmp.Save( sPath,flag );
	return this;
}


SE_CIRCLE::SE_CIRCLE( INT_32 W_,INT_32 H_,INT_32 flag ) : STRUC_ELE(){	
	type = CIRCLE;
	int r,c,pos;
	float dist,a,b,R=H_,area=PI*R*R;
	H_0=H_1=H_;		W_1=W_0=W_;
	ldw = 2*W_1+1;
	ASSERT( R==W_1 );

	M_nz = 0;
	mask = (BIT_8*)malloc( sizeof(BIT_8)*(2*H_1+1)*(2*W_1+1) );
	for( r = 0; r< 2*H_1+1; r++ )	{
	for( c = 0; c< 2*W_1+1; c++ )	{
		pos = G_RC2POS( r,c,ldw);
		ASSERT( pos>=0 && pos<(2*H_1+1)*(2*W_1+1) );
		a = r-H_1;			b = c-W_1;
		dist = sqrt( a*a+b*b );
		if( dist<=R )
		{	mask[pos] = 1;		M_nz++;	}
		else
			mask[pos] = 0;
	}
	}
}

BMPP* BMPP::TagBorder( BIT_8 *tag,BIT_8 *border,INT_32 flag ){
	GST_VERIFY( border!=tag,"BMPP::TagBorder" );
	int r,c,pos,width=si.wth,height=si.hei,j,pos_j,nPixel=si.Pixel();
	INT_32 neibor_3[4]={-width,-1,1,width};
	BIT_8 cur;

	memset( border,0x0,sizeof(BIT_8)*nPixel );
	for( r = 1; r< height-1; r++ )	{
	for( c = 1; c< width-1; c++ )	{
		pos = G_RC2POS( r,c,width);
		if( (cur=tag[pos])!=1 )
			continue;
		for( j=0; j<4; j++ )	{
			pos_j = pos+neibor_3[j];
			if( tag[pos_j]!=cur )
			{ border[pos]=1;	break;	}
		}
	}
	}

	return this;
}

BMPP* BMPP::MASK_Polygon( const POLYGON&poly,RoCLS &roi,BIT_8 b8,bool isFill,int flag ){
	int wth=Width(),hei=Height(),i,nPt=poly.size(),r,c,r0=hei,r1=0,c0=wth,c1=0,pos,step;
	int rMin=hei,rMax=0,cMin=wth,cMax=0,from=0,to;
	memset( mask,0x0,sizeof(BIT_8)*wth*hei );
	float s;
	for( i=0;i<nPt-1;i++ ){		//draw line
		c0=poly[i].c;		r0=poly[i].r;
		c1=poly[i+1].c;		r1=poly[i+1].r;
		rMax=MAX(r0,rMax);			cMax=MAX(c0,cMax);
		rMin=MIN(r0,rMin);			cMin=MIN(c0,cMin);

		if( r1==r0 && c1==c0 )		continue;
		if( abs(r1-r0)>abs(c1-c0) ){
			s = (c1-c0)*1.0/(r1-r0);		step=r1>r0?1:-1;
			for( r=r0;r!=r1;r+=step ){
				c=(int)(c0+(r-r0)*s+0.5);
				pos=G_RC2POS(r,c,wth);	mask[pos]=b8;
			}
		}else{
			s = (r1-r0)*1.0/(c1-c0);
			for( c=c0;c!=c1;c+=step ){		step=c1>c0?1:-1;
				r=(int)(r0+(c-c0)*s+0.5);
				pos=G_RC2POS(r,c,wth);	mask[pos]=b8;
			}
		}
	}
	roi.r_0=rMin;						roi.c_0=cMin;
	roi.wth=rMax-rMin+1;				roi.hei=cMax-cMin+1;
	for( r = rMin; r<=rMax; r++ )	{		//scan line
		c=cMin;		pos=G_RC2POS(r,c,wth);
		from=mask[pos]==b8?pos:-1;		to=-1;
		while( (++c)<=cMax )	{
			pos=G_RC2POS(r,c,wth);
			if( mask[pos]==b8 ){
				if( from==pos-1 )	{		//水平边界
					from=pos;
				}else{
					if( from==-1 )
					{	from=pos;	}
					else
					{	to=pos;	break;	}
				}
			}
		}
		if( from!=-1 && to!=-1 ){
			for( pos=from; pos<=to; pos++ ){
				mask[pos]=b8;
			}
		}
		//assert( to!=-1 );
	}
	
	return this;
}
BMPP* BMPP::Inner_( BIT_8 *ground,BIT_8 *tmp,INT_32 loop,INT_32 flag )	{
	int r,c,pos,width=si.wth,height=si.hei,j,pos_j,nPixel=si.Pixel();
	INT_32 neibor_3[9]={-width-1,-width,-width+1,-1,0,1,width-1,width,width+1};
	int isContain=0;

	for( INT_32 l=0; l<loop; l++ ){
		memset( tmp,0x0,sizeof(BIT_8)*nPixel );
		for( r = 1; r< height-1; r++ )	{
		for( c = 1; c< width-1; c++ )	{
			pos = G_RC2POS( r,c,width);
			if( ground[pos]!=1 )		continue;
			if( ground[pos-1]==0|| ground[pos+1]==0 || mask[pos+width]==0 || mask[pos-width]==0 )
				tmp[pos] = 1;
		}
		}
		for( pos=0;pos<nPixel;pos++ ){
			if( ground[pos]!=1 )		continue;
			if( tmp[pos]==1)		ground[pos]=0;
		}
	//	SaveMask( "Inner_"+to_string(l)+"_.bmp",ground,MSK_BLACK );
	}
//	CountII( 0x0 );
	return this;
}

BMPP* BMPP::Erose_( STRUC_ELE *hSE,BIT_8 *ground,BIT_8 *tmp,INT_32 loop,INT_32 flag )	{
	int r,c,pos,width=si.wth,height=si.hei,j,pos_j,nPixel=si.Pixel();
	INT_32 neibor_3[9]={-width-1,-width,-width+1,-1,0,1,width-1,width,width+1};
	BIT_8 *se=hSE->mask;
	int isContain=0;

	for( INT_32 l=0; l<loop; l++ ){
		memset( tmp,0x0,sizeof(BIT_8)*nPixel );
		for( r = 1; r< height-1; r++ )	{
		for( c = 1; c< width-1; c++ )	{
			pos = G_RC2POS( r,c,width);
			isContain=1;
			for( j=0; j<9; j++ )	{
				pos_j = pos+neibor_3[j];
				if( ground[pos_j]!=1 && se[j]==1 )
				{ isContain=0;	break;	}
			}
			tmp[pos] = isContain;
		}
		}
	//	for( pos = width*height-1; pos >= 0; pos-- )	ground[pos] = tmp[pos];
		memcpy( ground,tmp,sizeof(BIT_8)*nPixel );
	//	SaveMask( "erose_"+to_string(l)+"_.bmp",ground,MSK_BLACK );
	}
//	CountII( 0x0 );
	return this;
}
BMPP* BMPP::Dilate_( STRUC_ELE *hSE,BIT_8 *ground,BIT_8 *tmp,INT_32 loop,INT_32 flag )	{	
	int r,c,z,pos,width=si.wth,height=si.hei,j,pos_j,nPixel=si.Pixel();
//	INT_32 _SUPP_r[]={-1,-1,-1,0,0,0,1,1,1},_SUPP_c[]={-1,0,1,-1,0,1,-1,0,1};
	INT_32 neibor_3[9]={-width-1,-width,-width+1,-1,0,1,width-1,width,width+1};
	BIT_8 *se=hSE->mask;
	for( INT_32 l=0; l<loop; l++ ){
		memset( tmp,0x0,sizeof(BIT_8)*nPixel );
		for( r = 1; r< height-1; r++ )	{
		for( c = 1; c< width-1; c++ )	{
			pos = G_RC2POS( r,c,width);
			z = 0;
			for( j=0; j<9; j++ )	{
				pos_j = pos+neibor_3[j];
				z += ground[pos_j]*se[j];
			}
			tmp[pos] =  z>0 ? 1 : 0;
		}
		}
//		for( pos = width*height-1; pos >= 0; pos-- )		ground[pos] = tmp[pos];
		memcpy( ground,tmp,sizeof(BIT_8)*nPixel );
	//	SaveMask( "dilate_"+to_string(l)+"_.bmp",ground,MSK_BLACK );
	}
//	CountII( 0x0 );

	return this;
}

BMPP* BMPP::Open_( STRUC_ELE *hSE,BIT_8 *ground,BIT_8 *tmp,INT_32 loop,INT_32 flag )	{
	assert( tmp!=nullptr );		
	for( INT_32 i = 0; i < loop; i++ )	Erose_( hSE,ground,tmp,loop,flag );
	for( INT_32 i = 0; i < loop; i++ )	Dilate_( hSE,ground,tmp,loop,flag );
//	SaveMask( "_open_.bmp",ground,MSK_BLACK );
	return this;
};
BMPP* BMPP::Close_( STRUC_ELE *hSE,BIT_8 *ground,BIT_8 *tmp,INT_32 loop,INT_32 flag )	{	
	assert( tmp!=nullptr );		
	for( INT_32 i = 0; i < loop; i++ )	Dilate_( hSE,ground,tmp,loop,flag );
	for( INT_32 i = 0; i < loop; i++ )	Erose_( hSE,ground,tmp,loop,flag );
//	SaveMask( "_close_.bmp",ground,MSK_BLACK );
	return this;
};


//http://www.fourcc.org/fccyvrgb.php
void RGBtoYUVf(BIT_8 R,BIT_8 G,BIT_8 B,float *yuv ) {	//Julien suggested.
	float Y = 0.299*R + 0.587*G + 0.114*B;		yuv[0]=Y;
	yuv[1] = (B-Y)*0.565;
	yuv[2] = (R-Y)*0.713;
}
void GST_BMP::ColorSpace::ToRGB( int flag ){
	fomat = RGB;
	SHAPE_IMAGE *si=&(hBmp->si);
	int nPixel=si->Pixel(),r,c,wth=si->wth,hei=si->hei,pos=0;
	BIT_8 *cc1=ccc,*cc2=cc1+nPixel,*cc3=cc2+nPixel;
	BIT_8 cr,cg,cb;
	if( si->maps==1 )
	{	cc2=cc1;		cc3=cc1;	}
	for( r = 0; r < hei; r++ )	{
	for( c = 0; c < wth; c++,pos++ )	{
		hBmp->GetPixelColor( c,r,&cr,&cg,&cb );
		cc1[pos]=cr;	cc2[pos]=cg;	cc3[pos]=cb;		
	}
	}		
//	BMPP_save( &si,1,cc1,1 );	BMPP_save( &si,1,cc2,2 );	BMPP_save( &si,1,cc3,3 );
}
void GST_BMP::ColorSpace::ToLUV( int flag ){
	fomat = LUV;
	SHAPE_IMAGE *si=&(hBmp->si);
	float yuv[3];
	int nPixel=si->Pixel(),r,c,wth=si->wth,hei=si->hei,pos=0;
	BIT_8 *cc1=ccc,*cc2=cc1+nPixel,*cc3=cc2+nPixel;
//	BMPP_save( &si,1,cc1,1 );	BMPP_save( &si,1,cc2,2 );	BMPP_save( &si,1,cc3,3 );
	BIT_8 cr,cg,cb;
	if( si->maps==1 )
	{	cc2=cc1;		cc3=cc1;	}
	for( r = 0; r < hei; r++ )	{
	for( c = 0; c < wth; c++,pos++ )	{
		hBmp->GetPixelColor( c,r,&cr,&cg,&cb );
		cc1[pos]=cr;	cc2[pos]=cg;	cc3[pos]=cb;
		if( si->maps>1 ){
			RGBtoYUVf( cr,cg,cb,yuv );	
			yuv[1]+=128;			yuv[2]+=128;
			assert( yuv[1]>=0.0 && yuv[1]<256 && yuv[2]>=0.0 && yuv[2]<256 );
			cc1[pos]=yuv[0];	cc2[pos]=yuv[1];	cc3[pos]=yuv[2];
		}
	}
	}		
//	BMPP_save( &si,1,cc1,1 );	BMPP_save( &si,1,cc2,2 );	BMPP_save( &si,1,cc3,3 );
}
bool GST_BMP::ColorSpace::To( FORMAT format,int flag ){
	if( format==fomat )
		return true;
	GST_VERIFY( hBmp!=nullptr,"GST_BMP::ColorSpace::hBmp is 0" );
	if( format!=CF_0 && ccc==nullptr )		
		ccc=new BIT_8[hBmp->si.Count()]();
	switch( format ){
	case CF_0:
		if( ccc!=nullptr )	
		{	delete[] ccc;	ccc=nullptr; }
		fomat = CF_0;
		break;
	case LUV:
		ToLUV( flag );
		break;
	case RGB:
		ToRGB( flag );
		break;
	default:
		GST_THROW( "GST_BMP::ColorSpace::To format is XXX" );
	}
	return this;
}
BIT_8 *GST_BMP::ColorSpace::Channel( CHANEL chan,int flag ){
	GST_VERIFY( hBmp!=nullptr,"GST_BMP::ColorSpace::hBmp is 0" );
	SHAPE_IMAGE *si=&(hBmp->si);
	int ld=si->Pixel();
	switch( chan ){
	case GRAY:
		To( LUV,flag );
		return ccc;	
		break;
	case G_NORMAL:
		hBmp->Normalise( HISTO_EQUAL,0 );
		return ccc;	
		break;
	case R:		case G:			case B:
		To( RGB,flag );
		return ccc+ld*(chan-R);	
		break;
	case L:		case U:			case V:
		To( LUV,flag );
		return ccc+ld*(chan-L);	
		break;
	default:
		GST_THROW( "GST_BMP::ColorSpace::Channel chan is ..." );
		break;
	}
};
//似乎有问题	建议采用http://nomis80.org/ctmf.html
void ctmf(    const BIT_8* const src, BIT_8* const dst,const int width, const int height,const int src_step, const int dst_step,
        const int radius, const int channels, long unsigned int memsize=512*1024 );
//radius - The kernel will be a 2*r+1 by 2*r+1 square.
GST_BMP* GST_BMP::Median( BIT_8 *gray,int radius,int flag )	{
	int r,c,pos,M=si.hei,N=si.wth,i,pos_i,hash[256],stp=0,nz;
	INT_32 neibor_3[9]={-N-1,-N,-N+1,-1,0,1,N-1,N,N+1};
	BIT_8 g,g_0,g_1,*gray_M=new BIT_8[M*N],G_0,G_1;

	for( i=0; i<256; i++ )	hash[i]=stp;
	G_0=255,				G_1=0;
	stp++;
	for( r = 1; r < M-1; r++ )	{
	for( c = 1; c < N-1; c++ )	{
		pos=G_RC2POS( r,c,N );
		g_0=255,				g_1=0;
		for( i = 0; i < 9; i++ )	{
			pos_i = pos+neibor_3[i];	ASSERT( pos_i>=0 && pos_i<M*N );
			g = gray[pos_i];
			if( hash[g]<stp )
				hash[g]=stp;
			else
				hash[g]++;
			g_0 = MIN( g_0,g );			g_1 = MAX( g_1,g );
		}
		nz = 0;
		for( i = g_0; i <= g_1; i++ )	{
			if( hash[i]<stp )
				continue;
			nz += hash[i]-stp+1;
			if( nz>=5 )	{
				gray_M[pos] = i;
				G_0 = MIN( G_0,i );			G_1 = MAX( G_1,i );
				break;
			}
		}
//		ASSERT( nz==9 );
		stp += 10;
	}
	}
	memcpy( gray,gray_M,sizeof(BIT_8)*M*N );
	delete[] gray_M;
	return this;
}

GST_BMP* GST_BMP::HistoEqual( BIT_8 *gray,INT_32 flag )	{
    int histogram[256],map[256],equalize_map[256],nPixel=si.Pixel();
    int i, j,val,low,high;

    memset( &histogram,0x0, sizeof(int)*256 );
    memset( &map,0x0, sizeof(int)*256 );
    memset( &equalize_map,0x0, sizeof(int)*256 );

    for( i = nPixel-1; i>=0; i-- )	{
		val = gray[i];
		histogram[val]++;
	}
	j = 0;
	for(i=0; i <= 255; i++){
		j += histogram[i];
		map[i] = j;
	}
	// equalize
	low = map[0];	high = map[255];
	if (low == high)
		return this;
	for( i = 0; i <= 255; i++ ){
		equalize_map[i] = (unsigned int)((((double)( map[i] - low ) ) * 255) / ( high - low ) );
	}
	for( i = nPixel-1; i>=0; i-- )	{// stretch the histogram
		val = gray[i];
		gray[i] =(BIT_8)equalize_map[val];
	}
	return this;
}

GST_BMP* GST_BMP::Illumi( BIT_8 *gray,INT_32 flag )	{
    int i, j,M=si.hei,N=si.wth,MN=M*N,nPixel=si.Pixel();
	BIT_8 g;
	float *I_gray=nullptr,I_3,I_1,I_2,I_min=FLT_MAX,I_max=-FLT_MAX,I_avg=0,kv=1.0,mv,alpha=1.0,avg=0.0,s;
	double I_delta=0.0;

	for( i = 0; i < MN; i++ )	{	avg+=gray[i];	}
	avg /= MN;
	if( avg<1.0 )		return this;
	I_gray=new float[nPixel];
	for( i = 0; i < MN; i++ )	{
		g = gray[i];
		I_1 = log( 1.0+g );
		kv = g<=60 ? 7 : g<=200 ? (g-60)/70.0+7 : (g-200)/55.0+9;
		mv = kv*g/(g+alpha*avg);
		I_2 = (2.0/(1.0+exp(-mv))-1)*255-g;
		I_3 = 0.5*I_1+0.8*I_2+1.2*g;
		I_gray[i]=I_3;
		I_min=MIN(I_min,I_3);		I_max=MAX(I_max,I_3);
		I_avg+=I_3;					I_delta += I_3*I_3;
	}
	I_avg /= MN;
	I_delta = sqrt( I_delta/MN-I_avg*I_avg );
	I_min = I_avg-3.0*I_delta;			I_max = I_avg+3.0*I_delta;
	s = 255.0/(I_max-I_min);
	for( i = 0; i < MN; i++ )	{
		if( I_gray[i]<=I_min )
			gray[i]=0;
		else if( I_gray[i]>=I_max ) 
			gray[i]=255;
		else
			gray[i] = (BIT_8)((I_gray[i]-I_min)*s);
	}
	delete[] I_gray;
	return this;
}

/*
	ע�� 
	1	s_i,X_i�Ĵ�С��(M+bo*2+1)(N+bo*2+1)
	2   TEMP[2*MN]
	3   ����mean==value
*/
INT_32 GE_IMAGE_meanbox( INT_32 M,INT_32 N,float *value,INT_32 bo,float *mean,float *TEMP,INT_32 flag )	{
	INT_32 pos,r,c,nz,r0,r1,c0,c1,boM=M+bo*2+1,boN=N+bo*2+1,ldD=(M+2*bo+2)*(N+2*bo+2);
	INT_32 N4[4]={G_RC2POS(-bo-1,-bo-1,boN),G_RC2POS(bo,bo,boN),G_RC2POS(bo,-bo-1,boN),G_RC2POS(-bo-1,bo,boN)};
	float *s_i=TEMP,*X_i=TEMP+ldD,a;
	s_i[0] = 0;		
	pos = 0;
	for( c = 0; c < boN; c++ )	{		//��һ�� ��Ϊ0
		s_i[c] = 0 ;		X_i[c] = 0;
	}
	for( r = 1; r < boM; r++ )	{
		pos = G_RC2POS( r,0,boN );
		s_i[pos] = s_i[pos-boN];		X_i[pos] = s_i[pos];
		for( c = 1; c < boN; c++ )	{
			r1 = r-bo-1;		c1=c-bo-1;
			if( r1<0 )		r1=-r1;
			if( r1>=M )		r1=2*(M-1)-r1;
			if( c1<0 )		c1=-c1;
			if( c1>=N )		c1=2*(N-1)-c1;
			a = value[G_RC2POS(r1,c1,N)];
			pos++;
			s_i[pos] = s_i[pos-boN]+a;			X_i[pos] = X_i[(pos-1)]+s_i[pos];
		}
	}
	ASSERT( pos==boM*boN-1 );

	nz=0;
	a = 1.0f/(2*bo+1)/(2*bo+1);
	for( r = bo+1; r < M+bo+1; r++ )	{
	for( c = bo+1; c < N+bo+1; c++ )	{
	/*	r0=MAX(0,r-bo-1);		r1=MIN(boM-1,r+bo);
		c0=MAX(0,c-bo-1);		c1=MIN(boN-1,c+bo);
		a=(r1-r0)*(c1-c0);
		ASSERT( a == (2*bo+1)*(2*bo+1) );
		mean[nz++]=(X_i[G_RC2POS(r0,c0,boN)]+X_i[G_RC2POS(r1,c1,boN)]-X_i[G_RC2POS(r0,c1,boN)]-X_i[G_RC2POS(r1,c0,boN)])*a;*/
		pos = G_RC2POS(r,c,boN);
		mean[nz++]=(X_i[pos+N4[0]]+X_i[pos+N4[1]]-X_i[pos+N4[2]]-X_i[pos+N4[3]])*a;
	}
	}
	ASSERT( nz==M*N );
	
	return 0x0;
}
/*
	float[MN*6]
*/
GST_BMP* GST_BMP::Guide( BIT_8 *gray,INT_32 bo,float epsi,float *S_TEMP,INT_32 flag )	{
	int pos,M=si.hei,N=si.wth,MN=M*N;
	INT_32 ldX=M*N, ldD=(M+2*bo+2)*(N+2*bo+2);
	BIT_8 g;
	float *meanI=S_TEMP+2*ldD,*meanI2=meanI+ldX,*gA=meanI2+ldX,*gB=gA+ldX;
	float a,b,cov,var,r_0,r_1,a_sum=0,a2_sum=0,a_delta;

	for( pos=0; pos<MN; pos++ )	{
		a = gray[pos]/255.0f;
		meanI[pos]=	a;	meanI2[pos]=a*a;
	}
	GE_IMAGE_meanbox( M,N,meanI,bo,meanI,S_TEMP,0x0 );
	GE_IMAGE_meanbox( M,N,meanI2,bo,meanI2,S_TEMP,0x0 );

	for( pos=0; pos<MN; pos++ )	{
		var = meanI2[pos]-meanI[pos]*meanI[pos];
		cov = var;
		gA[pos]=cov/(var+epsi);		gB[pos]=meanI[pos]-gA[pos]*meanI[pos];
	}
	GE_IMAGE_meanbox( M,N,gA,bo,gA,S_TEMP,0x0 );
	GE_IMAGE_meanbox( M,N,gB,bo,gB,S_TEMP,0x0 );
	
	r_0=255;				r_1=-255;
	for( pos=0; pos<MN; pos++ )	{
	//	if( pos==16701 )
	//		pos = 16701;
		a = (gA[pos]*gray[pos]+255*gB[pos]);		//ASSERT( a>=0.0 && a<256 );
		//b = gray[pos];
		//gA[pos] = exp(log(b)-log(a));
		if( a<=0.0 )	{
			gA[pos] = 0.0;
		}else	{
			gA[pos] = gray[pos]/a;
			//gA[pos] = sqrt(gray[pos]/a);
			r_0 = MIN( r_0,gA[pos] );			r_1 = MAX( r_1,gA[pos] );
		}
		g = (BIT_8)(a);
		gray[pos] = g;
	}

/*	if( BIT_TEST( flag,GEI_GUIDE_RETINEX) )	{
		for( pos=0; pos<MN; pos++ )	{
			a = (gA[pos]-r_0)/(r_1-r_0);
			gray[pos] = (BIT_8)(255*a);
		}
	}*/
	return this;
}

GST_BMP* GST_BMP::Contrast( BIT_8 *cc,INT_32 flag ){
	int nPixel=si.Pixel( );
	float *S_TEMP=new float[nPixel*12],epsi=0.01;
	BMPP_save( &si,1,cc,10 );
	if( 0 ){
		Median(cc);				BMPP_save( &si,1,cc,11 );
		HistoEqual( cc );		//BMPP_save( &si,1,cc,10 );
	}
//	Illumi( cc );
	Guide( cc,1,epsi,S_TEMP);
	delete [] S_TEMP;
//	BMPP_save( &si,1,cc,12 );
//	HistoEqual( cc );		BMPP_save( &si,1,cc,13 );
	return this;
}
GST_BMP *GST_BMP::Normalise( NORMAL alg,int chan,int flag ){
	int nMap=si.maps,nPixel=si.Pixel(),i;
	BIT_8 *cc=space.Channel(GRAY);
	for( i=0;i<nMap;i++,cc+=nPixel ){
		if( chan>=0 && chan<nMap )
		{	if( i!=chan )	continue;	}
		switch( alg ){
		case HISTO_EQUAL:
			HistoEqual( cc );
			break;
		case CONTRAST:
			Contrast( cc );
			break;
		}
	}
	return this;
}