//#include <omp.h>
#if !defined(__APPLE__) && !defined(__ANDROID__)
	#include <sys/io.h>
	#include <io.h>
	#include "..\util\BLAS_t.hpp"
#endif
#include "GST_dataset.hpp"
#include "../image/GST_image.hpp"
#include "../util/GST_util.hpp"

std::string DataSample::sDumpFolder;
std::string const DataSample::sBackSrc="background:";
std::string DataSample::sLastPicPath;
/*
bool DataSample::Preprocess(int alg, int flag)	{
	try{
		int i, nS = nSample();
		Sigma_Scal(alg, 0x0);
		return true;
	}
	catch (...)	{
		return false;
	}
}

bool DataSample::Preprocess(double *trans, double *mean, double *temp, int alg, int flag)	{
	try{
		assert(temp != nullptr);
		int i, nSam = nSample();
		double *tX = temp, *X = (double *)Sample(0);
		memcpy(tX, X, sizeof(double)*ldX*nSam);
		for (i = 0; i < nSam; i++)	{
			AXPY(ldX, -1.0, mean, tX + i*ldX);
		}
		AB2C(charN, charN, ldX, nSam, ldX, trans, ldX, tX, ldX, X, ldX);		//X=trans*X

		return true;
	}
	catch (...)	{
		return false;
	}
}*/


int DataSample::Save(const std::wstring sPath, int flag)	{
#ifdef WIN32
	int ret = -1, dim = nSample();
	FILE *fp = _tfopen(sPath.c_str(), _T("wb"));
	if (fp == NULL)
		goto _FIO_EXIT_;
	ret = save_fp(fp, flag);
_FIO_EXIT_:
	if (fp != NULL && fclose(fp) != 0)	{		ret = -7;	}
	if (ret == 0)	{
	}
	else	{
		ret = fp != NULL ? ferror(fp) : ret;
	}
	return ret;
#else
	throw "DataSample::Save is ...";
#endif
}
int DataSample::Save(const std::string sPath, int flag)	{
	int ret = -1, dim = nSample();
	FILE *fp = fopen(sPath.c_str(), ("wb"));
	if (fp == NULL)
		goto _FIO_EXIT_;
	ret = save_fp(fp, flag);
_FIO_EXIT_:
	if (fp != NULL && fclose(fp) != 0)	{		ret = -7;	}
	if (ret == 0)	{
	} else	{
		ret = fp != NULL ? ferror(fp) : ret;
	}
	return ret;
}

int DataSample::Load(const std::wstring sPath, int flag)	{
#ifdef WIN32
	try{
		int ret = -1;
		FILE *fp = _tfopen(sPath.c_str(), _T("rb"));
		if (fp == NULL)
			goto _FIO_EXIT_;
		ret = load_fp(fp, flag);
	_FIO_EXIT_:
		if (fp != NULL && fclose(fp) != 0)
		{
			ret = -7;
		}
		if (ret == 0)	{
		} else	{
			ret = fp != NULL ? ferror(fp) : ret;
			//		G_PRINTF( ("\t!!!Failed to save %s. err=%d"),sLibPath,ret );
		}
		return ret;
	}
	catch (...)	{
		throw exception("DataSample::Load !!!");
	}
#else
	throw "DataSample::Load is ...";
#endif
}
int DataSample::Load(const std::string sPath, int flag)	{
	try{
		int ret = -1;
		FILE *fp = fopen(sPath.c_str(), ("rb"));
		if (fp == NULL)
			goto _FIO_EXIT_;
		ret = load_fp(fp, flag);
	_FIO_EXIT_:
		if (fp != NULL && fclose(fp) != 0)
		{
			ret = -7;
		}
		if (ret == 0)	{
		} else	{
			ret = fp != NULL ? ferror(fp) : ret;
			//		G_PRINTF( ("\t!!!Failed to save %s. err=%d"),sLibPath,ret );
		}
		return ret;
	}
	catch (...)	{
		throw ("DataSample::Load !!!");
	}
}

