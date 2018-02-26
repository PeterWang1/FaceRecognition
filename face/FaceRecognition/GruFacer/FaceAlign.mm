// ���ŻҶ�ͼƬ��face align��
//

#include <vector>
#include "./dlib/image_processing/frontal_face_detector.h"
#include "./dlib/gui_widgets.h"
#include "./dlib/image_io.h"
#include "GruST/util/GST_util.hpp"
#include "GruST/image/GST_image.hpp"
#include "FaceAlign_.h"

#import "getLuJIn.h"

using namespace std;
using namespace Grusoft;

//rf.cpp���ܲ�����
#define _RF_ALIGNER_

SHAPE_PtSet FaceAligner::spFace0(_FACE_MARK_0_,2);
SHAPE_PtSet FaceAligner::spFace(_FACE_MARK_,2);
int FaceAligner::wMax=480,FaceAligner::hMax=640;
static dlib::frontal_face_detector facor = dlib::get_frontal_face_detector();
static char sPath[500];
//int pts[]={18,20,25,27,55,56,57,58,59,60,49,18},nPt=sizeof(pts)/sizeof(int);
/*static int  face_58pts[]={37,38,28,30,31,34,52,58,59,60,49,37},face_nPt=sizeof(face_58pts)/sizeof(int);
 static int  face_all[]={37,38,45,46,55,56,57,58,59,60,49,37};
 static int  face_half[]={37,38,28,30,31,34,52,58,59,60,49,37};
 static int face_lE[]={37,38,39,40,41,42,37},face_rE[]={43,44,45,46,47,48,43};
 static int face_Blink[]={38,42,39,41,44,48,45,47};*/

int FaceAligner::face_58pts[12]={37,38,28,30,31,34,52,58,59,60,49,37};//face_nPt=sizeof(face_58pts)/sizeof(int);
int FaceAligner::face_all[12]={37,38,45,46,55,56,57,58,59,60,49,37};
int FaceAligner::face_half[12]={37,38,28,30,31,34,52,58,59,60,49,37};
int FaceAligner::face_lE[7]={37,38,39,40,41,42,37},FaceAligner::face_rE[7]={43,44,45,46,47,48,43};
int FaceAligner::face_Blink[8]={38,42,39,41,44,48,45,47};
static int face_nPt=sizeof(FaceAligner::face_58pts)/sizeof(int);
//#include "ge_face_detect.h"
extern "C" int FaceLocate_ge( BIT_8 *pixel,int wth,int hei,int*left,int*rigt,int*botom,int *top,INT_32 detect );

#define _FACE_NW_			256
#define _FACE_NH_			256

//static int *trees=nullptr;
//static size_t nnzMove=0,nnzTree=0;
static double tAlign=0,errAlign=0;

//static char *moves=nullptr;
void Face_Align_( BIT_8*gray,int hei,int ldW,float*shape,int **nodes,T_MOVE**moves,RoCLS& roi,int nMost,int flag );
//void Face_Align_( BIT_8*gray,int hei,int ldW,float*shape,int **nodes,signed char**moves,RoCLS& roi,int nMost,int flag=0x0){
FaceAligner::PARAM::PARAM( )	{
#ifdef ANDROID
    pMeanFace="/storage/sdcard0/1_cys/idea_face_2708.dat";
    pRF_D="/storage/sdcard0/1_cys/rf_D.dat";
    pRF_T="/storage/sdcard0/1_cys/rf_T.dat";
#else
    pMeanFace="F:\\GiFace\\ligner\\1_18\\idea_face_2708.dat";
    pRF_D="F:\\GiFace\\ligner\\1_18\\rf_D.dat";
    pRF_T="F:\\GiFace\\ligner\\1_18\\rf_T.dat";
#endif
}

