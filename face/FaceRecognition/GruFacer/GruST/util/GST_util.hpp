#pragma once

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#ifdef WIN32
    #include <xstring>
    #include <tchar.h>
#else
	#include <string>
    #include <wchar.h>
#endif
#include <stdarg.h>


class GST_util{
	static bool isToFile;
	static FILE *fpDump;
public:
	static FILE *fpPrint;
	static int dump,id_0,isArrOut;
	static int verify;
	static double tX;

	enum{
		VERIRY_KRYLOV_FORM=1,
		VERIRY_UNITARY_MAT=2,VERIRY_I_MAT=2,VERIRY_TRI_MAT=2,
	};
	//���̰߳�ȫ������
	static wchar_t* CHAR2W( const char *buffer );							
	//�̰߳�ȫ������
	static void CHAR2W( const char *buffer,wchar_t *wstr,int wlen=0 );		
	static char* W2CHAR( const wchar_t *buffer);

	static bool BeginDumpToFile( const char *sPath,int type=0x0,int flag=0x0 )	{
		if( (fpDump=fopen( sPath,("w") ))!=NULL ) 
			isToFile=true;
		return isToFile;
	}
	static bool EndDumpToFile( )	{
		assert( isToFile==true && fpDump!=NULL );
		fclose( fpDump );
		isToFile=false;		fpDump=NULL;
		return true;
	}

	static void print( const char *format,... );
	static void wprint( const wchar_t *format,... );
	static void fprint( FILE *fp,const wchar_t *format,... );

	static int LoadDoubleMAT( const char *sPath,const char* sVar,int *nRow,int *nCol,double **val,int flag );
	
/*	template<typename T>
	static void Output( char *sPath,int m,int n,T *dat,int flag=0x0 )	{
		std::ofstream file;
		file.open( sPath, ios::out );	
		if( file.is_open( ) )	{
			hGMAT hMat= make_shared<Matrix<Z>>(m,n,dat,GeMAT::DATA_REFER );
			file<< hMat.get() << endl;		
			file.close( );
		}
	}*/

	static char *TimeStr( int flag=0x0 );
	static wchar_t *wTimeStr( int flag=0x0 )	{	return CHAR2W(TimeStr(flag) );}

	static wchar_t *wFormat( const wchar_t *format,... );
	static char *Format( const char *format,... );

	template <typename T>
	static int ARR_ouput( const std::string sPath,T* val,int nRow,int nCol,bool isRowMajor=true,int flag=0x0 )	{	
		return ARR_ouput( CHAR2W(sPath.c_str()),val,nRow,nCol,isRowMajor,flag );
	}
	template <typename T>
	static int ARR_ouput( const wchar_t* sPath,T* val,int nRow,int nCol,bool isRowMajor=true,int flag=0x0 )	{	//�������
	try{
	#ifdef WIN32
			if(isArrOut<=0)			return isArrOut;
			double mean,devia,x_0,x_1,nrm2;
			STA_distribute( nRow*nCol,val,mean,devia,x_0,x_1,nrm2 );
			int r,c,pos;
			FILE* fp=_tfopen( sPath,_T("w") );
			if( fp==NULL )		
				throw "ARR_ouput::fp is 0";
			for( r = 0; r < nRow; r++ )	{
				for( c = 0; c < nCol; c++ )	{
					pos = isRowMajor ? r*nCol+c : c*nRow+r;
					_ftprintf( fp,_T("%16g\t"),val[pos] );
				}
				_ftprintf( fp,_T("\n") );
			}
			fclose( fp );			
	#endif
			return 0x0;
	}catch(...)	{	return -1;	}
	}

	static bool BeginWith( const std::string& str_0,const std::string& str_1,int flag=0x0 );
};

//#define PATH_dat_0( title,x )	(GST_util::print( "F:\\GiSeek\\trace\\%s_%d.dat",title,x ))
#define wpDAT_0( title,x )	(GST_util::wFormat( _T("F:\\GiSeek\\trace\\%s_%d.dat"),title,x ))

#include <stdint.h>
//http://xorshift.di.unimi.it/xorshift1024star.c
struct Ran_Vigna_1024 {
	uint64_t s[16]; 
	int p;	
	Ran_Vigna_1024( int seed );
	uint64_t next(void) { 
		uint64_t s0 = s[ p ];
		uint64_t s1 = s[ p = ( p + 1 ) & 15 ];
		s1 ^= s1 << 31; // a
		s1 ^= s1 >> 11; // b
		s0 ^= s0 >> 30; // c
		return ( s[ p ] = s0 ^ s1 ) * 1181783497276652981LL; 
	}
};