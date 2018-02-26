#pragma once

#include <memory>
#include <iostream>
#include <algorithm>
#include <random>
#ifdef WIN32
    #include <tchar.h>
	#include <assert.h>
#else    
	#define assert(cond)
#endif
#include <time.h>
#include <fstream>
#include <vector>
#include "../GST_def.h"
#include "../basic/GST_fno.hpp"
#include "GST_datashape.hpp"
//#include "GST_image.hpp"
//#include "..\mat\GVMAT_t.hpp"
//#include "..\mat\Matrix.hpp"
//#include "..\util\GST_util.hpp"

#ifdef FLOA_64BIT
	typedef double	FLOA;
#else
	typedef float FLOA;		//���Ƚϵ�,�ݶȼ������ϴ�.
#endif
typedef FLOA*	FLOA_ARR;


struct DATASET_cr;

/*
	Dataset��Matrix�������ƣ�Ҳ�������κϲ���		 10/19/2014		cys
	*/
struct DataSample;
typedef DataSample* hDATASET;

struct DataSample{
	static std::string sDumpFolder,sLastPicPath;
	static const string sBackSrc;

	string nam;
	enum{
		TAG_ZERO = 0x10, CHECK_DEVIA = 0x20,
		PREC_BASIC = 0x100,
		TRANS=0x200,
		DIM_MOST=0x10000,		//����ΪnMost��tag��Ч
	};
	enum{
		MARK_0=0x0,
		MARK_REFER=0x100,
	};

	IMAGE_AUG::TYPE align;
	hSHAPE shape;
	int nMost,ldX,*tag,*info,type,avrage;
	double x_0, x_1, mean, devia, rOK;		//Accuracy-the proportion of correctly classified
	double *eMean;	//mean for each element

	DataSample() : nMost(0),ldX(0),shape(nullptr),type(0),mean(0),devia(0),x_0(-DBL_MAX),x_1(DBL_MAX),rOK(0.0),tag(nullptr),info(nullptr),eMean(nullptr),avrage(1){}
	DataSample(int n, int ld, int tp = 0x0) : nMost(n), ldX(ld),shape(nullptr), type(tp), mean(0), devia(0), x_0(-DBL_MAX), x_1(DBL_MAX), rOK(0.0),
		info(nullptr),tag(nullptr),eMean(nullptr),avrage(1)	{
	}
	DataSample(int n, hSHAPE shp, int tp = 0x0) : nMost(n), ldX(shp->Count()),shape(nullptr), type(tp), mean(0), devia(0), x_0(-DBL_MAX), x_1(DBL_MAX), rOK(0.0),
		info(nullptr),tag(nullptr),eMean(nullptr),avrage(1)	{
	}
	virtual ~DataSample()	{ ; }
	virtual void Empty( int type ){
		if( tag!=nullptr )	
		{	for( int i=0; i<nMost;	i++ )	tag[i]=-1;	}
		mean=0, devia=0, x_0=-DBL_MAX, x_1=DBL_MAX, rOK=0.0;
	}

	virtual bool isValidShape( )	{	return shape!=nullptr;	}

	virtual void Stat(string sTitle, int flag = 0x0)	{ throw "DataSample::Stat unimplemented!!!"; }

	virtual void Sigma_Scal(int alg, int flag)			//���������ڵ�������ŵ�[0.1-0.9]
	{		throw "DataSample::Sigma_Scal unimplemented!!!";		}
	virtual bool Preprocess(int alg, int flag)	{	Sigma_Scal(alg, 0x0);	return true;	}
//	virtual bool Preprocess(double *trans, double *mean, double *temp, int alg, int flag = 0x0);

	virtual int Shrink(int nTo, int flag)		{ throw "DataSample::Shrink unimplemented!!!"; }
	int SelectTT(hDATASET train, hDATASET test, int type, int flag);
	int SelectTo(hDATASET train, vector<int>& nEach,bool isMove=false, int flag=0x0);
	int nSample() const	{
		if (nMost <= 0)	return 0;
		int i;
		for (i = 0; i < nMost && tag[i] >= 0; i++);
		return i;
	}