static float meanF_2708[]={
    -0.728181,-0.611386,-0.445398,-0.273414,-0.11747,0.175963,0.340002,0.511231,0.677728,0.793909,0.0380643,0.0400934,0.042496,0.0457743,-0.148613,-0.0531066,
    0.0460405,0.145351,0.234305,-0.534995,-0.433107,-0.310568,-0.205566,-0.314173,-0.437033,0.276196,0.384917,0.506262,0.606622,0.517432,0.39715,-0.312707,-0.177395,
    -0.0429328,0.045267,0.145417,0.282933,0.415656,0.289562,0.159287,0.0519715,-0.0456399,-0.178773,-0.257322,-0.0429054,0.0469794,0.148321,0.360228,0.151017,0.0480138,
    -0.0439503,0.513897,0.623441,0.655248,0.631679,0.565966,0.579606,0.649372,0.674874,0.64357,0.539558,0.381067,0.222209,0.0646933,-0.0975478,-0.207216,-0.240519,
    -0.267792,-0.237239,-0.204736,0.352837,0.413487,0.411474,0.332863,0.311144,0.309817,0.341521,0.426425,0.427996,0.371058,0.327729,0.322744,-0.5049,-0.447962,
    -0.41928,-0.441856,-0.417104,-0.444415,-0.491118,-0.624159,-0.68387,-0.69689,-0.688519,-0.63415,-0.510827,-0.49875,-0.50701,-0.494873,-0.500155,-0.564004,-0.578086,
    -0.568861,
};
FaceAligner::FaceAligner( int flag ) : meanFace(nullptr),siFace(_FACE_NW_,_FACE_NH_,3),moves(nullptr),trees(nullptr),becomGray(nullptr) {
    roiLast.c_0=0;	roiLast.r_0=0;	roiLast.wth=-1;	roiLast.hei=-1;
    lastFace=new float[_FACE_MARK_*2];
    siFace.bpp = 24;
    nCas=25;
    maxBlink=0.0;
    if( 1 ){
        assert( sizeof(meanF_2708)/sizeof(float)==_FACE_MARK_*2 );
        meanFace=new float[_FACE_MARK_*2];
        memcpy( meanFace,meanF_2708,sizeof(meanF_2708) );
        LOGP( "Init MeanFace from meanF_2708 "  );
    }else{
        LoadMeanFace( param.pMeanFace.c_str(),0x0 );
    }
    SetBecome( -1,nullptr,-1,-1 );
}

void FaceAligner::SetBecome( int nTo,BIT_8*gray,int wth,int hei,int flag ){
    if( becomGray!=nullptr)
        delete[] becomGray;
    becomNO=-1,		becomWth=-1,		becomHei=-1;
    becomGray=nullptr;
    maxBlink = 0.0;         LOGP( "FaceAligner::maxBlink to zero" );
    
    if( wth>0 && hei>0 && nTo>0 ){
        becomNO=nTo;
        becomGray=new BIT_8[wth*hei];
        memcpy( becomGray,gray,sizeof(BIT_8)*wth*hei );
        becomWth=wth,		becomHei=hei;
        LOGP( "FaceAligner::SetBecome nTo=%d,gray=(%d,%d..),w=%d,h=%d... ",nTo,becomGray[0],becomGray[hei*wth/2],wth,hei );
    }
}

//SHAPE_IMAGE siFace(_FACE_NW_,_FACE_NH_,3);
bool FaceAligner::LoadMeanFace( const char *sPath,int flag ){
    try{
        //	char *sPath="F:\\GiFace\\idea_face_2708.dat";
        LOGP( "********* Load mean shape from %s",sPath );
        SERIAL ar( sPath,true,flag);
        RoCLS roi;
        string nam;
        int n=_FACE_MARK_0_*2,i;
        meanFace=new float[_FACE_MARK_*2];
        float *vY=new float[n];
        int status=ar.OBJ(roi).CHARs(nam).ARR(vY,n).Close();
        //for( i=0;i<n;i++ )	meanFace[i]=dShape[i];
        for( i=0;i<_FACE_MARK_;i++ ){
            meanFace[i]=vY[i+17];		meanFace[i+_FACE_MARK_]=vY[i+17+_FACE_MARK_0_];
        }
        delete[] vY;
        return true;
    }catch (...)	{
        LOGP( "FaceAligner::LoadMeanFace is X" );
        GST_THROW( "FaceAligner::LoadMeanFace is X" );
    };
}

