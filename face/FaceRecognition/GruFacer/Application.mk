APP_ABI := armeabi-v7a
#APP_ABI := armeabi-v7a-hard
#APP_ABI := x86

#unlocked
#APP_ABI := armeabi-v7a x86

APP_STL := gnustl_static
APP_OPTIM := release

APP_PLATFORM := android-15
#cys        1/27/2016
#signal was an inline function until platform android-21, now it's not inline anymore.
#APP_PLATFORM := android-21
NDK_TOOLCHAIN_VERSION=4.9