	virtual void ImportFile( int no, const wchar_t *sPath,int tg,int flag=0x0 )	{ throw "DataSample::ImportFile unimplemented!!!"; }
	virtual bool SubRC(int r, int c, hDATASET sub, int flag){ throw "DataSample::Sub unimplemented!!!"; }
	virtual void *tX() const	{ throw "DataSample::X unimplemented!!!"; }
	virtual void *Sample(int k) const	{ throw "DataSample::Sample unimplemented!!!"; }
	virtual void Copy(int i, const hDATASET hS, int no, int nSample = 1, int flag = 0x0)	{ throw "DataSample::Sample unimplemented!!!"; }
	virtual void TransTo(int from, const hDATASET hTarget, int no, int nSample = 1, int flag = 0x0) { throw "DataSample::TransTo unimplemented!!!"; }
//	virtual void TransTo(int from, const hDATASET hTarget, int no, DPP_LINE &dppLine,int nSample = 1, int flag = 0x0) { throw exception("DataSample::TransTo_DPP_LINE unimplemented!!!"); }

	virtual int load_fp(FILE *fp, int flag)				{ throw "DataSample::load_fp unimplemented!!!"; }
	virtual int save_fp(FILE *fp, int flag)				{ throw "DataSample::save_fp unimplemented!!!"; }
	virtual void Dump(string sTitle, int flag);
	virtual int Load(const std::string sPath, int flag);
	virtual int Load(const std::wstring sPath, int flag);
	virtual int Save(const std::wstring sPath, int flag);
	virtual int Save(const std::string sPath, int flag);
	virtual int ToBmp(int epoch, int _x = 0, int flag = 0x0)	{ throw "DataSample::ToBmp unimplemented!!!"; }
//	virtual int ToFolder(string sRoot, int type, int flag = 0x0);

	enum{
		NORMAL_FILTERBANK = 100,
	};
	virtual double Normal(int type, int flag)	{ throw "DataSample::Normal unimplemented!!!"; }
};

//ע�� Standard deviation�ļ���ȼ���Matlab std(A,1)
template<typename T> int STA_distribute(int dim, T *X, double&mean, double&devia, double& x_0, double &x_1, double &nrm2, int flag = 0x0)	{
	x_0=DBL_MAX, x_1=-DBL_MAX;		mean=0.0, devia=0.0;		nrm2=0.0;
	if (dim == 0)
		return 0;
	int i;
	for (i = 0; i < dim; i++)	{
		x_0 = MIN(x_0, X[i]);		x_1 = MAX(x_1, X[i]);
		mean += X[i];		nrm2+=X[i]*X[i];
	}
	mean /= dim;
	double a;
	for (i = 0; i < dim; i++)	{
		a = X[i] - mean;		devia += a*a;
	}
	devia = sqrt(devia / dim);
//	if (1)		nrm2 = NRM2(dim, X);
	nrm2 = sqrt(nrm2);
	return 0x0;
}
template<typename T> 
int STA_distribute(int dim, T *X, int ldX,double&mean, double&devia,int flag = 0x0)	{
	mean = 0.0, devia = 0.0;
	if (dim == 0)
		return 0;
	int i;
	for (i = 0; i < dim; i++)	mean += X[i*ldX];
	mean /= dim;
	double a;
	for (i = 0; i < dim; i++)	{
		a = X[i*ldX] - mean;		devia += a*a;
	}
	devia = sqrt(devia / dim);
	return 0x0;
}

template<typename T> 
int STA_distribute(int dim, T *X_0, double&mean, double&devia,int flag = 0x0)	{
	mean = 0.0, devia = 0.0;
	if (dim == 0)
		return 0;
	double a;
	T *x=X_0,a2=0.0;
	int i;
	DOT(a2,dim,X_0,X_0);
	for (i = 0; i < dim; i++,x++)	{
		mean += *x;		//a2+=(*x)*(*x);
	}
	mean /= dim;
	a = a2-dim*mean*mean;	assert(a>=0);
	devia = sqrt(a / dim);
	return 0x0;
}

template<typename T>
class DataSample_T : public DataSample {
public:
	T *X;

	virtual void *tX() const	{ return X; }
	virtual void *Sample(int k) const	{ assert(k >= 0 && k < nMost);	return X + ldX*k; }