void FaceAligner::SetData( size_t nzM,T_MOVE*mov,size_t nzT,int *tre,int flag ){
    moves=new T_MOVE[nzM];
    memcpy( moves,mov,sizeof(T_MOVE)*nzM );
    int i,nNeg=0;
    for( i=0;i<nzM;i++ ){
        if( moves[i]>100 ){
            moves[i]=moves[i]-256;		nNeg++;
        }
        
    }
    
    trees=new int[nzT];
    memcpy( trees,tre,sizeof(int)*nzT );
    nnzMove=nzM,			nnzTree=nzT;
    //signed char *tm=moves;		//only for debug
    LOGP( "mov(%d)={%d %d %d %d ... %d %d},nNeg=%d",nzM,moves[0],moves[1],moves[51],moves[52],moves[nzM-2],moves[nzM-1],nNeg );
    LOGP( "tre(%d)={%d %d %d %d ... %d %d}",nzT,trees[0],trees[1],trees[51],trees[52],trees[nzT-2],trees[nzT-1] );
}

int FaceAligner::CopyMeanFace( float **mf,int flag ){
    int len=_FACE_MARK_*2;
    *mf=new float[len];
    memcpy( *mf,meanFace,sizeof(float)*len );
    return len;
}

static int nFaceDetect=0;
//��ʱȡ�������
bool FaceAligner::Detect_( GST_BMP *hSrc,BIT_8 *gray,int wth,int hei,int mode,int flag ){
    int left,rigt,botom,top,i,box_0=0,r,c,scale=1,pos,nz;
    double a;
    GST_TIC( tick );
    
    /*if( mode==0 ){
     LOGP( "Detect_mode=0" );
     roi=roiLast;
     return roi.wth>0;
     }*/
    
    std::vector<dlib::rectangle> dets;
    if( wth>=360 || hei>=360 ){
        dlib::array2d<unsigned char> img(hei/2,wth/2);
        BIT_8 *pixel=&(img[0][0]),*row1=pixel,*row0=gray;
        for( r=0;r<hei;r+=2){	//flip vertical
            //row0 = gray+(hei-1-r)*wth;
            row0 = gray+r*wth;
            for( c=0;c<wth;c+=2) {		row1[c/2]=row0[c];		}
            row1 += wth/2;
        }
        getLuJIn *luji =[[getLuJIn alloc]init];
        dlib::save_bmp(img, [[luji putTheFileName:@"1" andType:@"bmp"]UTF8String]);
      dets = facor(img);
    }else{
        dlib::array2d<unsigned char> img(hei,wth),img1;
        BIT_8 *pixel=&(img[0][0]),*row0=gray;
        for( nz=0,r=0;r<hei;r++){	//flip vertical
            row0 = gray+r*wth;
            
        }/**/
        for( r=0;r<hei;r++){	//flip vertical
            //memcpy( pixel+r*wth,gray+(hei-1-r)*wth,sizeof(BIT_8)*wth );
            memcpy( pixel+r*wth,gray+r*wth,sizeof(BIT_8)*wth );
        }
                getLuJIn *luji =[[getLuJIn alloc]init];
                dlib::save_bmp(img, [[luji putTheFileName:@"1" andType:@"bmp"] UTF8String]);
        		dets = facor(img);
    }
    
    /*	if( dets.size()==0 ){
     pyramid_up(img);
     dlib::save_jpeg(img, "F:\\GiFace\\trace\\gray.jpg");
     dets = facor(img);
     }*/
    a = GST_TOC( tick );
    LOGP( "Detect_time=%g\n",a );
    if( dets.size()==0 )		{
        roiLast.c_0=0;	roiLast.r_0=0;	roiLast.wth=0;	roiLast.hei=0;
        return false;
    }
    for( i=0;i<dets.size( );i++ )	{
        if( dets[i].width()>box_0 )	{
            left=dets[i].left()*scale,				rigt=dets[i].right()*scale;
            botom=hei-1-dets[i].bottom()*scale,		top=botom+dets[i].height()*scale;
            box_0=dets[i].width();
            printf("face %d-[%d:%d  %d:%d]\n",i+1,left,rigt,botom,top );
        }
    }
    
    if( a>1.0 )		printf( "time=%g,",a );
    //3/1/2016      确保box不越界
    G_RC_NORMAL(left,0,wth-1);              G_RC_NORMAL(rigt,0,wth-1);
    G_RC_NORMAL(top,0,hei-1);               G_RC_NORMAL(botom,0,hei-1);
    roi = RoCLS(left,botom,rigt-left,top-botom,0,0);
    roiLast=roi;
    //LOGP( "dr=%d,dc=%d",dr,dc );
    if( 0 && hSrc!=nullptr ){
        BMP_DRAW *hTrace=new BMP_DRAW(hSrc);		//BMP_DRAW���ڴ�й©
        hTrace->Line( left,rigt,botom,botom );		hTrace->Line( rigt,rigt,top,botom );
        hTrace->Line( left,rigt,top,top );			hTrace->Line( left,left,top,botom );
        //printf( "%d",rigt-left );
        char sPath[1000];
        sprintf( sPath,"F:\\GiFace\\trace\\%d.bmp",nFaceDetect++ );
        hTrace->Save( sPath );
        delete hTrace;
    }
    
    return true;
}

