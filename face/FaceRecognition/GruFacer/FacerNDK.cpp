#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#ifdef _WINDOWS
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include "./GruST/GST_def.h"
#include "./GruST/image/GST_bitmap.hpp"
#include "FaceAlign_.h"

using namespace Grusoft;

int gd_level=1,gMehtod=0;
static double t_All=0.0,t_Face=0.0;

static char sDumpPath[]="/storage/sdcard0/1_cys/";
static char sPath[500];

static size_t Asset2Chars( AAssetManager* mgr,char* sFile,char **dst,int flag )  {
	AAsset* asset = AAssetManager_open(mgr, sFile, AASSET_MODE_UNKNOWN);
	if (NULL == asset) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "_ASSET_NOT_FOUND_");
		return 0;
	}

	size_t size = AAsset_getLength(asset);
	char* buffer = (char*) malloc (sizeof(char)*size);
	AAsset_read (asset,buffer,size);
	AAsset_close(asset);
	LOGP( "Load %s size=%d",sFile,size );
	*dst=buffer;
	return size;
}

extern "C" JNIEXPORT int JNICALL Java_com_sightsoft_hitrack_Facer_nYUV2BMP( JNIEnv *env, jclass clazz, jbyteArray yuvArr,jint width, jint height, jintArray pixArr,jint flag ){
    GST_TIC(tick);
    jbyte *pYUV = env->GetByteArrayElements(yuvArr, 0);
    jint *pixels = env->GetIntArrayElements( pixArr, 0);
    INT_32 r,c,pos,pos0=0,posY,posUV,fW=height,fH=width;
    BIT_8 *pfc,*pfU,*pfV,*YUV=(BIT_8 *)pYUV,*UV=YUV+width*height,Y0=0,U0=0,V0=0;
    int U,V,Yf,fr,fg,fb,cr,cg,cb;

    for (c = 0; c < fW; c++ ) {
        posY = c*width;		posUV=c/2*width;        //source2frame_T [800,600]=>[300,400]
        //pfc = frame+fW-1-c;        pfU = fU+fW-1-c;        pfV = fV+fW-1-c;
        pos=fW-1-c;
        for (r = 0; r < fH; r++,YUV++ ) {
            //(*(pfc+r*fW)) = (BIT_8)(lSource[pos]);
            //uv = (BIT_8*)(uvSource+posUV);            (*(pfU+r*fW)) = (BIT_8)(uv[1]);            (*(pfV+r*fW)) = (BIT_8)(uv[0]);
            pos=G_RC2POS(fH-1-r,fW-1-c,fW);
            /*if( Y0==YUV[posY] && V0==UV[posUV] && U0==UV[posUV+1] ){  //无效，似乎像素之间总有微小的变化
                pixels[pos]=pixels[pos0];
                if( r%2==1 )        posUV+=2;
                continue;
            }else{                pos0=pos;       Y0=YUV[posY];   V0=UV[posUV];   U0=UV[posUV+1];            }*/
            V=*(UV+posUV)-128;              U=*(UV+posUV+1)-128;
            if( 0 ) {
                Yf=1164*YUV[posY]-16000;
                fr=(Yf+1596*V)>>10;		        fg=(Yf-391*U-813*V)>>10;		fb=(Yf+2018*U)>>10;
                //cr=(BIT_8)fr;               cg=(BIT_8)fg;                       cb=(BIT_8)fb;
                cr = fr>255? 255 : fr<0 ? 0 : fr;
                cg = fg>255? 255 : fg<0 ? 0 : fg;
                cb = fb>255? 255 : fb<0 ? 0 : fb;
                //cr=cg=cb=YUV[posY];
            }else   {  //from gpuimage
                Yf=*YUV;
                Yf = Yf + (Yf >> 3) + (Yf >> 5) + (Yf >> 7);
                cb = Yf + (U << 1) + (U >> 6);                          //2.018
                cg = Yf - V + (V >> 3) + (V >> 4) - (U >> 1) + (U >> 3);
                cr = Yf + V + (V >> 1) + (V >> 4) + (V >> 5);           //1.596
                if(cr < 0) cr = 0; else if(cr > 255) cr = 255;
                if(cg < 0) cg = 0; else if(cg > 255) cg = 255;
                if(cb < 0) cb = 0; else if(cb > 255) cb = 255;
            }

            pixels[pos]= (cr << 16) + (cg << 8) + cb;
            if( r%2==1 )        posUV+=2;
        }
    }
    //LOGP( "nYUV2BMP_ [%d,%d]=>[%d,%d] time=%g",width,height,fW,fH,GST_TOC(tick)  );
    env->ReleaseByteArrayElements(yuvArr, pYUV, 0);
    env->ReleaseIntArrayElements(pixArr,pixels, 0);
    return 1;
}

