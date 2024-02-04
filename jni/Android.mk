LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := NullElf
LOCAL_CPPFLAGS := -fno-rtti -fno-exceptions -DNDEBUG -fvisibility=hidden -Wno-narrowing -fdeclspec -pthread -w -s -fexceptions -Wall -O3
LOCAL_SRC_FILES := NullElf/src/nullelf.cpp
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/NullElf/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := NullTrace
LOCAL_CPPFLAGS := -fno-rtti -fno-exceptions -DNDEBUG -fvisibility=hidden -Wno-narrowing -fdeclspec -pthread -w -s -fexceptions -Wall -O3
LOCAL_SRC_FILES := NullTrace/src/nullproc.cpp \
				   NullTrace/src/nullutils.cpp \
				   NullTrace/src/pnull.cpp
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/NullTrace/include
LOCAL_STATIC_LIBRARIES := NullElf
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := NullInject
LOCAL_CPPFLAGS := -fno-rtti -fno-exceptions -DNDEBUG -fvisibility=hidden -Wno-narrowing -fdeclspec -pthread -w -s -fexceptions -Wall -O3
LOCAL_SRC_FILES := src/main.cpp
LOCAL_STATIC_LIBRARIES := NullTrace

include $(BUILD_EXECUTABLE)

