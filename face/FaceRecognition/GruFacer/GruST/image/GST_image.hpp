#pragma once
/*
	1��ͼ�δ��������㣬Ӧ����������
	2���������漰�κε���ͼ�ο�
*/
#include <vector>
#include <string>
#include <stdint.h>
#include "../GST_def.h"
#include "GST_bitmap.hpp"
using namespace Grusoft;
//���ɫ��ͨ��
typedef BIT_32 CCCx;
#define C3_WHITE	0xFFFFFF
#define C3_BLACK	0
#define CCC3(r,g,b)   ( (CCCx)(((BIT_8)(r)|((CCCx)((BIT_8)(g))<<8))|(((CCCx)(BIT_8)(b))<<16)) )
#define CCC_1(rgb)      (BIT_8 (rgb) )
#define CCC_2(rgb)      (BIT_8 (((CCCx)(rgb)) >> 8) )
#define CCC_3(rgb)      (BIT_8 ((CCCx)(rgb)>>16) )

int SAVE_colorbmp(int no, int w, int h, BIT_32 * pdata,int x,int type );
int SAVE_graybmp(int no, int w, int h, BIT_8 * pdata,int x,int type );

template <typename T>
int SAVE_BMP_N(int nBmp,int no, int width, int h, T * pdata,int x,int type )	{	//"_no_x.bmp"
#ifdef 	ANDROID
	return -13;
#elif defined	__APPLE__
    return -14;
#else
	bool isColor=BIT_TEST( type,GST_PIXEL_3 );
	int j_1=PICS_N::gridX>0 ? PICS_N::gridX : (int)sqrt(nBmp*1.0);
	int i_1=(int)ceil(nBmp*1.0/j_1),i,j,pos,r,c,cur,ret=0;
	int magin=nBmp==1 ? 0 : 1;
//	magin=1;
	int patch = width*h,ldBmp=j_1*(width+magin),rB,cB;
	size_t nPixel=(size_t)(width+magin)*(h+magin)*i_1*j_1;
		
	double S_1=-DBL_MAX,S_0=DBL_MAX,s=1.0;
	T *pd;
	BIT_8 *tmp8=isColor ? nullptr : new BIT_8[nPixel]( ),cr,cg,cb;
	BIT_32 *tmp32=isColor ? new BIT_32[nPixel]( ) : nullptr;
	if( isColor )	{
	} else		{
		memset( tmp8,0xFF,sizeof(BIT_8)*nPixel );
	//	for( i = sizeof(BIT_8)*nPixel-1; i>=0; i-- )	tmp8[i]=0xFF;
	}

	for( i = 0; i < i_1; i++ )	{		//����
	for( j = 0; j < j_1; j++ )	{		//����
		if( (cur = i*j_1+j)>=nBmp )
			break;
		rB=i*(h+magin),		cB=j*(width+magin);
		S_1=-DBL_MAX,S_0=DBL_MAX;
		if( isColor )	{
			pd = pdata+cur*patch*3;			
			for( pos=0; pos<patch*3; pos++ )	{
				S_0 = MIN( S_0,pd[pos] );		S_1=MAX( S_1,pd[pos] );	
			}
		/*	for( pos=0; pos<patch; pos++ )			{S_0 = MIN( S_0,pd[pos] );		S_1=MAX( S_1,pd[pos] );	}
			for( pos=patch; pos<patch*2; pos++ )	{S_0 = MIN( S_0,pd[pos] );		S_1=MAX( S_1,pd[pos] );	}
			for( pos=patch*2; pos<patch*3; pos++ )	{S_0 = MIN( S_0,pd[pos] );		S_1=MAX( S_1,pd[pos] );	}*/
		} else		{
			pd = pdata+cur*patch;	
			for( pos=0; pos<patch; pos++ )	{
				S_0 = MIN( S_0,pd[pos] );		S_1=MAX( S_1,pd[pos] );	
			}
		}
		s = ( S_0==S_1 ) ? 1.0 : 1.0/(S_1-S_0);
	//	S_0=-1.0;		s=0.5;
		for( pos=0; pos<patch; pos++ )	{
			if( BIT_TEST(type,DataSample::TRANS) )
			{	c=pos/width;		r=pos%width;	}
			else
			{	r=pos/width;		c=pos%width;	}
			if( isColor )	{
				cr=(BIT_8)(255*(pd[pos]-S_0)*s);
				cg=(BIT_8)(255*(pd[patch+pos]-S_0)*s);
				cb=(BIT_8)(255*(pd[2*patch+pos]-S_0)*s);
				tmp32[(rB+r)*ldBmp+c+cB]=CCC3(cr,cg,cb);//(BIT_32)(0xff000000 | (cb<<16) | (cg<<8) | cr);
			} else			{
				tmp8[(rB+r)*ldBmp+c+cB]=(BIT_8)(255*(pd[pos]-S_0)*s);
			}
		}
	}
	}

	if( isColor )	{
		ret = SAVE_colorbmp( no,(width+magin)*j_1,(h+magin)*i_1,tmp32,x,type );
		delete[] tmp32;
	}else	{
		ret = SAVE_graybmp( no,(width+magin)*j_1,(h+magin)*i_1,tmp8,x,type );
		delete[] tmp8;
	}
	return ret;
#endif
}

