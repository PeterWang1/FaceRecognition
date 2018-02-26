#pragma once
#include <string>
#include "../GST_def.h"
using namespace std;

#define SERIAL_i(isRead,fp,num )  if(isRead) FREAD_i(num,fp) else FWRIT_i(num,fp);
#define SERIAL_arr(isRead,fp,arr,size,count )  if(isRead) FREAD_arr(arr,size,count,fp) else FWRIT_arr(arr,size,count,fp);

//maybe bug!!!
#define SERIAL_string(isRead,fp,str )  {											\
	int len=str.size();		SERIAL_i(isRead,fp,len );								\
	if( isRead )	{	str.resize(len);	FREAD_arr(&str[0],sizeof(char),len,fp);		}	\
	else	FWRIT_arr(str.c_str(),sizeof(char),len,fp)	\
}
template <typename T> void SERIAL_( bool isRead,FILE *fp,T *num ){
	if(isRead){
		GST_VERIFY( fread( num,sizeof(T),1,(fp) )==1,"" );
	}	else{
		GST_VERIFY( fwrite( num,sizeof(T),1,(fp) )==1,"" );
	}
}

/*
	v0.1	12/21/2015	基于FILE *fp，适用于c-type arr
*/
class SERIAL{
private:
	string sPath;
	int status;
	FILE *fp;
	bool isRead;

public:
	enum{
		OK=0x0
	};
	/*static SERIAL& Open( const string &sPath,bool isRead,int flag=0x0 ){
		return SERIAL( sPath,isRead,flag );
	}*/
	SERIAL( const string &sP,bool isR,int flag=0x0 ) : fp(NULL),sPath(sP),status(0),isRead(isR)	{
		fp = isRead ? fopen(sPath.c_str(), ("rb")):fopen(sPath.c_str(), ("wb"));
		if( fp==NULL ) 
			GST_THROW( "SERIAL - open is failed!" );
	}
	virtual ~SERIAL( ){
		sPath.empty() ;			fp=NULL;
	}

	int Close( ){
		if (fp != NULL && fclose(fp) != 0)	{		status = -7;	}
		if (status == 0)	{
		} else	{
			status = fp != NULL ? ferror(fp) : status;
		}
		return status;
	}

	template <typename cls> SERIAL& OBJ( cls &obj ){
		obj.Serial( *this );		
		return *this;
	}
	template <typename cls> SERIAL& ARR( cls *arr,int count ){
		size_t sz=sizeof(cls);		
		if(isRead) {
			FREAD_arr(arr,sz,count,fp) 
		}	 else	{
			FWRIT_arr(arr,sz,count,fp);	
		}
		return *this;
	}

	SERIAL& CHARs( string &str )  {											
		int len=str.size();		
		T( len );								
		if( isRead )	{	
			str.resize(len);	
			FREAD_arr(&str[0],sizeof(char),len,fp);		
		}	else	{
			FWRIT_arr(str.c_str(),sizeof(char),len,fp);
		}
		return *this;
	}

	template <typename cls> SERIAL& T( cls &num ){
		size_t sz=sizeof(cls);		
		if(isRead){
			GST_VERIFY( fread( &num,sz,1,(fp) )==1,"" );
		}	else{
			GST_VERIFY( fwrite( &num,sz,1,(fp) )==1,"" );
		}
		return *this;
	}
};