	DataSample_T() : X(nullptr){}
	DataSample_T( hDATASET src,int flag=0x0 ): DataSample(src->nMost,src->ldX,src->type) { 
		tag = new int[nMost]();				info=new int[nMost]();
		for (int i = 0; i < nMost; i++)	tag[i] = -1;
		size_t sz=(size_t)(nMost)*ldX;
		X = new T[sz]();
		if( src->nSample()>0 )		
			Copy( 0,src,0,src->nSample(),flag );
	}
//	DataSample_T(int n, int ld, int tp = 0x0, T *x = nullptr) : DataSample(n, ld, tp), X(x){
	DataSample_T(int n, int ld, int tp = 0x0, T *x = nullptr) : DataSample(n, ld, tp), X(x){
	try{		//		nMost=n,	ldX=ld,	type=tp,tag=nullptr,	X=x,	rOK=0.0;
		assert(nMost > 0 && nMost<INT_MAX/8 );
		tag = new int[nMost]();				info=new int[nMost]();
		if (!BIT_TEST(type, TAG_ZERO))	{
			for (int i = 0; i < nMost; i++)	tag[i] = -1;
		}
		if (x == nullptr)	{
			size_t sz=(size_t)(nMost)*ldX;
			X = new T[sz]();
		} else		{
			X = x;
		}
	}catch( ... )	{
		tag=nullptr;		X=nullptr;
		throw "DataSample_T is XXX";
	}
	}

	virtual ~DataSample_T()	{ Clear(); }
	virtual void Clear(){
		delete[] tag;		delete[](X);
		if( info!=nullptr )			delete[] info;
		if( eMean!=nullptr )		
			delete[] eMean;
		tag = nullptr;		X = nullptr;
		ldX = 0;				nMost = 0;
	}
	virtual void Empty( int type ){
		DataSample::Empty( type );
		if(X!=nullptr)	 memset( X,0x0,sizeof(T)*ldX*nMost );
	}

	virtual int load_fp(FILE *fp, int flag)	{
		if (fread(&nMost, sizeof(int), 1, fp) != 1)		{	return -10;		}
		if (fread(&ldX, sizeof(int), 1, fp) != 1)		{	return -11;		}
		if( tag!=nullptr )		{	delete[] tag;		tag=nullptr;	}
		tag = new int[nMost];
		if (fread(tag, sizeof(int), nMost, fp) != nMost)
		{			return -12;		}
		if( X!=nullptr )		{	delete[] X;		X=nullptr;	}
		size_t sz=nMost*(size_t)(ldX);
		X = new T[sz];
		if (fread(X, sizeof(T), sz, fp) != sz)
		{			return -13;		}
		if( info!=nullptr )		{	delete[] info;		info=nullptr;	}	// 8/19/2015
		info = new int[nMost];
		if (fread(info, sizeof(int), nMost, fp) != nMost)
		{			return -14;		}
		return 0x0;
	}
	virtual int save_fp(FILE *fp, int flag)	{
		int dim = nSample(),i;//
		if( BIT_TEST(flag,DIM_MOST) )
			dim = nMost;
		if (fwrite(&dim, sizeof(int), 1, fp) != 1)
		{			return -10;		}
		if (fwrite(&ldX, sizeof(int), 1, fp) != 1)
		{			return -11;		}
		if (fwrite(tag, sizeof(int), dim, fp) != dim)
		{			return -12;		}
		size_t sz,sz_1=(size_t)(dim)*ldX,ldSec=100000000,nSec=(int)ceil(sz_1*1.0/ldSec ),pos=0;
		for(i = 0; i < nSec; i++,pos+=ldSec )	{
			sz=min(sz_1,pos+ldSec)-pos;		assert(sz>0);
			if (fwrite(X+pos, sizeof(T), sz, fp) != sz )			{
				return -13;
			}
		}
		if (fwrite(info, sizeof(int), dim, fp) != dim)
		{			return -14;		}
		return 0x0;
	}

	virtual int Shrink(int nTo, int flag)	{
		int nFrom = nSample(), grid = nFrom / nTo, i, pos;
		if (grid <= 1)
			return nFrom;
		for (i = 0; i < nTo; i++)	{
			pos = i*grid;
			tag[i] = tag[pos];
			memcpy(X + ldX*i, X + ldX*pos, ldX*sizeof(T));
		}
		for (i = nTo; i < nFrom; i++)		tag[i] = -1;
		return nTo;
	}

