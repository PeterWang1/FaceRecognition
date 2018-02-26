#ifndef _GST_DEF_H_
#define _GST_DEF_H_

//#define __APPLE__

#include <stdint.h>
#include <math.h>
typedef uint8_t BIT_8;
typedef uint64_t BIT_64;
typedef uint32_t BIT_32;
typedef int32_t INT_32;
#ifdef _WINDOWS
    #define GST_THROW(s) {													\
        char sInfo[1024]="exception";										\
        sprintf_s<1024>(sInfo, "\n@%s(%d): %s\n",__FILE__,__LINE__,s);		\
        throw sInfo;														\
    }
#else
    #define GST_THROW(s)    {   throw s;	   }
#endif

#ifdef WIN32
	#include <assert.h>
	#define GST_VERIFY(isTrue,info) {	if(!(isTrue)) {	assert(0);	GST_THROW(info);}	}
	#define LOGP printf
#else
	#define GST_VERIFY(isTrue,info) {	if(!(isTrue)) {		GST_THROW(info);}	}
#endif



//�ƺ���std::min std::max���ʺϱȽϲ�ͬ���������		7/11/2015	cys
#undef MAX
#undef MIN
#define MAX(a, b)    ( (a)>(b)?(a):(b) )
#define MIN(a, b)    ( (a)<(b)?(a):(b) )

//#define FLOA_64BIT

typedef int (*ON_TRAVEL_wpath_)(void* user_data,const wchar_t *sPath,int flag);

#define GST_OK 0x0

#define PI 3.1415926535897932384626433832795
#define EXP_UNDERFLOW	(-708)
#define EXP_OVERFLOW	(709)
#define IOFFE_epsi (1.0e-12)

#if defined (WIN32)
	/* Yes, this is exceedingly ugly.  Blame Microsoft, which hopelessly */
	/* violates the IEEE 754 floating-point standard in a bizarre way. */
	/* If you're using an IEEE 754-compliant compiler, then x != x is true */
	/* iff x is NaN.  For Microsoft, (x < x) is true iff x is NaN. */
	/* So either way, this macro safely detects a NaN. */
	#define IS_NAN(x)	(((x) != (x)) || (((x) < (x))))
	#define IS_ZERO(x)	(((x) == 0.) && !SCALAR_IS_NAN(x))
	#define IS_NONZERO(x)	(((x) != 0.) || SCALAR_IS_NAN(x))
	#define IS_LTZERO(x)	(((x) < 0.) && !SCALAR_IS_NAN(x))
//http://jacksondunstan.com/articles/983
	#define IS_NAN_INF(x)	( (x)*0!=0 )
	#define IS_FLOAT(x)		( (x)*0==0 )
#else
/* These all work properly, according to the IEEE 754 standard ... except on */
/* a PC with windows.  Works fine in Linux on the same PC... */
	#define SCALAR_IS_NAN(x)	((x) != (x))
	#define SCALAR_IS_ZERO(x)	((x) == 0.)
	#define SCALAR_IS_NONZERO(x)	((x) != 0.)
	#define SCALAR_IS_LTZERO(x)	((x) < 0.)
#endif


//λ����
#define BIT_FLAG_1				0x00000001
#define BIT_FLAG_H				0x80
#define BIT_SET( val,flag ) ((val) |= (flag))	
#define BIT_RESET( val,flag ) ((val) &= (~(flag)) ) 
#define BIT_TEST( val,flag ) (((val)&(flag))==(flag))
#define BIT_IS( val,flag ) (((val)&(flag))!=0)

//(r,c) <==> pos	����������		5/30/2008
#define G_RC2POS( r,c,ld )	((r)*(ld)+(c))
#define G_POS2R( pos,ld )		((pos)/(ld))
#define G_POS2C( pos,ld )		((pos)%(ld))
#define G_RC_VALID( r,r_0,r_1 )	((r)>=(r_0) && (r)<=(r_1) )
#define G_RC_NORMAL( r,r_0,r_1 )	{(r)=MAX((r),(r_0));(r)=MIN((r),(r_1)); }