int DataSample::SelectTo(hDATASET hTo, vector<int>& nEach,bool isMove, int flag){
	assert( hTo!=nullptr );
	int cls,nCls=nEach.size( ),i,nz,iT=0, iS=0,no;
	int nData=nSample(),nz_0=nData,nz_1=0,nLeft=nData,nTo;
	vector<int> list;
	for( cls=0; cls<nCls; cls++ ){
		nz=0;		list.clear( );
		for( i=0; i<nData; i++ )		{
			if(tag[i]==cls)	{	
				list.push_back(i);			nz++;
			}
		}
		//if( test!=nullptr )
		std::random_shuffle( list.begin(),list.end() );
		nEach[cls] = MIN( nEach[cls],nz );		assert(nEach[cls]>=0);
		nz_0=MIN(nz_0,nEach[cls]);					nz_1=MAX(nz_1,nEach[cls]);	
		for( i=0; i<nz; i++ )	{
			no=list[i];			assert( tag[no]==cls );
			if( i<nEach[cls] )	{
				TransTo(no, hTo, iS++);
			}
			else if( isMove ){
				TransTo(no, this, iT++);
			}
		}
	}
	for( i = iS; i < hTo->nMost; i++ )		hTo->tag[i]=-1;		nTo = hTo->nSample();
	if( isMove ){
		for( i = iT; i < nData; i++ ){
			if( tag[i]>=nCls)
					TransTo(i, this, iT++);
		}
		for( i = iT; i < nData; i++ )	tag[i]=-1;
		nLeft = nSample();
		assert(nLeft + nTo <= nData);
		GST_util::print( "===>Move from %d to %d. Left=%d\teach===", nData, nTo, nLeft );
	}else{
		GST_util::print( "===>Copy from %d to %d\teach===", nData, nTo );
	}
	for( i=0; i<nCls; i++ )		GST_util::print( "%d,",nEach[i]);
	GST_util::print( "===\n" );

	return 0;

}

/*	��X����������(train,test)	*/
int DataSample::SelectTT(hDATASET train, hDATASET test, int type, int flag)	{
	int nData = nSample(), nTrain=train->nMost, nTest=test->nMost, i = 0, j, iT = 0, iS = 0;
	int gdTrain, gdTest = 0;
	if (nData > 2 * (nTrain + nTest))	{
		//		nData=data.Shrink( nTrain+nTest,0x0 );
		//		data.ToBmp(-1);
	}
	int grid = max(nTrain / nTest, nTest / nTrain);//assert( grid>=2 );
	int step = max(0, nData / min(nTrain, nTest) - (1 + grid));
	if( type==2 ){	//select test from train
		assert( train==this );
		grid = (nData-nTest)/nTest;		step = max(0,nData/nTest-(1 + grid));
	}
	char *sPath;
	while (i < nData && (iT < nTrain || iS < nTest))	{
		if (nTrain > nTest)	{
			gdTrain = grid;		gdTest = 1;
		} else		{
			gdTest = grid;		gdTrain = 1;
		}
		for (j = 0; j < gdTrain; j++)			{
		//	sPath = (char*)Sample(i);		//only for debug
			if (iT < nTrain && i < nData)		TransTo(i++, train, iT++);
		}
		for (j = 0; j < gdTest; j++)			{
		//	sPath = (char*)Sample(i);		//only for debug
			if (iS < nTest && i < nData)			TransTo(i++, test, iS++);
		//	sPath = (char*)test->Sample(iS-1);		//only for debug
		}
		//	if( iS<nTest && i < nData)			TransTo( i++,test,iS++ );
		i += step;
	}
	for( i = iT; i < train->nMost; i++ )	train->tag[i]=-1;
	for( i = iS; i < test->nMost; i++ )		test->tag[i]=-1;
	nTrain = train->nSample();
	nTest = test->nSample();
	assert(nTrain + nTest <= nData);
	GST_util::print( ("SelectTT(%d,%d) from %d,grid=%d,last=%d\n"), nTrain, nTest, nData, grid, i);

	return nTrain + nTest;
}