	//	virtual void Copy( int i,const DataSample_T<T> &hS,int no,int nSample=1,int flag=0x0 )	{	
	virtual void Copy(int to, const hDATASET hS, int from, int nSample = 1, int flag = 0x0)	{
#if defined __APPLE__
        throw "Copy is ... in __APPLE__";
#else
		assert(to >= 0 && to < nMost && from >= 0 && from < hS->nSample());
		DATASET_cr *hCR=dynamic_cast<DATASET_cr*>(hS);
		if( hCR!=nullptr ){
			BIT_8 *pixel=(BIT_8 *)(hS->tX() )+ldX*from;
			T *X_1 = X+to*ldX;
			int k,nz=nSample*ldX;
			for( k=0; k<nz; k++ )	{
				X_1[k]=pixel[k];
			}
		}else{
			assert(ldX == hS->ldX);
			T *fromX = (T*)hS->tX();
			size_t pS=ldX*(size_t)(to),pF=ldX*(size_t)(from);
			memcpy(X + pS, fromX + pF, ldX*sizeof(T)*nSample);
		}
		memcpy(tag + to, hS->tag + from, sizeof(int)*nSample);
		if( info!=nullptr && hS->info!=nullptr )
			memcpy(info + to, hS->info + from, sizeof(int)*nSample);
#endif
		return;
	}

	virtual void Stat(string sTitle, int flag)	{
		double nrm2;
		int nSam=nSample(),i,j;
		STA_distribute( nSam*ldX, X, mean, devia, x_0, x_1, nrm2);	
		T *cur=X;
		if( eMean==nullptr )	{
			eMean=new double[ldX]();
			for( i = 0; i < nSam; i++,cur+=ldX )	{
				for( j=0; j < ldX; j++ )	eMean[j]+=cur[j];
			}
			for( i = 0; i < ldX; i++ )	eMean[i]/=nSam;
			//if( ldX==128*128*3 )	SAVE_BMP_N( 1,1100,128,128,eMean,0,GST_PIXEL_3 );
		}
		Dump(sTitle, flag);
	}

	void _mean(T *x, int flag)	{
		int i;
		T a0 = FLT_MAX, a1 = -FLT_MAX, mean = 0.0;
		for (i = 0; i < ldX; i++)	{
			a0 = min(a0, x[i]);		a1 = max(a1, x[i]);
			mean += x[i];
		}
		mean /= ldX;
		for (i = 0; i < ldX; i++)	x[i] -= mean;
	}
	//���������ڵ�������ŵ�[0.1-0.9]
	virtual void Sigma_Scal(int alg, int flag)	{
		int i, nz = ldX*nSample();
		T *x = X;
//		T a0 = (T)1.0e100, a1 = -(T)1.0e100;
		T a0 = x[0],		a1 = x[0];
		mean = 0.0, devia = 0.0;
		for (i = 0; i < nz; i++)	{
			a0 = min(a0, x[i]);		a1 = max(a1, x[i]);
			mean += x[i];
		}
		x_0 = a0;		x_1 = a1;
		mean /= nz;
		for (i = 0; i < nz; i++)	{
			x[i] -= mean;		devia += x[i] * x[i];
		}
		devia = sqrt(devia / nz);
		double d3 = 3.0*devia, a;
		x_0 = DBL_MAX;		x_1 = -DBL_MAX;
		mean = 0.0;
		for (i = 0; i < nz; i++)	{
			x[i] = MIN(d3, x[i]);		x[i] = MAX(-d3, x[i]);
			a = x[i] / d3;
			x[i] = (a + 1.0)*0.4 + 0.1;
			assert(x[i] >= 0.1 && x[i] <= 0.9);
			x_0 = MIN(x_0, x[i]);		x_1 = MAX(x_1, x[i]);
			mean += x[i];
		}
		mean /= nz;
	}

	virtual void TransTo(int from, const hDATASET hTarget, int no, int nSample=1, int flag=0x0)	{	
		DataSample_T<float> *hFloat=dynamic_cast<DataSample_T<float> *>(hTarget);
		T *fromX = X + from*ldX;
		int i,nz=nSample*ldX;
		memcpy(hTarget->tag + no, tag + from, sizeof(int)*nSample);
		if( hFloat!=nullptr ){	//DATASET_cr::TransTo�뱣��һ��
			float *toX=(float *)(hFloat->X)+no*ldX;
			for (i = 0; i < nz; i++)	{
				toX[i] = fromX[i]/255.0;
			}
		}else
			hTarget->Copy(no, this, from, nSample, flag);	
	}
};
typedef DataSample_T<double> DATASET_d;
typedef DataSample_T<float> DATASET_f;