bool FaceAligner::isStatic( BIT_8 *gray,int wth,int hei,float*shape,int flag ){
    if( roiLast.wth==0 )
        return false;
    GST_TIC(tick);
    int *tree0=trees,i,c,r,pos,nPix=pixs.size( ),g;
    double off=0.0,len=0.0,thrsh=0.03;
    if( nPix>0 ){
        for( i=0;i<nPix;i++ ){
            pos=G_RC2POS(pixs[i].r,pixs[i].c,wth );
            g = gray[pos];
            len+=g;		off+=abs(g-pixs[i].pos);
        }
        len/=nPix;		off/=nPix;
        thrsh=0.1;
    }else{
        memcpy( shape,meanFace,sizeof(float)*_FACE_MARK_*2 );
        T_MOVE *move0=moves;
        //LOGP( "shape={%g %g %g %g}",shape[0],shape[1],shape[51],shape[52] );
        Face_Align_( gray,hei,wth,shape,&tree0,&move0,roi,nCas,0x0 );
        //LOGP( "shape={%g %g %g %g}",shape[0],shape[1],shape[51],shape[52] );
        
        for( i=0;i<_FACE_MARK_*2;i++){
            off+=(lastFace[i]-shape[i])*(lastFace[i]-shape[i]);
            len+=shape[i]*shape[i];
        }
        off=sqrt(off);		len=sqrt(len);
    }
    
    LOGP( "isStatic:  time=%g,nPix=%d off=%g,len=%g",GST_TOC(tick),nPix,off,len );
    if( off<len*thrsh  ){
        for( i=0;i<_FACE_MARK_*2;i++){
            shape[i]=lastFace[i];
        }
        return true;
    }
    return false;
}

FaceAligner::CODE FaceAligner::Align( BIT_8 *gray,int wth,int hei,float*shape,int flag ){
    
    float e0,e1=1.0e6,delta;
    if( 1 )
        memcpy( shape,meanFace,sizeof(float)*_FACE_MARK_*2 );
    
    int *tree0=trees,nIter=0,i,c,r;
    GST_TIC(tick);
    
    //float *fm=moves;
    T_MOVE *move0=moves;
    move0=moves;		tree0=trees;
    //LOGP( "shape={%g %g %g %g}",shape[0],shape[1],shape[51],shape[52] );
    Face_Align_( gray,hei,wth,shape,&tree0,&move0,roi,nCas,0x0 );
    //LOGP( "shape={%g %g %g %g}",shape[0],shape[1],shape[51],shape[52] );
    //assert( (int)(move0-moves)==nnzMove );		assert( (int)(tree0-trees)==nnzTree );
    /*for( i=0;i<_FACE_MARK_;i++){
     roi.Map1_f( shape[i],shape[i+_FACE_MARK_],c,r );
     shape[i]=c;		shape[i+_FACE_MARK_]=r;
     }*/
    double off=0.0,len=0.0;
    for( i=0;i<_FACE_MARK_*2;i++){
        off+=(lastFace[i]-shape[i])*(lastFace[i]-shape[i]);
        len+=shape[i]*shape[i];
    }
    off=sqrt(off);		len=sqrt(len);
    //LOGP( "off=%g,len=%g",off,len );
    if( off<len*0.01  ){
        for( i=0;i<_FACE_MARK_*2;i++){
            shape[i]=lastFace[i];
        }
    }else	if( off<len*0.03  ){
        for( i=0;i<_FACE_MARK_*2;i++){
            shape[i]=(shape[i]+lastFace[i])/2;
        }
    }
    
    memcpy( lastFace,shape,sizeof(float)*_FACE_MARK_*2 );
    LOGP( "FaceAligner::Align_time=%g,w=%d,h=%d ... ",GST_TOC(tick),wth,hei );
    
    return OK;
}

