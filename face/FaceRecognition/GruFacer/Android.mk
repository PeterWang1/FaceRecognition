LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(LOCAL_PATH)/../Flags.mk
LOCAL_MODULE    := FreeImage
LOCAL_SRC_FILES := ./Source/libFreeImageStatic.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
include $(LOCAL_PATH)/../Flags.mk
FIMAGE := $(LOCAL_PATH)/Source
LOCAL_MODULE    := GrusFace
LOCAL_SRC_FILES :=  ./dlib_face.cpp ./FaceAlign_.cpp ./FacerNDK.cpp ./rf.cpp\
                ./GruST/util/GST_util.cpp ./GruST/util/GST_dataset.cpp ./GruST/image/GST_image.cpp ./GruST/image/GST_bitmap.cpp ./GruST/image/GST_bmp_draw.cpp ./GruST/image/GST_bitmap_filter.cpp

LOCAL_C_INCLUDES   := $(FIMAGE)
LOCAL_CFLAGS := -O3 -fstrict-aliasing -D__GXX_EXPERIMENTAL_CXX0X__ -DANDROID -DLOG_TAG=\"FaceAligner\" -D_LINUX -D_BIT_MOVE_\
        -std=c++11 -frtti -fexceptions -I -fno-math-errno -fno-signed-zeros -ftree-vectorize -D__STDC_CONSTANT_MACROS -fopenmp
LOCAL_CPPFLAGS := -std=c++11

LOCAL_LDLIBS := -ldl -llog -landroid
LOCAL_LDFLAGS += -fopenmp
LOCAL_STATIC_LIBRARIES := FreeImage

include $(BUILD_SHARED_LIBRARY)
