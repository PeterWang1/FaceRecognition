//
//  GruFacer.h
//  GruFacer
//
//  Created by gao on 2017/3/21.
//  Copyright © 2017年 KK. All rights reserved.
//
#import "./GruST/GST_def.h"

#ifdef __cplusplus
extern "C" {
#endif
    int Facer_init( const char *,const char *,INT_32 f );
    void GruFacer_Test( BIT_8 * pixels,float * pFeats,int * info, int wth, int  hei,int  flag );
    void Facer_clear(  )	;
#ifdef __cplusplus
}
#endif
