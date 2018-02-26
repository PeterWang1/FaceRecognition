//
//  vieconasdViewController.m
//  Hello_Test_2
//
//  Created by 看看智能 on 2017/5/19.
//  Copyright © 2017年 sqliteLearning. All rights reserved.
//

#import "vieconasdViewController.h"
#import "ViewController.h"
#import "getLuJIn.h"
@interface vieconasdViewController ()
@property(nonatomic,strong)NSString *strrrrr;
@end

@implementation vieconasdViewController

- (void)viewDidLoad {
    [super viewDidLoad];


    for (int i = 0;  i < 30; i ++) {
        __weak __typeof(self)weakSelf = self;
        __block const char *c_1 , *c_2;
        dispatch_queue_t queue =  dispatch_queue_create("zy", NULL);
        
        // 将任务添加到队列中
        dispatch_sync(queue, ^{
            getLuJIn *lujin = [getLuJIn new];
            
            
            //        [vc putImage:self.image_2.image];
            NSString *str1 =[lujin putTheFileName:@"rf_D" andType:@"dat"];
            NSString *str2 =[lujin putTheFileName:@"rf_T" andType:@"dat"];
            c_1=  [str1 UTF8String];
            c_2 =[str2 UTF8String];
            
            
        });
        
        dispatch_sync(queue, ^{
            
            
            ViewController *vc =[[ViewController alloc]init];
            [vc putImage:self.image_2.image andL:c_1 andL2:c_2];
        });
    }
    
    
    
    
    
    
//    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
//
//        
//    });
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