template <typename T>
void _RGB2HSV_( T R,T G,T B,T *h,T *s,T *v )	{
	T m_1=MAX( R,MAX(G,B) ); 
	T m_0=MIN( R,MIN(G,B) );

	*v = m_1;
	if( m_1==0 )	{	*h=0;	*s=0;		return;	}
	*s = (T)(255*(m_1-m_0)/m_1);
	if( *s==0 )		{	*h=0;			return;	}
	
	if( R==m_1 )
		*h = 0+43*(G-B)/(m_1-m_0);  
	else if( G == m_1 )
		*h = 85+43*(B-R)/(m_1-m_0);  
	else	
		*h = 171+43*(R-G)/(m_1-m_0);  
//	if( *h<0 )		
//		*h+=255.0;	
}

#define LAB_HALF	126	
template <typename T>
void _RGB2LAB_( T R,T G,T B,T &L,T &la,T &lb){
	double r,g,b,X, Y, Z,fX,fY,fZ;
	r = R/255.0;		g = G/255.0;		b = B/255.0;
	r = r>0.04045 ? pow( (r+0.055)/1.055,2.4 ) : r /12.92;
	g = g>0.04045 ? pow( (g+0.055)/1.055,2.4 ) : g /12.92;
	b = b>0.04045 ? pow( (b+0.055)/1.055,2.4 ) : b /12.92;
	r *= 100;			g *= 100;			b *= 100;

    X = 0.412453*r + 0.357580*g + 0.180423*b;
    Y = 0.212671*r + 0.715160*g + 0.072169*b;
    Z = 0.019334*r + 0.119193*g + 0.950227*b; 
	fX = X/95.0456;		fY = Y/100;		fZ = Z/108.88754;	
//	ASSERT( fX<=1.0 && fY<=1.0 && fZ<=1.0 );

	fX = fX>0.008856 ? pow(fX,1.0/3.0) : (7.787*fX + 16.0/116.0);
 	fY = fY>0.008856 ? pow(fY,1.0/3.0) : (7.787*fY + 16.0/116.0);
	fZ = fZ>0.008856 ? pow(fZ,1.0/3.0) : (7.787*fZ + 16.0/116.0);
 
	L=116*fY-16; 
    X = 500.0*(fX - fY);				Y = 200.0*(fY - fZ);
//	ASSERT( L>=0 && L<=100 && X>=-LAB_HALF && X <=LAB_HALF && Y>=-LAB_HALF && Y <=LAB_HALF);
	la = (X+LAB_HALF);		lb = (Y+LAB_HALF);
}

//http://www.fourcc.org/fccyvrgb.php
template <typename T>
void YUVtoRGB(T Y,T u,T v,unsigned char &R,unsigned char &G,unsigned char &B) {
	int r,g,b;
	r=Y+1.403*v;		g=Y-0.344*u-0.714*v;		b=Y+1.770*u;		//Julien suggested.
/*//http://stackoverflow.com/questions/9325861/converting-yuv-rgbimage-processing-yuv-during-onpreviewframe-in-android
    float Yf = 1.164f*(Y) - 16.0f;
    r = (int)(Yf + 1.596f*v);
    g = (int)(Yf - 0.813f*v - 0.391f*u);
    b = (int)(Yf            + 2.018f*u);*/
	R = r>255? 255 : r<0 ? 0 : r;
	G = g>255? 255 : g<0 ? 0 : g;
	B = b>255? 255 : b<0 ? 0 : b;
}

//http://www.fourcc.org/fccyvrgb.php
template <typename T>
void RGBtoYUV(BIT_8 R,BIT_8 G,BIT_8 B,T &Y,T &u,T &v ) {	//Julien suggested.
	Y = 0.299*R + 0.587*G + 0.114*B;		
	u = (B-Y)*0.565;
	v = (R-Y)*0.713;
}
BIT_8 *FIB8_open( const char *sPath,int tW,int tH,int flag );




