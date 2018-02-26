#include "GST_image.hpp"
#include <random>
#include <time.h>
#include <thread>
#include <assert.h>
#include <math.h>
#include "FreeImage.h"
int PICS_N::gridX=-1;

#if defined (_MSC_VER)  // Visual studio
    #define thread_local __declspec( thread )
#elif defined (__GCC__) // GCC
    #define thread_local __thread
#endif
/* Thread-safe function that returns a random number between min and max (inclusive).
This function takes ~142% the time that calling rand() would take. For this extra
cost you get a better uniform distribution and thread-safety. */

static thread_local std::mt19937* generator = nullptr;
void UniformInt_init( int seed_0 )	{
    if (generator!=nullptr) 
	{	delete generator;		generator=nullptr;	}

	clock_t t0=clock( );
	unsigned int seed=seed_0==0 ? t0 : seed_0;
	generator = new std::mt19937(seed);
	printf( "\\|/ GST_image::mt19937@%d thread=%d,time=%d\n",seed,std::this_thread::get_id(),t0 );
}
int UniformInt(const int & min, const int & max ) {
//	return (min+max)/2;		//�����ڲ���		9/24/2015
    if (generator==nullptr ) {
		throw "UniformInt:generator is ...";
	}
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(*generator);
}

#include "../util/GST_dataset.hpp"
//#include <omp.h>


void DATASET_cr::ToHSV( int flag ){
	printf( "\n>>>>>>DATASET_cr::ToHSV...\n" );
	BIT_8 *cur=X,*Xr=NULL, *Xg=NULL, *Xb=NULL,index,h,s,v;
	int nSamp = nSample(),samp,i,nz=ldX/3,r,c;
	for( samp=0; samp<nSamp; samp++,cur+=ldX )	{
		Xr=cur,			Xg=Xr+ldX/3,		Xb=Xg+ldX/3;
		for ( i = 0; i < nz; i++ )	{
			_RGB2HSV_( Xr[i],Xg[i],Xb[i],&h,&s,&v );
			Xr[i]=h,		Xg[i]=s,		Xb[i]=v;
		}
	}
}
int DATASET_cr::ToBmp(int epoch, int _x, int flag )	{
	int n = nSample();
	if( n>0 )	
		return	SAVE_BMP_N(n, epoch, width(), height(), X, n, type);
	return -1;
}

BIT_8 *FIB8_open( const char *sPath,int tW,int tH,int flag ){
	int r,c,wth,hei,nz=0,ldX=tW*tH*3;
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(sPath);
//	if( fif==FIF_UNKNOWN )		fif=FIF_JPEG;
	FIBITMAP *fib=FreeImage_Load( fif,sPath ),*dib=fib;
	if( fib==nullptr ){
		for( fif=FIF_BMP;fif<=FIF_RAW;fif=(FREE_IMAGE_FORMAT)(fif+1) ){
			fib=FreeImage_Load( fif,sPath );
			if(fib!=nullptr)	
				break;
		}
		if( fib==nullptr )
			throw "FIB8_open:sPath is XXX!";
	}
	int bpp=FreeImage_GetBPP( fib );
	if( bpp>32 ){
		FIBITMAP *fib32 = FreeImage_ConvertTo32Bits( fib );
		FreeImage_Unload( fib );		fib=fib32;
	}
	dib=fib;
	wth=FreeImage_GetWidth(dib),		hei=FreeImage_GetHeight(dib);
	assert( wth==tW && hei==tH );
	BIT_8 *X=new BIT_8[ldX];
	BIT_8 *Xr=X, *Xg=Xr+ldX/3, *Xb=Xg+ldX / 3,index;
	RGBQUAD cr;
	for (r = 0; r < tH; r++)	{
	for (c = 0; c < tW; c++)	{		
	//	RGBQUAD cr = img->GetPixelColor(c, (hIm-r) );		//BUG		6/30/2015		cys
		nz=G_RC2POS(r,c,tW);
		if(  FreeImage_GetPixelColor( dib,c,tH-1-r,&cr ) )	{
			Xr[nz] = cr.rgbRed;		Xg[nz] = cr.rgbGreen;			Xb[nz] = cr.rgbBlue;
		}else if( FreeImage_GetPixelIndex( dib,c,(tH-1-r),&index ) ){
			Xr[nz] = index;			Xg[nz] = index;					Xb[nz] = index;
		}else{
			throw "FIB8_open::Open:GetPixelColor is failed!";
		}
	}
	};
	if( dib!=fib ){		FreeImage_Unload(dib);		dib=nullptr;	}
	if( fib!=nullptr ){		FreeImage_Unload(fib);		fib=nullptr;	}
	return X;
}

