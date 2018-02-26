//
//  getLuJIn.h
//  5ty
//
//  Created by 看看智能 on 2017/5/18.
//  Copyright © 2017年 sqliteLearning. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface getLuJIn : NSObject
@property(nonatomic,strong)NSString *documentsPath;
@property(nonatomic,strong)NSString *str_LUJIN_1;
//调用方法:放入文件名和文件类型 如rf_D.dat
/**
 rf_D 放入 fileName
 dat 放入 type
 */
-(NSString*)putTheFileName:(NSString*)fileName andType:(NSString*)type;
@end
