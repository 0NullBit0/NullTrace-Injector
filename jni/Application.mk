APP_ABI := arm64-v8a armeabi-v7a x86 x86_64
APP_STL := c++_static
APP_PLATFORM = android-29
APP_OPTIM    := release
APP_PIE      := true
APP_MODULES := NullElf 
APP_MODULES += NullTrace 
APP_MODULES += NullInject
APP_CPPFLAGS := -std=c++17