void Image_Augment( FLOA*pic,SHAPE_IMAGE *si, IMAGE_AUG&aug,int flag=0x0 ){
	int wth=si->wth,hei=si->hei,map,r,c;		assert(si->maps==3);
	int ldM=si->Pixel( ),ldX=si->Count(),pos=0;
	FLOA *Xr=pic, *Xg = Xr + ldM, *Xb = Xg + ldM;
	FIBITMAP *fib=FreeImage_Allocate( wth,hei,24 ),*dib=nullptr;
	RGBQUAD cr;

//	SAVE_BMP_N( 1,1015,wth,hei,pic,1,GST_PIXEL_3 );
	for (r = 0; r < hei; r++)	{
	for (c = 0; c < wth; c++)	{		
		cr.rgbRed=Xr[pos];	cr.rgbGreen=Xg[pos];	cr.rgbBlue=Xb[pos];	pos++;	
		FreeImage_SetPixelColor( fib,c,hei-1-r,&cr ); 
	}
	}
//	FreeImage_Save( FIF_JPEG,fib,"F:\\GiSeek\\trace\\Dataset\\1016.jpg" );
	if( aug.ang!=0 ){
		dib = FreeImage_RotateEx(fib, aug.ang, 0, 0, wth/2, hei/2, FALSE);
		FreeImage_Unload( fib );		fib=dib;
	//	FreeImage_Save( FIF_JPEG,fib,"F:\\GiSeek\\trace\\1.jpg" );
	}
	#ifdef ANDROID
	dib = nullptr;
    #elif defined __APPLE__
    dib = nullptr;
	#else
	dib = FreeImage_RescaleRect( fib,aug.tW,aug.tH,aug.L,aug.T,aug.R,aug.B );
	#endif
	pos=0;
	for (r = 0; r < hei; r++)	{
	for (c = 0; c < wth; c++)	{		
	//	RGBQUAD cr = img->GetPixelColor(c, (hIm-r) );		//BUG		6/30/2015		cys
		if(  FreeImage_GetPixelColor( dib,c,hei-1-r,&cr ) )	{
			Xr[pos]=cr.rgbRed;	Xg[pos]=cr.rgbGreen;	Xb[pos]=cr.rgbBlue;	pos++;	
		}
	}
	}
//	SAVE_BMP_N( 1,1017,wth,hei,pic,1,GST_PIXEL_3 );
	FreeImage_Unload( fib );		
	if( dib!=fib )
	{	FreeImage_Unload( dib );	dib=nullptr;	}
}

