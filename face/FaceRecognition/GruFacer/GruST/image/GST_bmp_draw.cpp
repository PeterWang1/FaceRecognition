#include "GST_bitmap.hpp"
#include <random>
#include <time.h>
#include <thread>

#include <assert.h>
#include <math.h>
#include <string.h>
#include "FreeImage.h"

#define ToFIB(fib,ptr)	FIBITMAP *fib=static_cast<FIBITMAP *>(ptr);		GST_VERIFY(fib!=nullptr,"ToFIB is 0");	

using namespace Grusoft;

//�����BMP_DRAW( SHAPE_IMAGE *si,int type,int flag )���ô���
BMP_DRAW::BMP_DRAW( GST_BMP *hSrc,RoCLS*sub,int type,int flag ): GST_BMP( hSrc,0x0,sub,flag ){
#ifdef WIN32
	ToFIB( fib,hib );
	bits = FreeImage_GetBits(fib);
	bmi = FreeImage_GetInfo(fib);
	int wth=Width(),hei=Height();
	RGBQUAD white={0xFF,0xFF,0xFF,0};
	nColors = FreeImage_GetColorsUsed(fib);
	pDC = ::GetDC(0);
	tmpDC=CreateCompatibleDC(pDC);		//create a temporary dc in memory.
//create a new bitmap and select it in the memory dc
	tmpBmp=CreateDIBSection(pDC,bmi,DIB_RGB_COLORS,(void**)&pbase,0,0);
	HGDIOBJ oldObj=SelectObject(tmpDC,tmpBmp);
//draw the background
	SetStretchBltMode(tmpDC, COLORONCOLOR);	
	StretchDIBits(tmpDC, 0, 0, wth, hei, 0, 0, wth, hei,bits, bmi, DIB_RGB_COLORS, SRCCOPY);
//		SelectObject(tmpDC,oldObj);
	SetBkMode(tmpDC,TRANSPARENT);
	CrateFont( );
#else
	throw "BMP_DRAW::BMP_DRAW is ...";
#endif
//	this->Save( "F:\\GiFace\\trace\\1.bmp" );
}

BMP_DRAW::BMP_DRAW( SHAPE_IMAGE *si,int type,int flag ) : GST_BMP(si,type,flag ){
#ifdef WIN32
	ToFIB( fib,hib );
	bits = FreeImage_GetBits(fib);
	bmi = FreeImage_GetInfo(fib);
	int wth=Width(),hei=Height();
	RGBQUAD white={0xFF,0xFF,0xFF,0};
	FreeImage_FillBackground( fib,&white );
	nColors = FreeImage_GetColorsUsed(fib);
	pDC = ::GetDC(0);
	tmpDC=CreateCompatibleDC(pDC);		//create a temporary dc in memory.
//create a new bitmap and select it in the memory dc
	tmpBmp=CreateDIBSection(pDC,bmi,DIB_RGB_COLORS,(void**)&pbase,0,0);
	HGDIOBJ oldObj=SelectObject(tmpDC,tmpBmp);
//draw the background
	SetStretchBltMode(tmpDC, COLORONCOLOR);	
	StretchDIBits(tmpDC, 0, 0, wth, hei, 0, 0, wth, hei,bits, bmi, DIB_RGB_COLORS, SRCCOPY);
//		SelectObject(tmpDC,oldObj);
	SetBkMode(tmpDC,TRANSPARENT);
//			wchar_t font[]=_T("Times New Roman"),*wTitle=GST_util::CHAR2W(sTitle.c_str());

	CrateFont( );
#else
	throw "BMP_DRAW::BMP_DRAW is ...";
#endif
}

BMP_DRAW::~BMP_DRAW( )	{
#ifdef WIN32
	if( tmpDC!=NULL )	DeleteDC(tmpDC);
	if( pDC!=NULL )		ReleaseDC(NULL,pDC);
	if( hFont!=NULL)	DeleteObject(hFont);
#endif
}
void BMP_DRAW::CrateFont( int cH, int cW )	{
#ifdef WIN32
//	hFont = CreateFont(cH,cW, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, L"MONOSPACE");
	hFont = CreateFont(cH,cW, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0);
	SelectObject(tmpDC, hFont);
#endif
}
	