void DATASET_cr::TransTo(int from, const hDATASET hTarget, int no, int nSample, int flag)	{
	DATASET_cr *hCR=dynamic_cast<DATASET_cr *>( hTarget);
	if( hCR!=nullptr )	{	
		hTarget->Copy(no, this, from, nSample, flag);	
		return;
	}
	if( 0 )	{
		DATASET_d *hD=dynamic_cast<DATASET_d *>( hTarget);
		if( hD==nullptr )	throw ("DATASET_cr::TransTo");
	}
	assert(from >= 0 && from < nMost && no >= 0 && no < hTarget->nMost);
	assert(ldX == hTarget->ldX);
	assert(nSample == 1);
	memcpy(hTarget->tag + no, tag + from, sizeof(int)*nSample);
	if( info!=nullptr && hTarget->info!=nullptr )
		memcpy(hTarget->info + no, info + from, sizeof(int)*nSample);		
	FLOA *toX = (FLOA *)hTarget->tX();
	if( toX==nullptr )	
		throw ("DATASET_cr::TransTo	--- toX is nullptr!!!");
	toX += (size_t)(no)*hTarget->ldX;
	int i, color;
	BIT_8 r, g, b, *fromX = X + (size_t)(from)*ldX;
	for (i = 0; i < ldX; i++)	{
//			toX[i] = 2*fromX[i]/255.0-1.0;
		toX[i] = fromX[i]/255.0;
	}	
//		hD->Stat( ("train"),0x0 );
	return;
}

double _Devia_8(BIT_8 *X, int ldX, int flag)	{
	double mean = 0.0, devia = 0.0, a0 = FLT_MAX, a1 = -FLT_MAX, a;
	int i;
	for (i = 0; i < ldX; i++)	{
//		a0 = min(a0, X[i]);		a1 = max(a1, X[i]);
		a0 = a0<X[i] ? a0:X[i];		a1 = a1>X[i] ? a1:X[i];
		mean += X[i];
	}
	mean /= ldX;
	for (i = 0; i < ldX; i++)	{ a = X[i] - mean;		devia += a*a; }
	devia = sqrt(devia / ldX);
	return devia;
}

void DataSample::Dump(string sTitle, int flag)	{
	int nSamp = nSample(),tag_0=INT_MAX,tag_1=INT_MIN,i,t;
	for( i=0; i<nSamp; i++ ){
		t=tag[i];
		tag_0=min(t,tag_0);			tag_1=max(t,tag_1);
	}
	GST_util::print(("%s(%dx%d) (%g:%g) mean=%g,devia=%g,TAG=[%d,%d]\r\n"), sTitle.c_str(), ldX,nSamp, x_0, x_1, mean, devia,tag_0,tag_1);
	if (nSamp < nMost)	{
		GST_util::print(("\t\tnMost=%d\r\n"), nMost);
	}
}

bool DATASET_cr::SubRC(int r0, int c0, hDATASET sub_0, int flag)	{
	DATASET_cr *sub = dynamic_cast<DATASET_cr *>(sub_0);
	if( sub==nullptr )	return false;
	assert(r0 >= 0 && r0 + sub->height() <= height() && c0 >= 0 && c0 + sub->width() <= width() );
	int r, ldw = sub->width(), pos;
	BIT_8 *cr = X, *cg = cr + ldX / 3, *cb = cg + ldX / 3, *sr = sub->X, *sg = sr + sub->ldX / 3, *sb = sg + sub->ldX / 3;
	for (r = r0; r < r0 + sub->height(); r++)	{
		pos = r*width() + c0;
		memcpy(sr, cr + pos, sizeof(BIT_8)*ldw);		sr += ldw;
		memcpy(sg, cg + pos, sizeof(BIT_8)*ldw);		sg += ldw;
		memcpy(sb, cb + pos, sizeof(BIT_8)*ldw);		sb += ldw;
	}
	return true;
}