#include "./Eigen/Dense"
#include "./Eigen/Core"
#include "./Eigen/SVD"
using namespace Eigen;
void SimilarRS( Eigen::MatrixXf &vFrom,Eigen::MatrixXf &vTo,Eigen::MatrixXf& trans,Eigen::RowVector2f& off,int flag ){
    int nRow=vFrom.rows( ),i;
    Eigen::RowVectorXf meanF=vFrom.colwise().mean(),meanT=vTo.colwise().mean();
    Eigen::MatrixXf cov=Matrix2f::Zero(),matS=Matrix2f::Identity( );
    double sigF=0.0,sigT=0.0,x1,x2,x3,x4;
    for( i = 0;i < nRow;i++){
        Eigen::RowVectorXf dF=vFrom.row(i)-meanF,dT=vTo.row(i)-meanT;
        sigF += dF.squaredNorm();    sigT += dT.squaredNorm();
        cov+=dT.transpose()*dF;
    }
    Eigen::MatrixXf u,v;
    JacobiSVD<Matrix2f> svdOfA(cov,ComputeFullU|ComputeFullV);
    u=svdOfA.matrixU( );	v=svdOfA.matrixV( );
    Eigen::MatrixXf d=svdOfA.singularValues( );
    double det=cov.determinant( ),trace=0.0;
    if( det<0 || (det==0 && u.determinant()*v.determinant()<0) ){
        if (d(1,1) < d(0,0))
            matS(1,1) = -1;
        else
            matS(0,0) = -1;
    }
    trans=u*matS*v.transpose();
    //normS=1.0;
    if( sigF!=0 ){
        trace = d(0)*matS(0,0)+d(1)*matS(1,1);
        trans *= (1.0/sigF*trace);
    }
    //trans=toIdea;
    off = meanT - meanF*trans.transpose();
    //toThis = toIdea.inverse( );
    //x1=toIdea(0,0),x2=toIdea(0,1),x3=toIdea(1,0),x4=toIdea(1,1);
}
float NDK_FoldDis( NDKvFACE From,NDKvFACE To,Eigen::MatrixXf&trans,Eigen::RowVector2f&pan,int flag=0x0 ){
    Eigen::MatrixXf vFrom=Eigen::Map<Eigen::MatrixXf>(From,_FACE_MARK_,2);
    Eigen::MatrixXf vTo=Eigen::Map<Eigen::MatrixXf>(To,_FACE_MARK_,2);
    //LOGP( "%g,%g,%g,%g",From[0],From[51],From[1],From[52]);
    
    //GST_util::ARR_ouput( "D:\\trace\\from.dat",vFrom.data(),_FACE_MARK_,2,false );
    //GST_util::ARR_ouput( "D:\\trace\\to.dat",vTo.data(),_FACE_MARK_,2,false );
    
    SimilarRS( vFrom,vTo,trans,pan,0x0 );
    
    Eigen::MatrixXf vS0=(vFrom*trans.transpose( ));
    vS0.rowwise()+=pan;
    Eigen::MatrixXf ct1=vTo.rowwise()-vTo.colwise().mean(),
    ct2=vFrom.rowwise()-vFrom.colwise().mean(),
    ct3=vS0.rowwise()-vS0.colwise().mean();
    double a2=(ct2-ct1).norm( ),a3=(ct3-ct1).norm( ),a4=(vS0-vTo).norm();
    if( a2<=a3 || fabs(a3-a4)>1.0E-5 ){
        LOGP("ShapeBMPfold::SimilarRS failed");//GST_THROW("ShapeBMPfold::SimilarRS failed");
    }
    float interoc=NDKvF_interoc(To);
    float dis=a4/interoc;
    //LOGP( "%g,%g,%g,%g iroc=%g,dis=%g",To[0],To[51],To[1],To[52],interoc,dis );
    return dis;
}