void BMP_DRAW::ClearBack( int flag ){
#ifdef WIN32
	ToFIB( fib,hib );
	int wth=FreeImage_GetWidth(fib),hei=FreeImage_GetHeight(fib);
	RGBQUAD white={0xFF,0xFF,0xFF,0};		
	FreeImage_FillBackground( fib,&white );
	StretchDIBits(tmpDC, 0, 0, wth, hei, 0, 0, wth, hei,bits, bmi, DIB_RGB_COLORS, SRCCOPY);
#else
	throw "BMP_DRAW::ClearBack is ...";
#endif
}

bool BMP_DRAW::Flush( )	{
#ifdef WIN32
	ToFIB( fib,hib );
	int hei=Height();
	GetDIBits(pDC,tmpBmp,0,hei,bits,	bmi,DIB_RGB_COLORS	);
	FreeImage_GetInfoHeader(fib)->biClrUsed = nColors;
	FreeImage_GetInfoHeader(fib)->biClrImportant = nColors;
	return true;
#else
	throw "BMP_DRAW::ClearBack is ...";
#endif
}

bool BMP_DRAW::Save( const char*sPath,bool isX,int flag )	{
#ifdef WIN32
try{
	ToFIB( fib,hib );
	int hei=Height();
	GetDIBits(pDC,tmpBmp,0,hei,bits,	bmi,DIB_RGB_COLORS	);
	FreeImage_GetInfoHeader(fib)->biClrUsed = nColors;
	FreeImage_GetInfoHeader(fib)->biClrImportant = nColors;
	FREE_IMAGE_FORMAT fif=FIF_JPEG;
	if( stricmp(sPath+strlen(sPath)-3,"bmp")==0){
		fif=FIF_BMP;
	}
	bool bRet=FreeImage_Save( fif,fib,sPath )==TRUE;		
	return bRet;
}catch( ... ){
	return false;
}
#else
	throw "BMP_DRAW::Save is ...";
#endif
}

bool BMP_DRAW::SetPixelColor( unsigned int c, unsigned int r, BIT_8 cr,BIT_8 cg,BIT_8 cb,void *value ){
#ifdef WIN32
	if( !G_RC_VALID(c,0,si.wth-1) || !G_RC_VALID(r,0,si.hei-1) )
		return false;
	COLORREF color=RGB(cr,cg,cb);
	if( value!=nullptr ){
		;//color=(RGBQUAD *)value ;
	}
	::SetPixel(tmpDC,c,r,color );
	return true;
#else
	throw "BMP_DRAW::Save is ...";
#endif
}
void BMP_DRAW::BITBlt( const GST_BMP *hbmp,const RoCLS &box,int flag ){
	GST_THROW( "BMP_DRAW::BITBlt is ..." );
}
void BMP_DRAW::Text( int x, int y, const char* text, CCCx color,int flag ){
#ifdef WIN32
	int len=strlen(text);
	SetTextColor(tmpDC,RGB(CCC_1(color),CCC_2(color),CCC_3(color)));
	RECT pos = {x,y,0,0};
	DrawTextA(tmpDC,text,len,&pos,DT_CALCRECT);		
	DrawTextA(tmpDC,text,len,&pos,0);
	if( 0 ){
		Save( "F:\\GiSeek\\trace\\Dataset\\1.bmp",true );
	}
#endif
}

void BMP_DRAW::Line( int32_t StartX, int32_t EndX, int32_t StartY, int32_t EndY, CCCx color,int flag){
#ifdef WIN32
	int hei=Height();
	HPEN hPen=CreatePen( PS_SOLID,0,RGB(CCC_1(color),CCC_2(color),CCC_3(color)) );
	SelectObject(tmpDC,hPen);
	MoveToEx( tmpDC,StartX,hei-StartY,LPPOINT(NULL) );		LineTo( tmpDC,EndX,hei-EndY );
	DeleteObject( hPen );
#endif
}
void BMP_DRAW::Rect( int32_t StartX, int32_t EndX, int32_t StartY, int32_t EndY, CCCx color,int flag){
#ifdef WIN32
	int hei=Height();
	HBRUSH hBrsuh=CreateSolidBrush( RGB(CCC_1(color),CCC_2(color),CCC_3(color)) );
	RECT rc;
	rc.left=StartX,		rc.right=EndX,		rc.bottom=hei-StartY,	rc.top=hei-EndY;
	FillRect( tmpDC,&rc,hBrsuh );
	DeleteObject( hBrsuh );
#endif
}