bool DATASET_cr::ImportImage( int no, void *src,IMAGE_AUG&aug,int flag ){
#ifdef ANDROID
	return false;
#elif defined __APPLE__
    return false;
#else
	wchar_t *sPath=(wchar_t *)src;
//	wchar_t *sPath=_T("G:\\Ůװ\\i ����\\u=2949895703,663655533&fm=21&gp=0.jpg");
//	wchar_t *sPath=_T("G:\\Ůװ\\ţ������\\14dhcd_ie2dkmzugjtdqmzvgizdambqgiyde_640x960.gif_220x330.v1cDM.81.jpg");	//8λ���JPEG,��pallet
try{
	int len=_tcslen(sPath),ldw=width();
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeU(sPath);
	if( fif==FIF_UNKNOWN )		fif=FIF_JPEG;
//#pragma omp critical
{
	FIBITMAP *fib=FreeImage_LoadU( fif,sPath ),*dib=fib;
	if( fib==nullptr ){
		for( fif=FIF_BMP;fif<=FIF_RAW;fif=(FREE_IMAGE_FORMAT)(fif+1) ){
			fib=FreeImage_LoadU( fif,sPath );
			if(fib!=nullptr)	
				break;
		}
		if( fib==nullptr )
			throw "DATASET_cr::ImportImage is failed to load!";
	}
	RGBQUAD cr,*pallet=NULL;
	int bpp=FreeImage_GetBPP( fib );
	if( bpp>32 ){
		FIBITMAP *fib32 = FreeImage_ConvertTo32Bits( fib );
		FreeImage_Unload( fib );		fib=fib32;
	} 	
	if( si->maps==1 ){
		FIBITMAP *fib8 = FreeImage_ConvertToGreyscale( fib );
		FreeImage_Unload( fib );		fib=fib8;
	}else{
		
	}
	dib=fib;
	//pallet=FreeImage_GetPalette( dib );
	int wth=FreeImage_GetWidth(fib),hei=FreeImage_GetHeight(fib),r=hei/2, c=wth/2,dim,nz=0,left=0,top=0,ang;
	aug.Augment( wth,hei );
	if(aug.isRandom( ) && rand()%10==0 ){
	//if(aug.isRandom( ) && no%10==0 ){
	//	FreeImage_Save( FIF_JPEG,fib,"F:\\GiSeek\\trace\\0.jpg" );
		ang=rand()%2==1 ? -15:15;
		//ang=ang%2==1 ? -15:15;
		dib = FreeImage_RotateEx(fib, ang, 0, 0, wth/2, hei/2, FALSE);
		FreeImage_Unload( fib );		fib=dib;
	//	FreeImage_Save( FIF_JPEG,fib,"F:\\GiSeek\\trace\\1.jpg" );
	}
#ifdef ANDROID
		dib = nullptr;
#else
	dib = FreeImage_RescaleRect( fib,aug.tW,aug.tH,aug.L,aug.T,aug.R,aug.B );
#endif
	size_t off=ldX*(size_t)(no);		
	BIT_8 *Xr = X + off, *Xg = Xr + ldX / 3, *Xb = Xg + ldX / 3,index;
	int dW=(width()-aug.tW)/2,dH=(height()-aug.tH)/2;			assert( dW>=0 && dH>=0 );
	if( aug.isRandom( )/*align==IMAGE_AUG::IMAG_FLOAT || align==IMAGE_AUG::IMAG_MAXFLOAT*/ ){
//		dW = dW==0 ? 0:rand()%(dW*2);			dH = dH==0 ? 0:rand()%(dH*2);
//		dW = dW==0 ? 0:UniformInt(0,dW*2-1);			dH = dH==0 ? 0:UniformInt(0,dH*2-1);		//�ƺ����е�׼ȷ�ʸ�ߣ����ô��	9/24/2015
		dW = width()==aug.tW ? 0:UniformInt(0,width()-aug.tW);			dH = height()==aug.tH ? 0:UniformInt(0,height()-aug.tH);
		assert(aug.tW+dW>=0 && aug.tW+dW<=width() );
		assert(aug.tH+dH>=0 && aug.tH+dH<=height() );
	}
	if( bpp<32 )	{
		pallet=FreeImage_GetPalette( dib );
	}
	if( si->maps==1 ){
		for (r = 0; r < aug.tH; r++)	{
		for (c = 0; c < aug.tW; c++)	{		
			nz=G_RC2POS(r+dH,c+dW,ldw);
			if( FreeImage_GetPixelIndex( dib,c,(aug.tH-1-r),&index ) ){				
				Xr[nz] = index;			
			}else{	
				throw "DATASET_cr::ImportImage:GetPixelIndex is failed!";		
			}
		}
		};
	}else{
		for (r = 0; r < aug.tH; r++)	{
		for (c = 0; c < aug.tW; c++)	{		
			nz=G_RC2POS(r+dH,c+dW,ldw);
			if(  FreeImage_GetPixelColor( dib,c,(aug.tH-1-r),&cr ) )	{
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
			}
		}
		};
	}
	if( dib!=fib ){		FreeImage_Unload(dib);		dib=nullptr;	}
	if( fib!=nullptr ){		FreeImage_Unload(fib);		fib=nullptr;	}
}

	return true;
}catch( ... )	{
	_tprintf( _T("DATASET_cr::ImportImage failed at %s\n"),sPath );
	throw "DATASET_cr::ImportImage is XXX";
	return false;
}
#endif
}

int SAVE_colorbmp(int no, int w, int h, BIT_32 * pdata,int x,int type )	{
try{
	char sPath[256] = "\0",sDir[]="/storage/sdcard0/1_cys/";

	int ret=0x0,r,c,nz=0;
	switch( type )	{
	default:
		if( no<0 )
			sprintf( sPath,("%s%3d.jpg"),DataSample::sDumpFolder.c_str(),x );
		else
			sprintf( sPath,("%s%3d_%d.jpg"),DataSample::sDumpFolder.c_str(),no,x );
		break;
	}
	DataSample::sLastPicPath = sPath;
	RGBQUAD cr;
	FIBITMAP *fib=FreeImage_Allocate( w,h,24 );
	if( fib!=nullptr ){
		for (r = 0; r < h; r++)	{
		for (c = 0; c < w; c++,nz++)	{		
			cr.rgbRed=CCC_1(pdata[nz]);		cr.rgbGreen=CCC_2(pdata[nz]);		cr.rgbBlue=CCC_3(pdata[nz]);		
			FreeImage_SetPixelColor( fib,c,(h-1-r),&cr );	
		}
		}
		FreeImage_Save( FIF_JPEG,fib,sPath );		//FreeImage_Save( FIF_BMP,fib,sPath );		
		FreeImage_Unload( fib );
	}
END:
	return ret;
}catch( ... )	{
	throw "SAVE_colorbmp is XXX";
	return false;
}
}

