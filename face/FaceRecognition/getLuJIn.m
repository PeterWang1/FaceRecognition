//
//  getLuJIn.m
//  5ty
//
//  Created by 看看智能 on 2017/5/18.
//  Copyright © 2017年 sqliteLearning. All rights reserved.
//

#import "getLuJIn.h"

#import "./GruFacer/GruFacer.h"
#import <UIKit/UIKit.h>
#import <CoreImage/CoreImage.h>
#import <ImageIO/ImageIO.h>

@implementation getLuJIn

-(NSString *)documentsPath{
    if (!_documentsPath) {
        _documentsPath=NSSearchPathForDirectoriesInDomains(NSCachesDirectory,NSUserDomainMask,YES)[0];
    }
    return _documentsPath;
}


-(NSString *)putTheFileName:(NSString *)fileName andType:(NSString *)type
{
    NSString *str =[fileName stringByAppendingFormat:@".%@",type];
    NSString *sourcePath=[[NSBundle mainBundle] pathForResource:fileName ofType:type];
    //创建用于写入的目标文件
    NSString *targetPath=[self.documentsPath stringByAppendingPathComponent:str];
    BOOL success=[[NSFileManager defaultManager] createFileAtPath:targetPath contents:nil attributes:nil];
    if (success) {
        NSLog(@"目标文件创建成功");
    }else{
        NSLog(@"目标文件创建失败");
    }
    
    
    
    self.str_LUJIN_1 =   [self duquLujin:sourcePath andLujin:targetPath];
//    const char *luJin_1 =[self.str_LUJIN_1 UTF8String];
    return self.str_LUJIN_1;

}
-(NSString*)duquLujin:(NSString *)str1 andLujin:(NSString *)str2
{
    NSFileHandle *sourceReadHandle=[NSFileHandle fileHandleForReadingAtPath:str1];
    //写目标文件控制器
    NSFileHandle *targetWriteHandle=[NSFileHandle fileHandleForWritingAtPath:str2];
    //标识: 是否读取完毕
    BOOL notEnd = YES;
    //存储当前已读取的数量
    NSInteger readSize = 0;
    NSInteger sizePerTime = 5000;
    NSDictionary *attr=[[NSFileManager defaultManager] attributesOfItemAtPath:str1 error:nil];
    NSLog(@"attr %@",attr);
    NSNumber *fileSize=[attr objectForKey:NSFileSize];
    NSInteger fileToltalNum=fileSize.longValue;
    int count=0;
    while (notEnd) {
        //      计算还剩下多少没有读完
        NSInteger leftSize=fileToltalNum-readSize;
        NSData *data=nil;
        //    如果剩下的超过5000，则读5000。 如果少于5000 则读到结尾
        if (leftSize >= sizePerTime) {
            // 剩余大于5000，则都读5000
            data=[sourceReadHandle readDataOfLength:sizePerTime];
            //       修改已读数量
            readSize += sizePerTime;
            //       移动读取源文件的指针到新的位置
            [sourceReadHandle seekToFileOffset:readSize];
        }else{
            //          设置已读完
            notEnd = NO;
            //       将剩余数据读出来
            data=[sourceReadHandle readDataOfLength:leftSize];
        }
        [targetWriteHandle writeData:data];
        count++; //记录读取的次数
    }
    NSLog(@"共读取了%d次,路径%@",count, str2);
    
    return str2;
    
}

+ (UIImage*)getGrayImage:(UIImage*)sourceImage
{
    int width = sourceImage.size.width;
    int height = sourceImage.size.height;
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
    CGContextRef context = CGBitmapContextCreate (nil,width,height,8,0,colorSpace,kCGImageAlphaNone);
    CGColorSpaceRelease(colorSpace);
    
    if (context == NULL) {
        return nil;
    }
    
    CGContextDrawImage(context,CGRectMake(0, 0, width, height), sourceImage.CGImage);
    CGImageRef grayImageRef = CGBitmapContextCreateImage(context);
    UIImage *grayImage = [UIImage imageWithCGImage:grayImageRef];
    CGContextRelease(context);
    CGImageRelease(grayImageRef);
    
    return grayImage;
}


@end