//转置+左右镜像
void source2frame_T( INT_32 fH, INT_32 fW,INT_32 sH, INT_32 sW,jbyte* lSource,jbyte* uvSource,BIT_8 *frame,BIT_8 *fU,BIT_8 *fV,INT_32 inteval )	{
	INT_32 r,c,pos,posUV;
	BIT_8 *pfc,*pfU,*pfV,*uv;

	for (c = 0; c < fW; c++ ) {
		pos = c*inteval*sW;		posUV=c/2*inteval*sW;
		pfc = frame+fW-1-c;
		pfU = fU+fW-1-c;
		pfV = fV+fW-1-c;
		for (r = 0; r < fH; r++,pos+=inteval,posUV+=inteval ) {
			(*(pfc+r*fW)) = (BIT_8)(lSource[pos]);
			uv = (BIT_8*)(uvSource+posUV);
			(*(pfU+r*fW)) = (BIT_8)(uv[1]);
			(*(pfV+r*fW)) = (BIT_8)(uv[0]);
		}
	}

	if( 0 ){
		SHAPE_IMAGE si(fW,fH);
		BMPP bmpY( frame,&si ),bmpU( fU,&si ),bmpV( fV,&si );
		bmpY.Save( "/storage/sdcard0/1_cys/Y.bmp" );
		bmpU.Save( "/storage/sdcard0/1_cys/U.bmp" );
		bmpV.Save( "/storage/sdcard0/1_cys/V.bmp" );
	}

//	LOGP( "source2frame_T [%d,%d]=>[%d,%d],grid=%d",sW,sH,fW,fH,inteval );

}

extern "C" JNIEXPORT int JNICALL Java_com_sightsoft_hitrack_Facer_NDKinit( JNIEnv* env, jclass pClass,jobject assetManager,INT_32 f )	{
	LOGP( "NDKinit ... " );
	INT_32 ret=0x0;
	AAssetManager* mgr = AAssetManager_fromJava( env, assetManager );
	int *trees=nullptr;
	T_MOVE *moves=nullptr;
	size_t nzM=0,nzT=0,nzFace;
	nzM=Asset2Chars( mgr,"rf_D.dat",(char**)(&moves),0x0 );
	nzM /= sizeof(T_MOVE);
	nzT=Asset2Chars( mgr,"rf_T.dat",(char**)(&trees),0x0 );
	nzT /= sizeof(int);

	float *face000=nullptr;
	nzFace=Asset2Chars( mgr,"face_960.dat",(char**)(&face000),0x0 );		nzFace/=sizeof(float);
	//Asset2File( mgr,"3045005984_1.jpg","/storage/sdcard0/1_cys/",0x0 );
	FaceAlign_init( nzM,moves,nzT,trees,nzFace,face000,0x0 );
	free(moves);			free(trees);		free(face000);
	LOGP( "NDKinit OK ret=%d ",ret );
	return ret;
}

extern "C" JNIEXPORT void JNICALL Java_sightsoft_hitrack_Facer_NDKclear( JNIEnv* env, jclass pClass )	{
	LOGP( "Facer_NDKclear ... " );
	FaceAlign_clear( );
	__android_log_print(ANDROID_LOG_INFO,LOG_TAG, "Facer_NDKclear" );
	LOGP( "Facer_NDKclear ... !!! " );
}

extern "C" JNIEXPORT void JNICALL Java_com_sightsoft_hitrack_Facer_nBecome(
		JNIEnv *env, jclass clazz, jint nTo,jintArray pixArray,jint wth, jint hei,jintArray infArray, jint flag ){
	jint *pixels = env->GetIntArrayElements(pixArray, 0);
	jint *info = env->GetIntArrayElements(infArray, 0);
	LOGP( "nBecome to=%d, pixels=%p,w=%d,h=%d... ",nTo,pixels,wth,hei );
	NDK_FaceBecome_( nTo,pixels,wth,hei,info,flag );

	env->ReleaseIntArrayElements (pixArray, pixels, 0);
	env->ReleaseIntArrayElements (infArray,info,  0);
}


