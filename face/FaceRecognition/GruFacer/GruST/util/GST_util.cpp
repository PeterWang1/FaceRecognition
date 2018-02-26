#include "GST_util.hpp"
#include <algorithm>
using namespace std;

int GST_util::dump=0x0;
int GST_util::id_0=0x0;
double GST_util::tX=0.0;
int GST_util::verify=0;

#ifdef _DEBUG
	int GST_util::isArrOut=1;
#else
	int GST_util::isArrOut=0x0;
#endif

#ifdef _WIN64
#include "mat.h"
#include "matrix.h"
#pragma message("\t\tMATLAB:	R2012b \r\n")
//#pragma comment( lib, "G:\\MATLAB\\R2012b\\extern\\lib\\win64\\microsoft\\libmat.lib" )
//#pragma comment( lib, "G:\\MATLAB\\R2012b\\extern\\lib\\win64\\microsoft\\libmx.lib" )
#else
//#pragma comment( lib, "G:\\MATLAB\\R2011a\\extern\\lib\\win32\\microsoft\\libmat.lib" )
//#pragma comment( lib, "G:\\MATLAB\\R2011a\\extern\\lib\\win32\\microsoft\\libmx.lib" )
#endif

/*
	ע�⣺
		1 ��mat�ļ���Ҫ�ܶ�����DLL��32λ�汾����֧��	12/5/2014
*/
int GST_util::LoadDoubleMAT( const char *sPath,const char* sVar,int *nRow,int *nCol,double **val,int flag )	{
#ifdef _WIN64
/*	MATFile *pMat;
	mxArray *problem,*A;
	mxClassID cls;
	double *Ax, *Az;
	int nz,*Ap,*Ai;
	
	pMat = matOpen( sPath, "r");
	if (pMat == NULL) {
		printf("Error reopening file %s\n", sPath );
		return -101;
	}
	problem = matGetVariable(pMat, sVar );
	if (problem == NULL) {
		printf("Error reading existing matrix Problem\n");
		return -102;
	}
	cls = mxGetClassID(problem) ;
	*nRow=mxGetM (problem ) ;		*nCol=mxGetN ( problem ) ;
	if ( *nRow<=0 || *nCol<=0 ) {
		printf("Error get element A\n");
		return -103;
	}
	Ax = mxGetPr(problem) ;			Az = mxGetPi (problem);
	nz = (*nRow)*(*nCol);
	*val = new double[nz];
	memcpy( *val,Ax,sizeof(double)*nz );
	mxDestroyArray(problem);
	if (matClose(pMat) != 0) {
		printf("Error closing file %s\n",sPath );
		return -104;
	}*/
#endif
	return 0x0;
}


#ifdef WIN32
	#include <windows.h>
	#include <time.h>
#endif


bool GST_util::isToFile=false;
FILE *GST_util::fpDump=NULL,*GST_util::fpPrint=NULL;
#define MAX_WBUFFER 1000
wchar_t wBuffer[MAX_WBUFFER];

wchar_t *GST_util::CHAR2W( const char *buffer )	{
	if( buffer==NULL )
		throw "GST_util::CHAR2W -- str is NULL!!!";

	int len = strlen( buffer );
	if( len>0 )	{
#ifdef WIN32
	len = ::MultiByteToWideChar(CP_UTF8, 0, buffer,len,wBuffer, 1000 );
#else
	throw "GST_util::CHAR2W is ...";
		//len = UTF16FromUTF8(cDesc.c_str(), len, line, GoodsParser::MAX_LINE);
#endif
	}
	wBuffer[len]='\0';
	return wBuffer;
}
void GST_util::CHAR2W( const char *buffer,wchar_t *wstr,int wlen )	{
	if( buffer==NULL )		throw "GST_util::CHAR2W -- str is NULL!!!";
	int len = strlen( buffer );
	if( len>0 )	{
#ifdef WIN32
	len = ::MultiByteToWideChar(CP_UTF8, 0, buffer,len,wstr, 1000 );
#else
		throw "GST_util::CHAR2W is ...";
	//len = UTF16FromUTF8(cDesc.c_str(), len, line, GoodsParser::MAX_LINE);
#endif
	}
	wstr[len]='\0';
	return ;
}
char *GST_util::W2CHAR(const wchar_t *wtxt )	{
	if (wtxt == NULL)
		throw "GST_util::W2CHAR -- str is NULL!!!";
	char *buffer = (char *)wBuffer;
#ifdef WIN32
	int len = _tcslen(wtxt);
	if (len>0)	{
		len = ::WideCharToMultiByte(CP_UTF8, 0, wtxt, len, buffer, 1000,0,0);
		buffer[len] = '\0';
	}
#else
		throw "GST_util::W2CHAR";
#endif


	return buffer;
}

