//活体检测第二版
//
//  ViewController.m
//  Hello
//
//  Created by gao on 2017/3/22.
//  Copyright © 2017年 KK. All rights reserved.
//
/*
 人脸特征点定位
 v0.1    cys         3/10/2016
 输入：
 byte[] frame            onPreviewFrame传入的每帧图像数据
 int width, int height   图像的宽度，高度
 int flag                暂定为0
 
 输出：
 float[] feat        长度为256的float数组
 feat[0-135]     记录68个特征点的位置,第i个特征点的x-y坐标为 feat[i-1],feat[i-1+68]
 注意：前17个特征点总是为0！坐标系的原点在图片的左下角
 
 int[] result        长度为64的int数组
 result[9]           =1 检测到人脸特征点，否则feat数组无意义
 result[0,1,2,3]		图像中的人脸位置
 result[8]		    图像中的人脸个数， result[8]<0，未检测到人脸
 */
#import "ViewController.h"
#import "AppDelegate.h"
#import "./GruFacer/GruFacer.h"
#import <UIKit/UIKit.h>
#import <CoreImage/CoreImage.h>
#import <ImageIO/ImageIO.h>
#define Mask8(x) ( (x) & 0xFF)
#define R(x) ( Mask8(x) )
#define G(x) ( Mask8(x >> 8 ) )
#define B(x) ( Mask8(x >> 16) )
#define A(x) ( Mask8(x >> 24) )
#define RGBAMake(r, g, b, a) ( Mask8(r) | Mask8(g) << 8 | Mask8(b) << 16 | Mask8(a) << 24)
@interface ViewController ()

@property(nonatomic,strong)NSString *documentsPath;
@property(nonatomic,strong)NSString *documentsPath_2;
@property(nonatomic,strong)NSString *documentsPath_3;
@property(nonatomic,strong)NSString *str_LUJIN_1;
@property(nonatomic,strong)NSString *str_LUJIN_2;

//是否通过活体检测的算法
@property(nonatomic)BOOL isSuccess;
@end