FaceAligner::CODE FaceAligner::Align( GST_BMP *hbmp,float*shape,int flag ){
    float e0,e1=1.0e6,delta;
    int hei=hbmp->Height(),wth=hbmp->Width( );
    BIT_8 *gray=new BIT_8[wth*hei];
    hbmp->Channel_(GST_BMP::GRAY,gray);
    if(	Detect_( hbmp,gray,wth,hei,1) ){
    }	else
        return NO_FACE;
    
    if( 1 )
        memcpy( shape,meanFace,sizeof(float)*_FACE_MARK_*2 );
    int *tree0=trees,nIter=0;
    GST_TIC(tick);
    T_MOVE *move0=moves;
    //	do{
    move0=moves;		tree0=trees;
#ifdef _RF_ALIGNER_
    Face_Align_( gray,hei,wth,shape,&tree0,&move0,roi,nCas,0x0 );
#else
    GST_THROW( "Face_Align_ is ..." );
#endif
    //		shape0-=shape;
    //		delta=shape0.norm()/shape.norm( );
    assert( (int)(move0-moves)==nnzMove );		assert( (int)(tree0-trees)==nnzTree );
    //	}while( delta>0.01 && (++nIter)<1 );
    tAlign+=GST_TOC(tick);
    
    return OK;
}

static FaceAligner *hAlign=nullptr;
//static float *fMark=nullptr;
extern "C" int FaceAlign_init( size_t nzM,T_MOVE*mov,size_t nzT,int *tre,size_t nzFace,float *face000,int flag ){
    
    hAlign=new FaceAligner( );
    hAlign->SetData( nzM,mov,nzT,tre );
    hAlign->face000=new float[nzFace];
    hAlign->nMorph=nzFace/NDKvF_ld;
    memcpy( hAlign->face000,face000,sizeof(float)*nzFace );
    float *To=face000;
    
    int i,off=18;
    if( FaceAligner::face_58pts[0]==37 ){
        LOGP( "FaceAlign_init ... landmarks off by %d ",off  );
        for( i=0;i <face_nPt; i++ ) {
            FaceAligner::face_58pts[i] -= off;
            FaceAligner::face_all[i] -= off;
            FaceAligner::face_half[i] -= off;
        }
        for( i=sizeof(FaceAligner::face_lE)/sizeof(int)-1; i>=0;   i-- ) {        FaceAligner::face_lE[i] -= off;    }
        for( i=sizeof(FaceAligner::face_rE)/sizeof(int)-1; i>=0;   i-- ) {        FaceAligner::face_rE[i] -= off;    }
        for( i=sizeof(FaceAligner::face_Blink)/sizeof(int)-1; i>=0;   i-- ) {        FaceAligner::face_Blink[i] -= off;    }
    }
    LOGP( "FaceAlign_init ... nzFace=%d %d MARKs={%d...%d,%d}",nzFace,face_nPt,FaceAligner::face_58pts[0],FaceAligner::face_58pts[face_nPt-2],FaceAligner::face_58pts[face_nPt-1]  );
    return 0x0;
}
extern "C" void FaceAlign_ToMark0_( float *marks,int flag ){
    int i,off=18-1;
    float mark0[_FACE_MARK_0_*2],*mc=mark0,*mr=mc+_FACE_MARK_0_;
    memset( mark0,0x0,sizeof(float)*_FACE_MARK_0_*2 );
    for( i=0;i<_FACE_MARK_;i++ )	{
        mc[i+off]=marks[i];
        mr[i+off]=marks[i+_FACE_MARK_];
        if( flag==1 )
            LOGP( "%d-(%g %g)",i+off+1,mc[i+off],mr[i+off] );
    }
    memcpy( marks,mark0,sizeof(float)*_FACE_MARK_0_*2 );
    return ;
}

FaceAligner* FaceAligner::Instance( ){
    return hAlign;
}

extern "C" int FaceAlign_clear( ) {
    if (hAlign != nullptr) delete hAlign;
    //if (fMark != nullptr) delete[] fMark;
    LOGP("FaceAlign_clear !!!");
    return 0x0;
}

#if defined ANDROID || defined __APPLE__

extern "C" int NDK_FaceBecome_( int nTo,int *pixels,int wth,int hei,int *info,int flag ){
    Grusoft::FaceAligner *hAlign=Grusoft::FaceAligner::Instance( );
    LOGP( "NDK_FaceBecome_ nTo=%d,pixels=%p,w=%d,h=%d... ",nTo,pixels,wth,hei );
    int no,r,c,pos,pix,nPixel=wth*hei,alpha=(0<<24),aFF=(0xFF<<24);
    BIT_8 cr,cg,cb,*gray=new BIT_8[wth*hei];
    for (pos = 0; pos < nPixel; pos++) {
        pix = pixels[pos];
        cr = (pix & 0x00FF0000) >> 16;		cg = (pix & 0x0000FF00) >> 8;		cb = (pix & 0x000000FF);
        cr = cg = cb = (((cb) * 117 + (cg) * 601 + (cr) * 306) >> 10);
        pixels[pos] = alpha + (cr << 16) + (cg << 8) + cb;
        r=G_POS2R(pos,wth);			c=G_POS2C(pos,wth);
        gray[G_RC2POS(hei-1-r,c,wth)] = cg;
    }
    if( 1 ){
        if( hAlign!=nullptr)    {
            hAlign->SetBecome( nTo,gray,wth,hei );
            hAlign->maxBlink=0.0;
        }
    }
    delete[] gray;
    
    return 0x0;
}