void GST_util::print( const char *format,... )	{	
	va_list args;
	va_start( args, format );
//	vfprintf( stderr,format,args );
	char *buffer=(char*)wBuffer;
#ifdef WIN32
	_vsnprintf( buffer,MAX_WBUFFER,format,args );
#else
	sprintf( buffer,format,args );
#endif
	va_end(args);
	if( dump!=0 )
		printf( "%s",buffer );
	if( fpPrint!=NULL ){
		if( buffer[0]=='\r' && buffer[1]!='\n')
			return;
		fprintf( fpPrint,("%s"),buffer );
	}
	if( isToFile )	{
		assert( fpDump!=NULL );
		if( buffer[0]=='\r' && buffer[1]!='\n')
			return;
		fprintf( fpDump,("%s"),buffer );
	}
}
void GST_util::wprint( const wchar_t *format,... )	{	
//	if( dump==0 )
//		return;
	va_list args;
	va_start( args, format );
#ifdef WIN32
	_vsntprintf( wBuffer,MAX_WBUFFER,format,args );
	va_end(args);
	if( dump!=0 )
		_tprintf( wBuffer );
	if( fpPrint!=NULL ){
		if( wBuffer[0]=='\r' && wBuffer[1]!='\n')
			return;
		_ftprintf( fpPrint,_T("%s"),wBuffer );
	}
	if( isToFile )	{
		assert( fpDump!=NULL );
		if( wBuffer[0]=='\r' && wBuffer[1]!='\n')
			return;
		_ftprintf( fpDump,_T("%s"),wBuffer );
	}
#else
	throw "GST_util::wprint is ...";
#endif

}

void GST_util::fprint( FILE *fp,const wchar_t *format,... )	{	
try{
#ifdef WIN32
	if( dump==0 )
		return;
	va_list args;
	va_start( args, format );
	_vsntprintf( wBuffer,MAX_WBUFFER,format,args );
//	_tprintf( wBuffer );
	va_end(args);
	if( wBuffer[0]=='\r' && wBuffer[1]!='\n')
		return;
	assert( fp!=nullptr );
	_ftprintf( fp,_T("%s"),wBuffer );
#else
	throw "GST_util::fprint is ...";
#endif
}catch( ... )	{
}
}

wchar_t *GST_util::wFormat( const wchar_t *format,... ){
#ifdef WIN32
	va_list args;
	va_start( args, format );
	_vsntprintf( wBuffer,MAX_WBUFFER,format,args );
	_tprintf( wBuffer );
	va_end(args);

	return wBuffer;
#else
	throw "GST_util::wFormat is ...";
#endif
}
char *GST_util::Format( const char *format,... ){
	va_list args;
	va_start( args, format );
#ifdef WIN32
	_vsnprintf( (char *)wBuffer,MAX_WBUFFER,format,args );
#else
	throw "GST_util::Format is ...";
#endif
	printf( "%s",(char *)wBuffer );
	va_end(args);

	return (char *)wBuffer;
}


char *GST_util::TimeStr( int flag ){
	wBuffer[0]='\0';
#ifdef WIN32
	time_t      szClock;
	time( &szClock );
	char *str=asctime(localtime(&szClock));
	str[strlen(str)-1]='\0';		//���һ��Ϊ\n
	return str;
#else
	return (char*)wBuffer;
#endif
}

bool GST_util::BeginWith( const std::string& str_0,const std::string& str_1,int flag )	{
#ifdef WIN32
	std::string s1=str_1,s0=str_0;
	std::transform( s1.begin(),s1.end(),s1.begin(),toupper );
	std::transform( s0.begin(),s0.end(),s0.begin(),toupper );

	if( s0.compare( 0,s1.size(),s1 )==0 )
		return true;
	return false;
#else
	throw "GST_util::BeginWith is ...";
#endif
}

Ran_Vigna_1024::Ran_Vigna_1024( int seed ) : p(0)	{	
	int n32=sizeof(uint64_t)/sizeof(int)*16,*pn=(int*)s;
	for( int i=0;i<n32;i++)	pn[i]=seed+i;	
}