@implementation ViewController
typedef enum {
    ALPHA = 0,
    BLUE = 1,
    GREEN = 2,
    RED = 3
} PIXELS;
-(NSString *)documentsPath{
    if (!_documentsPath) {
//        _documentsPath=NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
        _documentsPath =NSSearchPathForDirectoriesInDomains(NSCachesDirectory,NSUserDomainMask,YES)[0];
    }
    return _documentsPath;
}
-(NSString *)documentsPath_2{
    if (!_documentsPath_2) {
//        _documentsPath_2=NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
          _documentsPath_2 =NSSearchPathForDirectoriesInDomains(NSCachesDirectory,NSUserDomainMask,YES)[0];
    }
    return _documentsPath_2;
}
-(NSString *)documentsPath_3{
    if (!_documentsPath_3) {
//        _documentsPath_3=NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
          _documentsPath_3 =NSSearchPathForDirectoriesInDomains(NSCachesDirectory,NSUserDomainMask,YES)[0];
    }
    return _documentsPath_3;
}
-(double)putImage:(UIImage *)kImage andL:(const char*)str1 andL2:(const char*)str2
{
    NSString *sourcePath=[[NSBundle mainBundle] pathForResource:@"rf_D" ofType:@"dat"];
    //创建用于写入的目标文件
    NSString *targetPath=[self.documentsPath stringByAppendingPathComponent:@"rf_D.dat"];
    BOOL success=[[NSFileManager defaultManager] createFileAtPath:targetPath contents:nil attributes:nil];
    if (success) {
        NSLog(@"目标空dat文件创建成功");
    }else{
        NSLog(@"目标空dat文件创建失败");
    }
    self.str_LUJIN_1 =   [self duquLujin:sourcePath andLujin:targetPath];
    const char *luJin_1 =[self.str_LUJIN_1 UTF8String];
//    -----------------------------------------------------------------------------------
    //    -----------------------------------------------------------------------------------
    //    -----------------------------------------------------------------------------------
    
    //读取dat文件-2
    NSString *sourcePath_2=[[NSBundle mainBundle] pathForResource:@"rf_T" ofType:@"dat"];
    //创建用于写入的目标文件
    NSString *targetPath_2=[self.documentsPath_2 stringByAppendingPathComponent:@"rf_T.dat"];
    BOOL success_2=[[NSFileManager defaultManager] createFileAtPath:targetPath_2 contents:nil attributes:nil];
    if (success_2) {
        NSLog(@"目标空dat文件2创建成功");
    }else{
        NSLog(@"目标空dat文件2创建失败");
    }
    
    
    self.str_LUJIN_2 =  [self duquLujin:sourcePath_2 andLujin:targetPath_2];
    const char *luJin_2 =[self.str_LUJIN_2 UTF8String];
    //    -----------------------------------------------------------------------------------
    //    -----------------------------------------------------------------------------------
    //    -----------------------------------------------------------------------------------
    /**
     彩色图片转换成灰度的图片
     //网上找的方法调用_1
         self.imageV.image =    [self convertToGrayscale];
     */
    //网上找的方法调用_2
    //   kImage =[ViewController getGrayImage:kImage];
    //kImage= [ViewController image:kImage rotation:UIImageOrientationLeft ];
    
    /**
     这里把uiimage转换uint8_t
     //网上找的方法调用_1
     uint8_t *rgbaPixel =  [self convertUIImageToBitmapRGBA8:self.imageV.image];
     */
    //网上找的方法调用_2
    
    //  uint8_t *rgbaPixel =  [self   pixelBRGABytesFromImage:self.imageV.image];
    
    
    NSData *testData = UIImagePNGRepresentation(kImage);
    Byte *testByte = (Byte *)[testData bytes];
    
    for(int i=0;i<[testData length];i++){
        //       printf("testByte = %d\n",testByte[i]);
        //        [array_1 addObject:testByte[i]];
    }
    int hei=360,wth=270,r,c;
    uint8_t *rgbaPixel =  [self convertUIImageToBitmapRGBA8:kImage],*row0;
    
    float a[256] ={0};
    int resultz[64] = {0};
    
    /**
     活体检测的接口
     */
    
    NSString *sourcePath_3333=[[NSBundle mainBundle] pathForResource:@"1" ofType:@"bmp"];
    //创建用于写入的目标文件
    NSString *targetPath_3333=[self.documentsPath_3 stringByAppendingPathComponent:@"1_hjt.bmp"];
    BOOL success_3333=[[NSFileManager defaultManager] createFileAtPath:targetPath_3333 contents:nil attributes:nil];
    if (success_3333) {
        NSLog(@"目标空dat文件2创建成功");
    }else{
        NSLog(@"目标空dat文件2创建失败");
    }
    //   [self duquLujin:sourcePath_3333 andLujin:targetPath_3333];
    //获取沙盒路径，
    //    NSMutableArray *arr = [[NSMutableArray alloc] init];
    //     NSData *_data = UIImageJPEGRepresentation(kImage, 1.0f);
    //     NSString *strimage64 = [_data base64EncodedDataWithOptions:b];
    //
    //    NSData *_decodedImageData = [[NSData alloc]initWithBase64EncodedString:strimage64 options:NSDataBase64DecodingIgnoreUnknownCharacters];
    //    UIImage *_decodedImage = [UIImage imageWithData:_decodedImageData];
    //
    
    [UIImagePNGRepresentation(kImage) writeToFile:targetPath_3333 atomically:YES];
    NSLog(@"------------------%@",targetPath_3333);
    //     [[NSUserDefaults standardUserDefaults]setValue:targetPath_3333 forKey:@"LUJI_PICTURE"];
    Facer_init(str1,str2,0x0);
    //void GruFacer_Test( BIT_8 * pixels,float * feats,int * info, int wth, int  hei,int  flag ){
    GruFacer_Test(   [self  processUsingPixels:kImage],a,resultz,kImage.size.height,kImage.size.width,0x0 );
    Facer_clear();
//    遍历
        for(int i = 0; i < 64; i++)
        {
            printf("===========results====\n");
            printf("%d   ", resultz[i]);
        }
        //遍历
        for(int i = 0; i < 256; i++)
        {
            printf("===========feat====\n");
            printf("%f   ", a[i]);
        }
    //遍历
    if (resultz[9] == 1) {
        printf("检测到人脸特征点");
        printf("检测到人脸个数:%d",resultz[8]);
    }
    
    //取到上嘴唇的嘴巴:
    /*
     52 为上嘴唇
     58 为下嘴唇
     49 为左边嘴唇的点
     55 为右边嘴唇的点
     公式: mouthWidthF = rightX - leftX < 0 ? abs(rightX - leftX) : rightX - leftX;
     mouthHeightF = lowerY - upperY < 0 ? abs(lowerY - upperY) : lowerY - upperY;
     //初始化的嘴唇的宽高
     abs:为绝对值
     
     mouthWidth = rightX - leftX < 0 ? abs(rightX - leftX) : rightX - leftX;
     mouthHeight = sqrt(lowerX*lowerX +upperX *upperX -2*upperX*lowerX +lowerY*lowerY +upperY *upperY -2*upperY*lowerY);
     float ihsd =mouthHeight / faceHeight;
     
     *************************************************************************************************************************************
     //必须都为竖直坐标:
     sqrt 为平方
     abs(58) * abs(58) + abs(52) * abs(52) -2 *abs(52)* abs(58) +abs(126) * abs(126) + abs(120) * abs(120) -2 *abs(120) * abs(126) / abs(result[3] - result[2])
     **/
    float mouthWidthF = fabs(a[119] -a[125]) / fabs(a[42] -a[39]);
    NSLog(@"mouthWIDTHF:%lf",mouthWidthF);
    return mouthWidthF;
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

- (unsigned char *) convertUIImageToBitmapRGBA8:(UIImage *) image {
    
    CGImageRef imageRef = image.CGImage;
    
    // Create a bitmap context to draw the uiimage into
    CGContextRef context = [self newBitmapRGBA8ContextFromImage:imageRef];
    
    if(!context) {
        
        return NULL;
        
    }
    
    size_t width = CGImageGetWidth(imageRef);
    
    size_t height = CGImageGetHeight(imageRef);
    
    CGRect rect = CGRectMake(0, 0, width, height);
    
    // Draw image into the context to get the raw image data
    
    CGContextDrawImage(context, rect, imageRef);
    
    // Get a pointer to the data
    unsigned char *bitmapData = (unsigned char*)CGBitmapContextGetData(context);
    
    // Copy the data and release the memory (return memory allocated with new)
    
    size_t bytesPerRow = CGBitmapContextGetBytesPerRow(context);
    
    size_t bufferLength = bytesPerRow * height;
    
    unsigned char *newBitmap = NULL;
    
    if(bitmapData) {
        newBitmap = (unsigned char *)malloc(sizeof(unsigned char) * bytesPerRow * height);
        
        if(newBitmap) {    // Copy the data
            
            for(int i = 0; i < bufferLength; ++i) {
                
                newBitmap[i] = bitmapData[i];
                
            }
            
        }
        free(bitmapData);
    } else {
        
        NSLog(@"Error getting bitmap pixel data\n");
        
    }
    CGContextRelease(context);
    return newBitmap;
    
}

- (CGContextRef) newBitmapRGBA8ContextFromImage:(CGImageRef) image {
    CGContextRef context = NULL;
    CGColorSpaceRef colorSpace;
    uint32_t *bitmapData;
    size_t bitsPerPixel = 32;
    size_t bitsPerComponent = 8;
    size_t bytesPerPixel = bitsPerPixel / bitsPerComponent;
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    size_t bytesPerRow = width * bytesPerPixel;
    size_t bufferLength = bytesPerRow * height;
    colorSpace = CGColorSpaceCreateDeviceRGB();
    if(!colorSpace) {
        NSLog(@"Error allocating color space RGB\n");
        return NULL;
        
    }
    
    
    
    // Allocate memory for image data
    
    bitmapData = (uint32_t *)malloc(bufferLength);
    
    
    
    if(!bitmapData) {
        
        NSLog(@"Error allocating memory for bitmap\n");
        
        CGColorSpaceRelease(colorSpace);
        
        return NULL;
        
    }
    
    
    
    //Create bitmap context
    
    
    
    context = CGBitmapContextCreate(bitmapData,
                                    
                                    width,
                                    
                                    height,
                                    
                                    bitsPerComponent,
                                    
                                    bytesPerRow,
                                    
                                    colorSpace,
                                    
                                    kCGImageAlphaPremultipliedLast);    // RGBA
    
    if(!context) {
        
        free(bitmapData);
        
        NSLog(@"Bitmap context not created");
        
    }
    
    
    
    CGColorSpaceRelease(colorSpace);
    
    
    
    return context;
    
}
//把图片变成灰度图

//- (UIImage *)convertToGrayscale {
//    CGSize size = self.imageV.image.size;
//    int width = size.width;
//    int height = size.height;
//
//    // the pixels will be painted to this array
//    uint32_t *pixels = (uint32_t *) malloc(width * height * sizeof(uint32_t));
//
//    // clear the pixels so any transparency is preserved
//    memset(pixels, 0, width * height * sizeof(uint32_t));
//
//    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
//
//    // create a context with RGBA pixels
//    CGContextRef context = CGBitmapContextCreate(pixels, width, height, 8, width * sizeof(uint32_t), colorSpace,
//                                                 kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedLast);
//
//    // paint the bitmap to our context which will fill in the pixels array
//    CGContextDrawImage(context, CGRectMake(0, 0, width, height), self.imageV.image.CGImage);
//
//    for(int y = 0; y < height; y++) {
//        for(int x = 0; x < width; x++) {
//            uint8_t *rgbaPixel = (uint8_t *) &pixels[y * width + x];
//
//            // convert to grayscale using recommended method: http://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale
//            uint32_t gray = 0.3 * rgbaPixel[RED] + 0.59 * rgbaPixel[GREEN] + 0.11 * rgbaPixel[BLUE];
//
//            // set the pixels to gray
//            rgbaPixel[RED] = gray;
//            rgbaPixel[GREEN] = gray;
//            rgbaPixel[BLUE] = gray;
//        }
//    }
//
//    // create a new CGImageRef from our context with the modified pixels
//    CGImageRef image = CGBitmapContextCreateImage(context);
//
//    // we're done with the context, color space, and pixels
//    CGContextRelease(context);
//    CGColorSpaceRelease(colorSpace);
//    free(pixels);
//
//    // make a new UIImage to return
//    UIImage *resultUIImage = [UIImage imageWithCGImage:image];
//
//    // we're done with image now too
//    CGImageRelease(image);
//
//    return resultUIImage;
//}

//uiimage 转换成 usign char
//- (unsigned char *)pixelBRGABytesFromImage:(UIImage *)image {
//    return [self pixelBRGABytesFromImageRef:image.CGImage];
//}

//- (unsigned char *)pixelBRGABytesFromImageRef:(CGImageRef)imageRef {
//
//    NSUInteger iWidth = CGImageGetWidth(imageRef);
//    NSUInteger iHeight = CGImageGetHeight(imageRef);
//    NSUInteger iBytesPerPixel = 4;
//    NSUInteger iBytesPerRow = iBytesPerPixel * iWidth;
//    NSUInteger iBitsPerComponent = 8;
//    unsigned char *imageBytes = malloc(iWidth * iHeight * iBytesPerPixel);
//
//    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
//
//    CGContextRef context = CGBitmapContextCreate(imageBytes,
//                                                 iWidth,
//                                                 iHeight,
//                                                 iBitsPerComponent,
//                                                 iBytesPerRow,
//                                                 colorspace,
//                                                 kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
//
//    CGRect rect = CGRectMake(0 , 0 , iWidth , iHeight);
//    CGContextDrawImage(context , rect ,imageRef);
//    CGColorSpaceRelease(colorspace);
//    CGContextRelease(context);
//    CGImageRelease(imageRef);
//
//    return imageBytes;
//}

//彩色图片转灰度方法2
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

//旋转90
+ (UIImage *)image:(UIImage *)image rotation:(UIImageOrientation)orientation

{
    
    long double rotate = 0.0;
    
    CGRect rect;
    
    float translateX = 0;
    
    float translateY = 0;
    
    float scaleX = 1.0;
    
    float scaleY = 1.0;
    
    
    
    switch (orientation) {
            
        case UIImageOrientationLeft:
            
            rotate = M_PI_2;
            
            rect = CGRectMake(0, 0, image.size.height, image.size.width);
            
            translateX = 0;
            
            translateY = -rect.size.width;
            
            scaleY = rect.size.width/rect.size.height;
            
            scaleX = rect.size.height/rect.size.width;
            
            break;
            
        case UIImageOrientationRight:
            
            rotate = 3 * M_PI_2;
            
            rect = CGRectMake(0, 0, image.size.height, image.size.width);
            
            translateX = -rect.size.height;
            
            translateY = 0;
            
            scaleY = rect.size.width/rect.size.height;
            
            scaleX = rect.size.height/rect.size.width;
            
            break;
            
        case UIImageOrientationDown:
            
            rotate = M_PI;
            
            rect = CGRectMake(0, 0, image.size.width, image.size.height);
            
            translateX = -rect.size.width;
            
            translateY = -rect.size.height;
            
            break;
            
        default:
            
            rotate = 0.0;
            
            rect = CGRectMake(0, 0, image.size.width, image.size.height);
            
            translateX = 0;
            
            translateY = 0;
            
            break;
            
    }
    
    
    
    UIGraphicsBeginImageContext(rect.size);
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    //做CTM变换
    
    CGContextTranslateCTM(context, 0.0, rect.size.height);
    
    CGContextScaleCTM(context, 1.0, -1.0);
    
    CGContextRotateCTM(context, rotate);
    
    CGContextTranslateCTM(context, translateX, translateY);
    
    
    
    CGContextScaleCTM(context, scaleX, scaleY);
    
    //绘制图片
    
    CGContextDrawImage(context, CGRectMake(0, 0, rect.size.width, rect.size.height), image.CGImage);
    
    
    
    UIImage *newPic = UIGraphicsGetImageFromCurrentImageContext();
    
    
    
    return newPic;
    
}

//新



- (UInt8 *)processUsingPixels:(UIImage *)image{
    
    //1.获得图片的像素 以及上下文
    UInt32 *inputPixels,nz=0;
    CGImageRef inputCGImage = [image CGImage];
    size_t w = CGImageGetWidth(inputCGImage);
    size_t h = CGImageGetHeight(inputCGImage);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    NSInteger bytesPerPixel = 4;//每个像素的字节数
    NSInteger bitsPerComponent = 8;//每个组成像素的 位深
    NSInteger bitmapBytesPerRow = w * bytesPerPixel;//每行字节数
    
    inputPixels = (UInt32 *)calloc(w * h , sizeof(UInt32));//通过calloc开辟一段连续的内存空间
    //inputPixels = new UInt32[w*h];//通过calloc开辟一段连续的内存空间
    
    CGContextRef context = CGBitmapContextCreate(inputPixels, w, h, bitsPerComponent, bitmapBytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    
    CGContextDrawImage(context, CGRectMake(0, 0, w, h), inputCGImage);
    
    UInt8 *pixels=new UInt8[h*w];
    //2操作像素
    for (NSInteger j = 0; j < h; j ++) {
        for (NSInteger i = 0 ; i < w; i ++) {
            UInt32 *currentPixel = inputPixels + (w * j) + i;
            UInt32 color = *currentPixel;
            
            //灰度图（举例）
            UInt32 averageColor = (R(color) + G(color) + B(color)) / 3.0;
            //printf("%d,%d,%d => %d",R(color) , G(color) , B(color),averageColor );
            pixels[nz++]=averageColor;
        }
    }
    //3从上下文中取出
    CGImageRef newImageRef = CGBitmapContextCreateImage(context);
    UIImage *newImage = [UIImage imageWithCGImage:newImageRef];
    
    //4释放
    CGColorSpaceRelease(colorSpace);
    CGContextRelease(context);
    free(inputPixels);
    
    return pixels;
}








@end