/*
int DATASET_cr::Expand4Convol(hDATASET &hConv, int uW, int uH, int &conH, int &conW, int dt)	{
	conW = width() - uW + 1, conH = height() - uH + 1;
	int nUnit = (conW)*(conH), ldU = uW*uH * 3;
	int r, c, i, pos = 0;
	DATASET_cr sub(1, ldU);
	if (hConv == nullptr)	{
		hConv = new DATASET_d(nUnit, ldU, dt | DataSample::TAG_ZERO);
	} else
		assert(hConv->ldX == ldU && hConv->nMost == nUnit);
	double *unit = (double *)hConv->Sample(0);
	for (r = 0; r <= height() - uH; r++)	{
		for (c = 0; c <= width() - uW; c++)	{
			SubRC(r, c, &sub, 0x0);
			for (i = 0; i < ldU; i++)	unit[pos + i] = sub.X[i];
			pos += ldU;
		}
	}
	//	DataSample::sDumpFolder=("F:\\GiSeek\\trace\\Dataset\\");
	//	GST_bitmap::save_doublebmp_n( nUnit,0,uW,uH,unit,nUnit,GST_PIXEL_3 );
	return 0x0;
}


//#include "C:\\Program Files (x86)\\Intel\\Composer XE 2013\\ipp\\include\\ippi.h"
bool DATASET_cr::Convol(hDATASET hFilterBank, int *conH, int *conW, double *conv, int flag){
	int nFilter = hFilterBank->nMost, ldU = hFilterBank->ldX, uW = sqrt(ldU / 3.0), uH = ldU / 3.0 / uW, nUnit = 0;
	assert(uW <= width() && uH <= height());
	int r, c, i, pos = 0;
	DATASET_cr sub(1, ldU);
	*conW = width - uW;		*conH = height - uH;
	nUnit = (*conW)*(*conH);
	double *unit = new double[nUnit*ldU], *filter = (double *)hFilterBank->tX(), one(1.0), zero(0.0), dot;
	for (r = 0; r < height - uH; r++)	{
		for (c = 0; c < width - uW; c++)	{
			SubRC(r, c, &sub, 0x0);
			for (i = 0; i < ldU; i++)	unit[pos + i] = sub.X[i];
			pos += ldU;
		}
	}
	//	GST_bitmap::save_doublebmp_n( nUnit,0,uW,uH,unit,nUnit,GST_PIXEL_3 );
	GEMM(charT, charN, nUnit, nFilter, ldU, &one, unit, ldU, filter, ldU, &zero, conv, nUnit);
	delete[] unit;
	return true;
}*/

void DATASET_path::ImportFile( int no, const wchar_t *wPath,int tg,int flag ){
	char *sPath=GST_util::W2CHAR( wPath );
	int len=strlen(sPath);	assert( len<ldX );
	memcpy( X+ldX*no,sPath,sizeof(char)*len );
	tag[no]=tg;		assert(tg>=0);
}

void DATASET_cr::ImportFile( int no, const wchar_t *wPath,int tg,int flag ){
//	char *sPath=GST_util::W2CHAR( wPath );
//	int len=_tcslen(wPath);	assert( len<ldX );
//	double plign[8]={0.4};
	IMAGE_AUG aug(width(),height(),align );
		ImportImage( no,(void*)wPath,aug );		
		tag[no]=tg;		assert(tg>=0);			
//	memcpy( X+ldX*no,sPath,sizeof(char)*len );
}

void CopyBackGround( BIT_8 *dst,BIT_8 *src,SHAPE_IMAGE *si,int flag ){
	int	ldw=si->wth,height=si->hei,map=si->maps,bord=height/10,r,c,from,ldm=si->Pixel();
	memcpy( dst,src,sizeof(BIT_8)*si->Count() );
	BIT_8 *dstR=dst,*dstG=dst+ldm,*dstB=dst+ldm*2,*srcR=src,*srcG=src+ldm,*srcB=src+ldm*2,pos;
	for( r=height*0.2;r<height*0.8;r++){
		c = ldw/4;
		memset( dstR+r*ldw+c,0x0,sizeof(BIT_8)*ldw/2 );
		memset( dstG+r*ldw+c,0x0,sizeof(BIT_8)*ldw/2 );
		memset( dstB+r*ldw+c,0x0,sizeof(BIT_8)*ldw/2 );
		//for( c=ldw*0.3;c<ldw*0.7;c++){
		//	pos= G_RC2POS(r,c,ldw);
		//}
	/*	from=rand()%bord;
		memcpy( dstR+r*ldw,srcR+from*ldw,sizeof(BIT_8)*ldw );
		memcpy( dstG+r*ldw,srcG+from*ldw,sizeof(BIT_8)*ldw );
		memcpy( dstB+r*ldw,srcB+from*ldw,sizeof(BIT_8)*ldw );*/
	}

}