void Frame2Morph( BIT_8* frame,int fWth,int fHei,int *morphs,int *info,int flag ){
	LOGP( "nMorph pixels=%p,w=%d,h=%d... ",frame,fWth,fHei );
	//int left=info[0],rigt=info[1],botom=info[2],top=info[3];
	int c1=info[1],r1=info[3],wth=info[1]-info[0],hei=info[3]-info[2];
	int pos,r,c,pix,alpha=(50<<24),aFF=(0xFF<<24);
	BIT_8 cr,cg,cb,gray;
	for( r = info[2]; r < r1; r++) {
		for( c = info[0]; c < c1; c++) {
			pos=G_RC2POS(r, c, fWth);			gray = frame[pos];
			r = G_POS2R(pos, fWth);
			c = G_POS2C(pos, fWth);
			cr = cg = cb = gray;
			pix = aFF + (cr << 16) + (cg << 8) + cb;
			morphs[G_RC2POS(hei-1-(r-info[2]), c-info[0], wth)] = pix;
		}
	}
	info[4]=0;		info[5]=0;		info[6]=wth;		info[7]=hei;
}

void ctmf(    const BIT_8* const src, BIT_8* const dst,const int width, const int height,const int src_step, const int dst_step,
			  const int radius, const int channels, long unsigned int memsize=512*1024 );

extern "C" JNIEXPORT void JNICALL Java_com_sightsoft_hitrack_Facer_nFaceAlign(
        JNIEnv *env, jclass clazz, jbyteArray ain,jfloatArray pFeats,jintArray pInfo, jint wth, jint hei,jint flag )
{
    //nTest_1( flag );	return;
    jbyte *pixels = env->GetByteArrayElements(ain, 0);
    jfloat *feats = env->GetFloatArrayElements( pFeats, 0);
    jint *info = env->GetIntArrayElements( pInfo, 0);

    clock_t t0=clock( );
    //only support PORTRAIT mode, must ROTATE
    int grid=1,nWidth=hei/grid,nHeight=wth/grid,nPixels=wth*hei;
    while( nWidth>=480 && nHeight>=480 ){
        grid*=2;
        nWidth=hei/grid,nHeight=wth/grid;
    }

    int ori=0,i,no=info[32];
    BIT_8 *frame=new BIT_8[nWidth*nHeight],*fU=new BIT_8[nWidth*nHeight],*fV=new BIT_8[nWidth*nHeight];
    source2frame_T( nHeight,nWidth,hei,wth,pixels,pixels+wth*hei,frame,fU,fV,grid );

    BIT_8 *median=new BIT_8[nWidth*nHeight];
    ctmf( frame,median,nWidth,nHeight,nWidth,nWidth,1,1 );
    delete[] frame;		frame=median;

    //LOGP( "source2frame_T [%d,%d]=>[%d,%d],grid=%d",sW,sH,fW,fH,inteval );
    LOGP( "nTest source2frame=%g,[%d,%d]=>[%d,%d],morph=%d... ",(clock()-t0)*1.0/CLOCKS_PER_SEC,wth,hei,nWidth,nHeight,info[32] );

    int r,c,pos,nFace=NDK_FaceAlign_( frame,fU,fV,feats,info,nWidth,nHeight,nullptr,flag );

    for( i=0;i<4;i++ ) {
        info[i]*=grid;
    }
    int left=info[0],rigt=info[1],botom=info[2],top=info[3];
    if( nFace>0 ){
        LOGP( "face@nTest [%d,%d,%d,%d]",left,botom,rigt,top );
        float fr,fc;
        for( i=0;i<_FACE_MARK_;i++ )	{
            fc=feats[i];					fr=feats[i+_FACE_MARK_];
            feats[i]=fc*grid;			    feats[i+_FACE_MARK_]=fr*grid;
            //feats[i]=fc*grid-left;			feats[i+_FACE_MARK_]=fr*grid-botom;
            //LOGP( "%d (%g %g)=>(%g %g)",i,fc,fr,feats[i],feats[i+_FACE_MARK_] );
            //pos=r*nWidth+c;        frame[pos]=0xFF000000;
        }
        FaceAlign_ToMark0_( feats,1 );
        //for( i=0;i<nPixels;i++ )		pixels[i]=0;

    }
    delete[] frame;		delete[] fU;			delete[] fV;

    env->ReleaseByteArrayElements(ain, pixels, 0);
    env->ReleaseIntArrayElements(pInfo,info, 0);
    env->ReleaseFloatArrayElements(pFeats,feats, 0);
}