int SAVE_graybmp(int no, int w, int h, BIT_8 * pdata,int x,int type ){
try{
	char sPath[256] = "\0",sDir[]="/storage/sdcard0/1_cys/";

	int ret=0x0,r,c,nz=0;
	switch( type )	{
	default:
		if( no<0 )
			sprintf( sPath,("%s%3d.bmp"),DataSample::sDumpFolder.c_str(),x );
		else
			sprintf( sPath,("%s%3d_%d.bmp"),DataSample::sDumpFolder.c_str(),no,x );
		break;
	}
	DataSample::sLastPicPath = sPath;
	FIBITMAP *fib=FreeImage_Allocate( w,h,8 );
	if( fib!=nullptr ){
		for (r = 0; r < h; r++)	{
		for (c = 0; c < w; c++,nz++)	{		
			FreeImage_SetPixelIndex( fib,c,(h-1-r),pdata+nz );	
		}
		}
		FreeImage_Save( FIF_BMP,fib,sPath );		FreeImage_Unload( fib );
	}
END:
	return ret;
}catch( ... )	{
	throw "SAVE_colorbmp is XXX";
	return false;
}
}

PICS_N::PICS_N( int nBm_,int wt_, int he_,int ma_,int x,int magw,int flag ) : hBMP(nullptr) ,maps(ma_),magi_w(magw) {
	if( nBm_<=0 || wt_<=0 || he_<=0 )
		throw "PICS_N::PICS_N nBmp is XXX";
	nBmp = nBm_;		wth=wt_;		hei=he_;
	j_1=x>0 ? x : (int)sqrt(nBmp*1.0);
	i_1=(int)ceil(nBmp*1.0/j_1);
	if( magi_w<=0 )
	{	magi_w=nBmp==1 ? 0 : 1;		magi_h=magi_w;	}
	else{
		magi_h=20;		//for text info
	}
	nWTH = j_1*(wth+magi_w);		nHEI=(hei+magi_h)*i_1+100;
	arrInfo.resize(nBmp);
	//hBMP = new FIB_DRAW(nWTH,nHEI,24);
	SHAPE_IMAGE si(nWTH,nHEI,3);		si.bpp=24;
	hBMP = new BMP_DRAW(&si);
}
PICS_N::~PICS_N( )	{
	if( hBMP!=nullptr )		delete hBMP;
/*	if( hDIB!=nullptr ){
		ToFIB( fib,hDIB );
		FreeImage_Unload( fib );		hDIB=nullptr;
	}*/
}

void PICS_N::SetText( int no,const char *format,... )	{	
	if( no<0 || no>=nBmp )
		return;
	va_list args;
	va_start( args, format );
	char buffer[1000];
#ifdef ANDROID
	sprintf( buffer,format,args );
#elif defined __APPLE__
    sprintf( buffer,format,args );
#else
	_vsnprintf( buffer,1000,format,args );
#endif
	va_end(args);
	arrInfo[no] = buffer;
}