DATASET_cr::DATASET_cr( DATASET_path& uris,vector<RoCLS> &arrROI,int nSamp,int*nos,SHAPE_IMAGE *sp,int flag ): si(nullptr),
	DataSample_T<BIT_8>(nSamp,sp->Count(),GST_PIXEL_3, nullptr){
	assert(sp!=nullptr);		shape = make_shared<SHAPE_IMAGE>(*sp);
	si=dynamic_cast<SHAPE_IMAGE*>(shape.get());

	assert( nSamp<=uris.nSample() );
	align=uris.align;
	int width = si->wth, height = si->hei;
	//double plign[8]={0.4};
	int i,no,nBack=0;
	vector<int> vBack,vFore;
	wchar_t wPath[256*4];
	IMAGE_AUG aug(width,height,align );
	for( i=0; i < nSamp; i++ )	{
		no=nos[i];		assert( no>=0 );
		/*if( arrROI[no].p>0.8 ){
			memcpy( X+ldX*i,arrROI[no].hX,sizeof(BIT_8)*ldX );
		}	else*/		{
			char *path=(char *)uris.Sample(no);
			if( stricmp( path,sBackSrc.c_str() )==0){
				vBack.push_back(i);
			}else{
				GST_util::CHAR2W(path,wPath,0x0);
				ImportImage( i,wPath,aug );	
				vFore.push_back(no);
			}	
		}
		tag[i] = uris.tag[no];
	}
	if( vBack.size()>vFore.size() ){		//�޷���ȡ 
		Empty( 0x0 );
		return;
	}
	assert( vBack.size()<=vFore.size() );
	for ( i=0;i<vBack.size( ); i++ )	{
		no = vFore[i];		
		char *path=(char *)uris.Sample(no);		GST_util::CHAR2W(path,wPath,0x0);
		aug.align = IMAGE_AUG::IMAG_BACKGROUND;
		ImportImage( vBack[i],wPath,aug );	
	//	if( tag[vBack[i]]!=17 )
	//		throw "DATASET_cr tag[back] is XXX";
	}
}
int N_SAMP(DATASET_path& uris){
	int nSample=uris.nSample(),avg=uris.align== IMAGE_AUG::SAMP_4 ? 4 : 1;
	assert( avg>=1 && avg<10 );
	return nSample*avg;
}
DATASET_cr::DATASET_cr(int n, SHAPE_IMAGE *sp, int tp ) : DataSample_T<BIT_8>(n, sp->Count(), tp|GST_PIXEL_3, nullptr){
	assert(sp!=nullptr);		shape = make_shared<SHAPE_IMAGE>(*sp);
	si=dynamic_cast<SHAPE_IMAGE*>(shape.get());//width = si->wth, height = si->hei;
}
DATASET_cr::DATASET_cr( DATASET_path& uris,SHAPE_IMAGE *sp,int tp,int flag ) : 
	DataSample_T<BIT_8>( N_SAMP(uris),sp->Count(), tp|GST_PIXEL_3, nullptr){
	assert(sp!=nullptr);		shape = make_shared<SHAPE_IMAGE>(*sp);
	si=dynamic_cast<SHAPE_IMAGE*>(shape.get());

	align=uris.align;			avrage=align==IMAGE_AUG::SAMP_4? 4 : 1;
	int width = si->wth, height = si->hei;
	//double plign[8]={0.5};
	int i,j,tg,n=uris.nSample( );
	IMAGE_AUG aug(width,height,align );
	vector<RoCLS> arrROI;
	for( i=0; i < n; i++ )	{
		/*if( arrROI[i].p>0.9 ){
			FILE *fp=nullptr;
			if( fread( X+ldX*i,sizeof(BIT_8),ldX,fp )!=ldX )		throw "";
			fclose(fp);
		}else*/{
			char *path=(char *)uris.Sample(i);
			wchar_t *sPath=GST_util::CHAR2W(path);
		//	ImportImage( i*step,sPath,plign );
			tg = uris.tag[i];
			for( j = 0; j < avrage; j++ )	{
				aug.param[7] = j;			//plign[7] = j;
				ImportImage( avrage*i+j,sPath,aug );
				tag[avrage*i+j]=tg;		assert(tg>=0);
			}
		}
	}
}