bool BMP_DRAW::SaveText( const wchar_t* sPath,const wchar_t *sText, CCCx color,int flag )	{
#ifdef WIN32
	ToFIB( fib,hib );
	int len=wcslen(sText),wth=Width(),hei=Height(),gap=3;
	if( len==0 )		return false;
	SetTextColor(tmpDC,RGB(CCC_1(color),CCC_2(color),CCC_3(color)));
//	SetBkMode( tmpDC,TRANSPARENT );
	RECT rc = {gap,gap,0,0};
	DrawText(tmpDC,sText,len,&rc,DT_CALCRECT);		
	if( wth<rc.right+gap || hei<rc.bottom+gap ){
		//Resize( wth,hei );
	}
	wth=Width(),	hei=Height();
	assert( wth>=rc.right+gap && hei>=rc.bottom+gap);
	ClearBack( );		//all in white
	DrawText(tmpDC,sText,len,&rc,0);
	GetDIBits(pDC,tmpBmp,0,hei,bits,	bmi,DIB_RGB_COLORS	);
	FreeImage_GetInfoHeader(fib)->biClrUsed = nColors;
	FreeImage_GetInfoHeader(fib)->biClrImportant = nColors;

	FIBITMAP *sub=FreeImage_Copy( fib,0,0,rc.right+gap,rc.bottom+gap );
	assert( sub!=nullptr );
	FREE_IMAGE_FORMAT fif=FIF_BMP;	//FIF_JPEG
	bool bRet=FreeImage_SaveU( fif,sub,sPath )==TRUE;
	FreeImage_Unload( sub );
	return bRet;
#else
	throw "BMP_DRAW::SaveText is ...";
#endif
}

bool PICS_N::PlotNails( string sPath,SHAPE_IMAGE siNail,int ld,bool isClear,int flag ){
	int	nBmp=nails.size( );		
	int j_1=ld>0 ? ld : (int)sqrt(nBmp*1.0),i_1=(int)ceil(nBmp*1.0/j_1),i,j,cur,rB,cB,r,c;
	if( magi_w<=0 )
	{	magi_w=nBmp==1 ? 0 : 1;		magi_h=magi_w;	}
	else{
		magi_h=20;		//for text info
	}
	wth=siNail.wth;						hei=siNail.hei;
	nWTH = j_1*(wth+magi_w);		nHEI=(hei+magi_h)*i_1+100;
	//arrInfo.resize(nBmp);
	SHAPE_IMAGE si(nWTH,nHEI,3);		si.bpp=24;
	hBMP = new BMP_DRAW(&si);

	BIT_8 grey,cr,cg,cb;		
	for( i = 0; i < i_1; i++ )	{		//����
	for( j = 0; j < j_1; j++ )	{		//����
		if( (cur = i*j_1+j)>=nBmp )
			break;
		GST_BMP* nail=nails[cur];
		if( nail==nullptr || nail->Width()==0 )
			continue;
		nail->ToSize( siNail.wth,siNail.hei,false );
		rB=i*(hei+magi_h),		cB=j*(wth+magi_w);
		for( r=0;r<siNail.hei;r++ )	{
		for( c=0;c<siNail.wth;c++ )	{
			//if( BIT_TEST(flag,DataSample::TRANS) )
			//{	c=pos/wth;		r=pos%wth;	}
			nail->GetPixelColor( c,siNail.hei-1-r,&cr,&cg,&cb );
			hBMP->SetPixelColor( c+cB,(rB+r),cr,cg,cb );
		}
		}
		if( arrInfo.size( )>cur && !arrInfo[cur].empty() ){
			hBMP->Text( cB,rB+hei,arrInfo[cur].c_str() );
		}
	}
	}
	if( sPath.size()>0 )
		hBMP->Save( sPath.c_str() );
	if( isClear ){
		for( vector<GST_BMP*>::iterator it=nails.begin();it!=nails.end();it++ )	delete *it;
		//for each ( GST_BMP* nail in nails )		delete nail;
		nails.clear( );
	}
	return true;
}