int NDK_FaceAlign_( BIT_8 *gray,BIT_8 *fU,BIT_8 *fV,float *fMark,int *info,int wth,int hei,int *morphs,int flag )	{
    clock_t t0=clock( );
    
    int i,c,r,pos,nFace=0,morph=info[32];
    BMPP *frame=nullptr;
    info[8]=0;					info[9]=0;		info[10]=0;		info[16]=-1;
    if( hAlign != nullptr ){
        if( hAlign->isStatic( gray,wth,hei,fMark ) ){
            info[8]=nFace=1;		info[9]=1;
            info[10]=1;		//isStatic
            int left=hAlign->roi.c_0,rigt=left+hAlign->roi.wth,	botom=hAlign->roi.r_0,top=botom+hAlign->roi.hei;
            info[0]=left;	info[1]=rigt;	info[2]=botom;	info[3]=top;
            LOGP( "hAlign isStatic!!! -- [%d,%d,%d,%d]",left,botom,rigt,top );
        }else	if( hAlign->Detect_( nullptr,gray,wth,hei,info[33] ) )	{
            info[8]=nFace=1;
            int left=hAlign->roi.c_0,rigt=left+hAlign->roi.wth,	botom=hAlign->roi.r_0,top=botom+hAlign->roi.hei;
            LOGP( "hAlign -- [%d,%d,%d,%d]",left,botom,rigt,top );
            hAlign->Align( gray,wth,hei,fMark,0x0 );			info[9]=1;
        }
        if( info[9]==1 ){//为了上层函数的需要
            for( i=0;i<_FACE_MARK_;i++){
                hAlign->roi.Map1_f( fMark[i],fMark[i+_FACE_MARK_],c,r );
                fMark[i]=c;		fMark[i+_FACE_MARK_]=r;
            }
            if( info[10]==0 )	{	//is NON-Static
                Makeup::PIXELS &pixs=hAlign->pixs;		pixs.clear( );
                int neibor_3[9]={-wth-1,-wth,-wth+1,-1,0,1,wth-1,wth,wth+1},j,pos_0;
                for( i=0;i<_FACE_MARK_;i++){
                    c=fMark[i];		r=fMark[i+_FACE_MARK_];		pos_0=G_RC2POS(r,c,wth);
                    for( j=0;j<9;j++ ){
                        pos=pos_0+neibor_3[j];
                        c=G_POS2C(pos,wth);		r=G_POS2R(pos,wth);
                        pixs.push_back( F4NO(-1,gray[pos],c,r) );
                    }
                    
                }
            }
            
        }
        
        
    }else{
        dlib::array2d<unsigned char> img(hei,wth);
        BIT_8 *pixel=&(img[0][0]);
        for( r=0;r<hei;r++){	//flip vertical
            memcpy( pixel+r*wth,gray+(hei-1-r)*wth,sizeof(BIT_8)*wth );
        }
        std::vector<dlib::rectangle> dets = facor(img);
        nFace = dets.size();
        if( nFace>0 ){
            info[8]=nFace=1;
            int left=dets[0].left(),rigt=dets[0].right(),botom=hei-1-dets[0].bottom(),top=botom+dets[0].height();
            LOGP( "%d Faces. [%d,%d,%d,%d]",nFace,left,botom,rigt,top );
            info[0]=left;	info[1]=rigt;	info[2]=botom;	info[3]=top;
        }else{
            LOGP( "NDK_FaceAlign_ hAlign=NULL!!!!	nFace=%d",nFace );
        }
    }
    LOGP( "NDK_FaceAlign_time=%g,gray=%p,w=%d,h=%d ... ",(clock()-t0)*1.0/CLOCKS_PER_SEC,gray,wth,hei );
    return nFace;
}
#endif