int PICS_N::SaveF( const char *sPath,float * pdata,int flag )	{	//"_no_x.bmp"		
	int i,j,pos,r,c,cur,ret=0,nPixel=wth*hei,rB,cB,ldBmp=nWTH;//j_1*(wth+magin);
	double S_1=-DBL_MAX,S_0=DBL_MAX,s=1.0;
	float *pd,*pr=nullptr,*pg=nullptr,*pb=nullptr;
	BIT_8 grey,cr,cg,cb;		
	for( i = 0; i < i_1; i++ )	{		//����
	for( j = 0; j < j_1; j++ )	{		//����
		if( (cur = i*j_1+j)>=nBmp )
			break;
		rB=i*(hei+magi_h),		cB=j*(wth+magi_w);
		S_1=-DBL_MAX,S_0=DBL_MAX;
		pd = pdata+cur*nPixel*maps;		
		pr=pd,	pg=pr+wth*hei,	pb=pg+wth*hei;	
		for( pos=nPixel*maps-1; pos>=0; pos-- )	{	S_0 = MIN( S_0,pd[pos] );		S_1=MAX( S_1,pd[pos] );		}				
		s = ( S_0==S_1 ) ? 1.0 : 1.0/(S_1-S_0);
	//	S_0=-1.0;		s=0.5;
		for( pos=0; pos<nPixel; pos++ )	{
			if( BIT_TEST(flag,DataSample::TRANS) )
			{	c=pos/wth;		r=pos%wth;	}
			else
			{	r=pos/wth;		c=pos%wth;	}
			if( maps==1 ){
				grey=(BIT_8)(255*(pd[pos]-S_0)*s);
				cr=cg=cb=grey;
			}	else{
				cr=(BIT_8)(255*(pr[pos]-S_0)*s);
				cg=(BIT_8)(255*(pg[pos]-S_0)*s);
			//	cg=(BIT_8)(255*0.8*(pd[patch+pos]-S_0)*s);//����Ϊ�ζ���ɫ����
				cb=(BIT_8)(255*(pb[pos]-S_0)*s);
			}
			hBMP->SetPixelColor( c+cB,(rB+r),cr,cg,cb );
			//CCC_P( cr,cg,cb,(rB+r),c+cB );		
		}
		if( arrInfo.size( )>=cur && !arrInfo[cur].empty() ){
			hBMP->Text( cB,rB+hei,arrInfo[cur].c_str() );
		}
	}
	}
	if( sTitle.size()>0 )
		hBMP->Text( 0,nHEI-20,sTitle.c_str() );
	hBMP->Save( sPath );
	return ret;
}
/*
void PICS_N::CCC_P( BIT_8 cr,BIT_8 cg,BIT_8 cb,int r,int c,int flag ){
	ToFIB( fib,hDIB );
	RGBQUAD quad;
	quad.rgbRed=cr;		quad.rgbGreen=cg;		quad.rgbBlue=cb;	
	FreeImage_SetPixelColor( fib,c,(nHEI-1-r),&quad );
}
void PICS_N::Gray_P( BIT_8 gray,int r,int c,int flag ){
	ToFIB( fib,hDIB );
	FreeImage_SetPixelIndex(fib,c,(nHEI-1-r),&gray );
}*/

INT_32 GE_IMAGE_meanbox( INT_32 M,INT_32 N,float *value,INT_32 bo,float *mean,float *TEMP,INT_32 flag );
/*
	float[MN*6]
*/
INT_32 GE_IMAGE_guide( INT_32 no,INT_32 M,INT_32 N,float *gray,INT_32 bo,float epsi,float *S_TEMP,INT_32 flag )	{
	int pos,MN=M*N,ret=0x0;
	INT_32 ldX=M*N, ldD=(M+2*bo+2)*(N+2*bo+2);
	BIT_8 g;
	float *meanI=S_TEMP+2*ldD,*meanI2=meanI+ldX,*gA=meanI2+ldX,*gB=gA+ldX;
	float a,b,cov,var,r_0,r_1,a_sum=0,a2_sum=0,a_delta;
#ifdef WIN32
//	save_graybmp( hGei->no,N,M,gray,hGei->x,BMP_EYE_MORPHO1 );
#endif
	for( pos=0; pos<MN; pos++ )	{
		a = gray[pos]/255.0f;
		meanI[pos]=	a;	meanI2[pos]=a*a;
	}
	GE_IMAGE_meanbox( M,N,meanI,bo,meanI,S_TEMP,0x0 );
	GE_IMAGE_meanbox( M,N,meanI2,bo,meanI2,S_TEMP,0x0 );
#ifdef WIN32
//	for( pos=0; pos<MN; pos++ )		gray[pos]=(BIT_8)(255.0*meanI[pos]);
//	save_graybmp( no,N,M,gray,0,BMP_EYE_MORPHO2 );
#endif
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
//			a_sum+=a;		a2_sum+=a*a;
			gA[pos] = gray[pos]/a;
			//gA[pos] = sqrt(gray[pos]/a);
			r_0 = MIN( r_0,gA[pos] );			r_1 = MAX( r_1,gA[pos] );
		}
		g = (BIT_8)(a);
		gray[pos] = g;
	}
//	hGei->gray_0 = G_0,		hGei->gray_1=G_1;

	if( 1/*BIT_TEST( flag,GEI_GUIDE_RETINEX)*/ )	{
//		a_delta = a2_sum/MN-a_sum*a_sum/MN/MN;
		for( pos=0; pos<MN; pos++ )	{
			a = (gA[pos]-r_0)/(r_1-r_0);
			gray[pos] = (255*a);
		}
	}
#ifdef WIN32
//	save_graybmp( no,N,M,gray,0,BMP_EYE_MORPHO2 );
#endif
//	_DUMP_IF_( 0601,LOGP( "GE_IMAGE_guide{%x,%d},G_0=%d,G_1=%d",hGei,flag,G_0,G_1 ) );
	return ret;
}

