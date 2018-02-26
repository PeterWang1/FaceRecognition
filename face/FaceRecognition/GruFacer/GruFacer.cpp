//
//  GruFacer.m
//  GruFacer
//
//  Created by gao on 2017/3/21.
//  Copyright © 2017年 KK. All rights reserved.
//
#import "GruFacer.h"
#include "FaceAlign_.h"

//
extern "C"  size_t Asset2Chars( const char* sFile,char **dst,int flag )  {
    /*AAsset* asset = AAssetManager_open(mgr, sFile, AASSET_MODE_UNKNOWN);
     if (NULL == asset) {
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "_ASSET_NOT_FOUND_");
     return 0;
     }*/
    
    size_t szDat = 0;//AAsset_getLength(asset);
    char* buffer = NULL;  //(char*) malloc (sizeof(char)*size);
    FILE *fp=fopen(sFile, "rb");
    
    if( fp != NULL ){
        fseek(fp,0,SEEK_END);
        szDat = ftell(fp);
        fseek(fp,0,SEEK_SET);
        buffer = new char[szDat];
        if(fread( buffer,sizeof(char),szDat,fp) != szDat){
            delete[] buffer;       buffer=NULL;
        }
        //AAsset_read (asset,buffer,size);
        //AAsset_close(asset);
        fclose(fp);
    }
    LOGP( "Load %s size=%d",sFile,szDat );
    *dst=buffer;
    return szDat;
}

int Facer_init( const char *uJin1,const char *uJin2,INT_32 f )	{
    LOGP( "NDKinit ... " );
    INT_32 ret=0x0;
    //AAssetManager* mgr = AAssetManager_fromJava( env, assetManager );
    int *trees=nullptr;
    T_MOVE *moves=nullptr;
    size_t nzM=0,nzT=0,nzFace;
    nzM=Asset2Chars( uJin1,(char**)(&moves),0x0 );
    nzM /= sizeof(T_MOVE);
    nzT=Asset2Chars( uJin2,(char**)(&trees),0x0 );
    nzT /= sizeof(int);
    
    float *face000=nullptr;
    nzFace=0;
    FaceAlign_init( nzM,moves,nzT,trees,nzFace,face000,0x0 );
    //free(moves);			free(trees);		free(face000);
    delete[] moves;         delete[] trees;     //delete[] face000;
    LOGP( "NDKinit OK ret=%d ",ret );
    return ret;
}

void Facer_clear(  )	{
    LOGP( "Facer_NDKclear ... " );
    FaceAlign_clear( );
    //__android_log_print(ANDROID_LOG_INFO,LOG_TAG, "Facer_NDKclear" );
    LOGP( "Facer_NDKclear ... !!! " );
}

//转置+左右镜像
void source2frame_T( INT_32 fH, INT_32 fW,INT_32 sH, INT_32 sW,BIT_8* lSource,BIT_8* uvSource,BIT_8 *frame,BIT_8 *fU,BIT_8 *fV,INT_32 inteval )	{
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
        /*SHAPE_IMAGE si(fW,fH);
         BMPP bmpY( frame,&si ),bmpU( fU,&si ),bmpV( fV,&si );
         bmpY.Save( "/storage/sdcard0/1_cys/Y.bmp" );
         bmpU.Save( "/storage/sdcard0/1_cys/U.bmp" );
         bmpV.Save( "/storage/sdcard0/1_cys/V.bmp" );*/
    }
    
    //	LOGP( "source2frame_T [%d,%d]=>[%d,%d],grid=%d",sW,sH,fW,fH,inteval );
    
}

void ctmf(    const BIT_8* const src, BIT_8* const dst,const int width, const int height,const int src_step, const int dst_step,
          const int radius, const int channels, long unsigned int memsize=512*1024 );
//int NDK_FaceAlign_( BIT_8 *gray,BIT_8 *fU,BIT_8 *fV,float *fMark,int *info,int wth,int hei,int *morphs,int flag );
void GruFacer_Test( BIT_8 * pixels,float * feats,int * info, int wth, int  hei,int  flag ){
    //jbyte *pixels = env->GetByteArrayElements(ain, 0);
    //jfloat *feats = env->GetFloatArrayElements( pFeats, 0);
    //jint *info = env->GetIntArrayElements( pInfo, 0);
    
    clock_t t0=clock( );
    //only support PORTRAIT mode, must ROTATE
    int grid=1,nWidth=hei/grid,nHeight=wth/grid,nPixels=wth*hei;
    while( nWidth>=480 && nHeight>=480 ){
        grid*=2;
        nWidth=hei/grid,nHeight=wth/grid;
    }
    
    int ori=0,i,no=info[32];
    BIT_8 *frame=new BIT_8[nWidth*nHeight],*fU=new BIT_8[nWidth*nHeight],*fV=new BIT_8[nWidth*nHeight];
    if( true )
        memcpy(  frame,pixels,sizeof(BIT_8)*nWidth*nHeight );
    else
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
}