IMAGE_AUG::IMAGE_AUG( int wM,int hM,TYPE t,int flag ) : wMap(wM),hMap(hM),align(t),L(0),R(-1),T(0),B(-1),ang(0)	{
	param[0]={0.4};
//	for( int i=0; i<8; i++ )
//		param[i]=p[i];
}
int UniformInt(const int & min, const int & max );
//0(oigin size)=>(L,R,T,B)=>(tW,tH)				MAP:ȡ����		tW<=wView,tH<=hView
void IMAGE_AUG::Augment( int wth_0,int hei_0,float xita,int flag )	{
	int dim=(align==IMAG_MAXFLOAT || align==IMAG_MAX_CENTER || align==SAMP_4 ) ? 
			(wth_0+hei_0)/4 : min(wth_0,hei_0)*param[0];
	int dW=min(wth_0/2,dim),dH=min(hei_0/2,dim),r=hei_0/2, c=wth_0/2;
	double s,s1;
//	tW=dW*s,tH=dH*s;
	switch( align ){
	case IMAG_ALL:
		s=min(wMap*1.0/wth_0,hMap*1.0/hei_0);
		L=0;	R=wth_0;	T=0;	B=hei_0;
		tW=wth_0*s;			tH=hei_0*s;
		break;
	case IMAG_CENTER:
	case IMAG_MAX_CENTER:
//		dib = FreeImage_RescaleRect( fib,tW,tH,c-dW,r-dH,c+dW,r+dH );
		L=c-dW,R=c+dW,T=r-dH,B=r+dH;
		break;
	case IMAG_FLOAT:
	case IMAG_MAXFLOAT:
	//	L = wth_0==dW*2 ? 0 : rand()%(wth_0-dW*2);		R=L+dW*2;	
	//	T = hei_0==dH*2 ? 0 : rand()%(hei_0-dH*2);		B=T+dH*2;
		L = wth_0==dW*2 ? 0 : UniformInt(0,wth_0-dW*2-1);		R=L+dW*2;	
		T = hei_0==dH*2 ? 0 : UniformInt(0,hei_0-dH*2-1);		B=T+dH*2;
		break;
	case IMAG_BACKGROUND:
		L = 0;	T=0;	
		if( wth_0>hei_0 )
		{	L=UniformInt(0,wth_0/2-1);	R=L+wth_0/2;		B=hei_0/3;		}
		else
		{	R=wth_0/3;		T=UniformInt(0,hei_0/2-1);		B=T+hei_0/2;	}
		tW = wMap;	tH=hMap;
		return;
		break;
	case RAND_SUB:
		dW=wMap*xita;		dH=hMap*xita;
		L = UniformInt(0,dW);	T=UniformInt(0,dH);	
		R = wMap-1-UniformInt(0,dW);	B=hMap-1-UniformInt(0,dH);			
		tW = wMap;	tH=hMap;
		return;
		break;
	case SAMP_4:
		if( param[7]<3 ){
			s1 = param[7]/2.0;
			L= (wth_0-dW*2)*s1;		R=L+dW*2;	
			T =(hei_0-dH*2)*s1;		B=T+dH*2;						
			//dib = FreeImage_RescaleRect( fib,tW,tH,left,top,left+dW*2,top+dH*2 );
		}else{
			s=min(wMap*1.0/wth_0,hMap*1.0/hei_0);
			tW=wth_0*s,			tH=hei_0*s;
			L=0,	R=tW,		T=0,	B=tH;
			//dib = FreeImage_RescaleRect( fib,tW,tH,0,0,wth,hei );
		}
		break;
	default:
		throw "DATASET_cr::align is XXX";
		break;
	}
	assert( R>L && B>T );
	s=min( wMap*1.0/(R-L),hMap*1.0/(B-T) );
	tW=(R-L)*s,tH=(B-T)*s;
	assert( tW<=wMap && tH<=hMap );
}