template<typename T>
T* TO(const hDATASET hDat, int sample = 0) 	{
	if (typeid(T) == typeid(double))	{
#ifdef _DEBUG
		if (dynamic_cast<DATASET_d*>(hDat) == nullptr)
			throw bad_cast("TO DATASET_d");
#endif
		return (T*)(hDat->Sample(sample));
	}
	else if (typeid(T) == typeid(BIT_8))	{
#ifdef _DEBUG
		if (dynamic_cast<DATASET_cr*>(hDat) == nullptr)
			throw bad_cast("TO DATASET_bit8");
#endif
		return (T*)(hDat->Sample(sample));
	}
	else if (typeid(T) == typeid(float))	{
		if (dynamic_cast<DATASET_f*>(hDat) == nullptr)
			throw ("TO DATASET_f");
		return (T*)(hDat->Sample(sample));
	}
	throw ("TO(const hDATASET hDat)");
}

struct DataReg : public DATASET_d{
	int nCls;
	double *p;		//probability

	DataReg(int n, int ld, int nC, int tp = 0x0, double *x = nullptr)
		: DATASET_d(n, ld, tp, x), nCls(nC), p(nullptr)	{
		p = new double[nCls*n];
	}
	virtual ~DataReg()	{
		delete[] p;
	}

	virtual int save_fp(FILE *fp, int flag)	{
		int ret = DATASET_d::save_fp(fp, flag);
		if (ret = 0x0 && fwrite(&nCls, sizeof(int), 1, fp) != 1)
		{
			return -100;
		}
		return ret;
	}
	virtual int load_fp(FILE *fp, int flag);
};

//ר�����ļ�Ŀ¼
struct DATASET_path : public DataSample_T<char> {
public:
	DATASET_path(int n, int ld, int tp = 0x0) : DataSample_T<char>(n, ld, tp, nullptr){
	}
	virtual int ToBmp(int epoch, int _x = 0x0, int flag = 0x0){	return 0x0;}
	virtual void ImportFile( int no, const wchar_t *sPath,int tg,int flag=0x0 );
	virtual void Stat(string sTitle, int flag){		Dump(sTitle, flag);}
};

//ר����ͼƬ
class DATASET_cr : public DataSample_T < BIT_8 > {
public:
	//int width, height;
	SHAPE_IMAGE *si;
public:	
	int width( )	{	return si->wth;		}
	int height( )	{	return si->hei;		}
	DATASET_cr( DATASET_path& uris,vector<RoCLS> &arrROI,int nSamp,int*nos,SHAPE_IMAGE *sp,int flag=0x0 );
	DATASET_cr( DATASET_path& uris,SHAPE_IMAGE *sp,int tp=0x0,int flag=0x0 );
	DATASET_cr(int n, SHAPE_IMAGE *sp, int tp = 0x0);

	DATASET_cr(int n, int ld, int tp = 0x0) : DataSample_T<BIT_8>(n, ld, tp|GST_PIXEL_3, nullptr){
		throw "DATASET_cr(int n, int ld, int tp = 0x0) is XXX";//width = (int)(sqrt(ldX / 3.0)), height = (int)(ldX / 3.0 / width);
	}
	virtual int ToBmp(int epoch, int _x = 0x0, int flag = 0x0);

	void ToHSV( int flag=0x0 );
	virtual void ImportFile( int no, const wchar_t *sPath,int tg,int flag=0x0 );
	bool ImportImage(int no, void *src,IMAGE_AUG&augment,int flag=0x0);
//	bool ImportImage(int no, void *src,double *plign=nullptr,int flag=0x0);
	virtual void TransTo(int i, const hDATASET hTarget, int no, int nSample = 1, int flag = 0x0);
//	virtual void TransTo(int from, const hDATASET hTarget, int no, DPP_LINE &dppLine,int nSample = 1, int flag = 0x0);

	virtual bool Convol(hDATASET hFilterBank, int *conW, int *conH, double *conv, int flag){	throw "DATASET_cr::Convol is ...";			}
	virtual int Expand4Convol(hDATASET &hConv, int uW, int uH, int &conH, int &conW, int flag){	throw "DATASET_cr::Expand4Convol is ...";	}

	virtual bool SubRC(int r, int c, hDATASET sub, int flag);
//	virtual void Stat(string sTitle, int flag = 0x0)	{ Dump(sTitle, flag); }
	virtual void Sigma_Scal(int alg, int flag)	{	throw "DATASET_cr::Sigma_Scal unimplemented!!!";	}
};

struct GST_rand {
	mt19937 g;
	//GST_rand(): g(static_cast<uint32_t>(time(0)))	{	}

	GST_rand( int seed ): g(static_cast<uint32_t>(seed+100))	{	}
	size_t operator()(size_t n)	{
		std::uniform_int_distribution<size_t> d(0, n ? n-1 : 0);
		return d(g);
	}
};