#define G_MINMAX(y,y0,y1)			{(y1)=MAX((y),(y1));(y0)=MIN((y),(y0)); }

//���ؾ�a�����������(int)����(��ȥС���)		2/11/2010	cys
//#define G_DOUBLE2INT(a)			(int)( (a)+0.5 )
#define G_DOUBLE2INT(a)			(int)( a>=0.0 ? (a)+0.5 : (a)-0.5 )
#define G_DOUBLE2BYTE(a)			( (a)>255.5 ? 0xFF : (G_U8)((a)+0.5) )
#define G_RADIEN2DEGREE( a )	(int)( (a)<0.0 ? (6.283185307179+(a))*57.295779513082+0.5 : (a)*57.295779513082+0.5 )
//ʱ��
#ifdef CHRONO_LIB
	#include <chrono>
	#define GST_TIC(tick)	std::chrono::thread_clock::time_point tick = std::chrono::thread_clock::now( );
	#define GST_TOC(tick)	( (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::thread_clock::now( )-(tick)).count( ))/1000.0)
#else
	#include <time.h>
	#define GST_TIC(tick)	clock_t tick=clock( );
	#define GST_TOC(tick)	((clock()-(tick))*1.0f/CLOCKS_PER_SEC)
#endif
#define G_CLOCK2SEC(a)			( (a)*1.0/CLOCKS_PER_SEC )
#define G_TV2SEC(a,b)			( ((a).tv_sec-(b).tv_sec + (double)((a).tv_usec-(b).tv_usec)/1.0e6) )

#define I2STR( i )				(std::to_string(_ULonglong(i)).c_str())

/****	�ļ���д	****/
#define FREAD_i(num,fp)	{ GST_VERIFY( fread( &(num),sizeof(int),1,(fp) )==1,"" );	}
#define FWRIT_i(num,fp) { GST_VERIFY( fwrite( &(num),sizeof(int),1,(fp) )==1,"" );	}	
#define FREAD_arr(arr,size,count,fp) { GST_VERIFY( fread( (arr),(size_t)(size),(size_t)(count),(fp) )==((size_t)count),"" );	}
#define FWRIT_arr(arr,size,count,fp) { GST_VERIFY( fwrite( (arr),(size_t)(size),(size_t)(count),(fp))==((size_t)count),"" );	}

/*	��buffer */
//#define FREAD_i(num,fp) if( (iRet=fread( &(num),sizeof(int),1,(fp) ))!=1 )	throw -104;
#define READ_buf(dst,size,pb) {		memcpy((dst),(pb),(size) );	pb+=size;		}

typedef enum{
	CPU_MODE=0x0,		//cpu mode
	GPU_MODE,		//gpu mode
	HPU_MODE
}GST_PROCESS_UNIT;

//���ֲ��
typedef enum {
	MESER_L2=0x1,MESER_COS=10,
}GST_MEASURE;

#ifdef _WINDOWS
	#define LOGE(msg) printf(msg);
	#define LOGI(msg) printf(msg);
	#define LOGD(msg) printf(msg);
	#define LOGP(...)
#elif defined ANDROID
    #define ASSERT(expression)
    #define assert(expression)
    #define stricmp strcasecmp
	#define strnicmp strncasecmp

    #include <jni.h>
    #include <android/log.h>

    #define LOGE(msg) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, msg)
    #define LOGI(msg) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, msg)
    #define LOGD(msg) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, msg)
    extern INT_32 gd_level;
    #define LOGP(...) { if(gd_level>0) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__);}
#elif defined __APPLE__
    #define DBL_MAX MAXFLOAT
    #define FLT_MAX MAXFLOAT

    #define ASSERT(expression)
    #define assert(expression)
    #define stricmp strcasecmp
    #define strnicmp strncasecmp


    #define LOGE(msg) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, msg)
    #define LOGI(msg) //__android_log_print(ANDROID_LOG_INFO, LOG_TAG, msg)
    #define LOGD(msg) //__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, msg)
    extern INT_32 gd_level;
    //#define LOGP(...) //{ if(gd_level>0) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__);}
    #define LOGP printf
#endif

#endif
